/*
 * main.c
 * Le BCM virtuel qui "vit" : 4 taches FreeRTOS qui tournent en parallele.
 *
 *   Tache      Rythme    Priorite  Travail
 *   CanRx      1 ms      3         lit les messages CAN recus
 *   BcmLogic   10 ms     4         applique les regles (bcm_update)
 *   CanTx      20 ms     2         envoie l'etat des lampes et des portes
 *   Monitor    100 ms    1         surveille la perte du tableau de bord
 */
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "bcm.h"
#include "bcm_comm.h"
#include "can_socket.h"

/* ---------------- Donnees partagees entre les taches ---------------- */
static SemaphoreHandle_t g_mutex;     /* le "cadenas" qui protege les donnees ci-dessous */
static bcm_comm_t        g_comm;      /* entrees recues par CAN                          */
static bcm_outputs_t     g_outputs;   /* sorties calculees par le BCM                    */
static bool              g_safe_mode; /* tableau de bord perdu ?                         */
static int               g_can_fd = -1;

static uint32_t now_ms(void)
{
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

/* ---------------- Tache 1 : reception CAN ---------------- */
static void task_can_rx(void *arg)
{
    (void)arg;
    for (;;)
    {
        uint32_t id;
        uint8_t  data[8];
        uint8_t  dlc;
        while (can_receive(g_can_fd, &id, data, &dlc) == 1)
        {
            xSemaphoreTake(g_mutex, portMAX_DELAY);    /* fermer le cadenas */
            (void)comm_receive(&g_comm, id, data, dlc, now_ms());
            xSemaphoreGive(g_mutex);                   /* rouvrir le cadenas */
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

/* ---------------- Tache 2 : logique du BCM ---------------- */
static const char *alarm_name(alarm_status_t s)
{
    static const char *const names[] = { "DESACTIVEE", "ARMEMENT", "ARMEE", "DECLENCHEE" };
    return names[s];
}

static void task_bcm_logic(void *arg)
{
    (void)arg;
    bcm_t         bcm;
    bcm_outputs_t out;
    bcm_outputs_t prev = { false, false, false, false, false, ALARM_DISARMED };
    bool          prev_safe = false;
    bool          first = true;

    bcm_init(&bcm);
    TickType_t last_wake = xTaskGetTickCount();

    for (;;)
    {
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));   /* exactement toutes les 10 ms */

        xSemaphoreTake(g_mutex, portMAX_DELAY);
        const bcm_inputs_t in   = g_comm.inputs;           /* copie des entrees */
        const bool         safe = g_safe_mode;
        xSemaphoreGive(g_mutex);

        bcm_update(&bcm, &in, now_ms(), &out);

        xSemaphoreTake(g_mutex, portMAX_DELAY);
        g_outputs = out;
        xSemaphoreGive(g_mutex);

        /* Afficher une ligne seulement quand quelque chose change */
        const bool changed = first || (safe != prev_safe) ||
                             (out.lamp_left != prev.lamp_left) || (out.lamp_right != prev.lamp_right) ||
                             (out.headlights != prev.headlights) || (out.doors_locked != prev.doors_locked) ||
                             (out.siren != prev.siren) || (out.alarm_status != prev.alarm_status);
        if (changed)
        {
            printf("[%7lu ms]  Clignotant G:%-3s D:%-3s | Phares:%-3s | Portes:%-12s | Alarme:%-10s | Sirene:%-3s%s\n",
                   (unsigned long)now_ms(),
                   out.lamp_left ? "ON" : "off", out.lamp_right ? "ON" : "off",
                   (out.headlights || safe) ? "ON" : "off",
                   out.doors_locked ? "verrouillees" : "ouvertes",
                   alarm_name(out.alarm_status), out.siren ? "ON" : "off",
                   safe ? "  << MODE SECURITE : tableau de bord perdu" : "");
            fflush(stdout);
            prev      = out;
            prev_safe = safe;
            first     = false;
        }
    }
}

/* ---------------- Tache 3 : emission CAN ---------------- */
static void task_can_tx(void *arg)
{
    (void)arg;
    TickType_t last_wake = xTaskGetTickCount();

    for (;;)
    {
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(20));

        xSemaphoreTake(g_mutex, portMAX_DELAY);
        const bcm_outputs_t out  = g_outputs;
        const bool          safe = g_safe_mode;
        xSemaphoreGive(g_mutex);

        uint8_t lamps[CAN_DLC_BCM_LAMPS];
        uint8_t status[CAN_DLC_BCM_STATUS];
        comm_encode_lamps(&out, safe, lamps);
        comm_encode_status(&out, safe, status);
        (void)can_send(g_can_fd, CAN_ID_BCM_LAMPS, lamps, CAN_DLC_BCM_LAMPS);
        (void)can_send(g_can_fd, CAN_ID_BCM_STATUS, status, CAN_DLC_BCM_STATUS);
    }
}

/* ---------------- Tache 4 : surveillance de la communication ---------------- */
static void task_monitor(void *arg)
{
    (void)arg;
    TickType_t last_wake = xTaskGetTickCount();

    for (;;)
    {
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(100));

        xSemaphoreTake(g_mutex, portMAX_DELAY);
        g_safe_mode = comm_dash_lost(&g_comm, now_ms());
        xSemaphoreGive(g_mutex);
    }
}

/* ---------------- Demarrage ---------------- */
int main(void)
{
    const char *iface = getenv("CAN_IFACE");
    if (iface == NULL)
    {
        iface = "vcan0";
    }

    g_can_fd = can_open(iface);
    if (g_can_fd < 0)
    {
        printf("Erreur : impossible d'ouvrir %s. As-tu cree vcan0 ? (voir scripts/setup_vcan.sh)\n", iface);
        return 1;
    }

    comm_init(&g_comm);
    g_mutex = xSemaphoreCreateMutex();

    printf("BCM virtuel demarre sur %s (FreeRTOS %s). Ctrl+C pour arreter.\n", iface, tskKERNEL_VERSION_NUMBER);
    fflush(stdout);

    const uint32_t stack = configMINIMAL_STACK_SIZE * 4U;
    xTaskCreate(task_can_rx,    "CanRx",    stack, NULL, 3, NULL);
    xTaskCreate(task_bcm_logic, "BcmLogic", stack, NULL, 4, NULL);
    xTaskCreate(task_can_tx,    "CanTx",    stack, NULL, 2, NULL);
    xTaskCreate(task_monitor,   "Monitor",  stack, NULL, 1, NULL);

    vTaskStartScheduler();   /* FreeRTOS prend la main et ne rend jamais la main */
    return 0;
}
