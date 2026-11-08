/*
 * bcm_comm.c
 * Decodage des messages recus et codage des messages envoyes.
 * Chaque signal occupe un octet (simple a lire avec candump).
 */
#include "bcm_comm.h"

/* Lit un nombre sur 2 octets (octet de poids faible en premier = "little endian") */
static uint16_t read_u16(const uint8_t *data)
{
    return (uint16_t)((uint16_t)data[0] | (uint16_t)((uint16_t)data[1] << 8U));
}

void comm_init(bcm_comm_t *c)
{
    /* Valeurs par defaut tant qu'aucun message n'est recu */
    c->inputs.lever         = LEVER_OFF;
    c->inputs.hazard        = false;
    c->inputs.bulb_ok_left  = true;
    c->inputs.bulb_ok_right = true;
    c->inputs.key_lock      = false;
    c->inputs.key_unlock    = false;
    c->inputs.ignition_on   = false;
    c->inputs.speed_kmh     = 0U;
    c->inputs.light_lux     = 10000U;   /* plein jour */
    c->inputs.door_open     = false;
    c->last_dash_ms         = 0U;
    c->dash_seen            = false;
}

bool comm_receive(bcm_comm_t *c, uint32_t id, const uint8_t *data, uint8_t dlc, uint32_t now_ms)
{
    bool ok = false;

    switch (id)
    {
        case CAN_ID_DASH_INPUTS:
            /* 5 octets attendus, et un levier valide (0, 1 ou 2) */
            if ((dlc >= 5U) && (data[0] <= (uint8_t)LEVER_RIGHT))
            {
                c->inputs.lever       = (lever_t)data[0];
                c->inputs.hazard      = (data[1] != 0U);
                c->inputs.key_lock    = (data[2] != 0U);
                c->inputs.key_unlock  = (data[3] != 0U);
                c->inputs.ignition_on = (data[4] != 0U);
                c->last_dash_ms       = now_ms;
                c->dash_seen          = true;
                ok = true;
            }
            break;

        case CAN_ID_VEH_SPEED:
            if (dlc >= 2U)
            {
                c->inputs.speed_kmh = read_u16(data);
                ok = true;
            }
            break;

        case CAN_ID_ENV_LIGHT:
            if (dlc >= 2U)
            {
                c->inputs.light_lux = read_u16(data);
                ok = true;
            }
            break;

        case CAN_ID_DOOR_STATUS:
            if (dlc >= 1U)
            {
                c->inputs.door_open = (data[0] != 0U);
                ok = true;
            }
            break;

        case CAN_ID_LAMP_DIAG:
            if (dlc >= 2U)
            {
                c->inputs.bulb_ok_left  = (data[0] == 0U);   /* 0 = pas de panne */
                c->inputs.bulb_ok_right = (data[1] == 0U);
                ok = true;
            }
            break;

        default:
            /* Message qui ne concerne pas le BCM : ignore */
            break;
    }
    return ok;
}

bool comm_dash_lost(const bcm_comm_t *c, uint32_t now_ms)
{
    bool lost;
    if (c->dash_seen)
    {
        lost = ((now_ms - c->last_dash_ms) >= COMM_DASH_TIMEOUT_MS);
    }
    else
    {
        lost = (now_ms >= COMM_DASH_TIMEOUT_MS);   /* jamais rien recu depuis le demarrage */
    }
    return lost;
}

void comm_encode_lamps(const bcm_outputs_t *out, bool safe_mode, uint8_t data[CAN_DLC_BCM_LAMPS])
{
    data[0] = out->lamp_left  ? 1U : 0U;
    data[1] = out->lamp_right ? 1U : 0U;
    data[2] = (out->headlights || safe_mode) ? 1U : 0U;   /* mode securite : phares forces */
    data[3] = out->siren      ? 1U : 0U;
}

void comm_encode_status(const bcm_outputs_t *out, bool safe_mode, uint8_t data[CAN_DLC_BCM_STATUS])
{
    data[0] = out->doors_locked ? 1U : 0U;
    data[1] = (uint8_t)out->alarm_status;
    data[2] = safe_mode ? FAULT_DASH_LOST : 0U;
}
