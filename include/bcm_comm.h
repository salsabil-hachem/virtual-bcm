/*
 * bcm_comm.h
 * Traduction entre les messages CAN (octets) et les entrees / sorties du BCM.
 * Le contenu de chaque message est decrit dans can/bcm.dbc.
 */
#ifndef BCM_COMM_H
#define BCM_COMM_H

#include <stdbool.h>
#include <stdint.h>
#include "bcm.h"

/* Numeros (ID) des messages CAN */
#define CAN_ID_DASH_INPUTS   (0x100U)  /* tableau de bord -> BCM */
#define CAN_ID_VEH_SPEED     (0x101U)
#define CAN_ID_ENV_LIGHT     (0x102U)
#define CAN_ID_DOOR_STATUS   (0x103U)
#define CAN_ID_LAMP_DIAG     (0x104U)
#define CAN_ID_BCM_LAMPS     (0x200U)  /* BCM -> tableau de bord */
#define CAN_ID_BCM_STATUS    (0x201U)

#define CAN_DLC_BCM_LAMPS    (4U)      /* nombre d'octets envoyes */
#define CAN_DLC_BCM_STATUS   (3U)

#define COMM_DASH_TIMEOUT_MS (500U)    /* plus de DASH_Inputs pendant 500 ms -> mode securite */
#define FAULT_DASH_LOST      (0x01U)

/* Ce que le BCM sait du vehicule, d'apres les messages recus */
typedef struct
{
    bcm_inputs_t inputs;
    uint32_t     last_dash_ms;   /* heure du dernier DASH_Inputs recu */
    bool         dash_seen;      /* au moins un DASH_Inputs recu ?    */
} bcm_comm_t;

void comm_init(bcm_comm_t *c);

/* Traite un message recu. Retourne false si le message est invalide ou inconnu. */
bool comm_receive(bcm_comm_t *c, uint32_t id, const uint8_t *data, uint8_t dlc, uint32_t now_ms);

/* Vrai si le tableau de bord ne parle plus (mode securite) */
bool comm_dash_lost(const bcm_comm_t *c, uint32_t now_ms);

/* Fabrique les messages a envoyer */
void comm_encode_lamps(const bcm_outputs_t *out, bool safe_mode, uint8_t data[CAN_DLC_BCM_LAMPS]);
void comm_encode_status(const bcm_outputs_t *out, bool safe_mode, uint8_t data[CAN_DLC_BCM_STATUS]);

#endif /* BCM_COMM_H */
