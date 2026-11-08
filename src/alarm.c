/*
 * alarm.c
 * Machine d'etats de l'alarme :
 *
 *  DISARMED --lock--> ARMING (20 s) --> ARMED --porte ouverte--> TRIGGERED (30 s)
 *     ^                                   ^                           |
 *     |                                   +------- 30 s ecoulees -----+
 *     +-------- unlock ou contact mis (depuis n'importe quel etat) ----+
 */
#include "alarm.h"

void alarm_init(alarm_state_t *s)
{
    s->status         = ALARM_DISARMED;
    s->timer_start    = 0U;
    s->prev_door_open = false;
}

void alarm_update(alarm_state_t *s, const alarm_inputs_t *in,
                  uint32_t now_ms, alarm_outputs_t *out)
{
    /* Une porte VIENT de s'ouvrir (fermee avant, ouverte maintenant) */
    const bool door_opened = in->door_open && (!s->prev_door_open);

    /* L'alarme ne doit fonctionner que voiture verrouillee ET contact coupe */
    const bool can_be_armed = in->locked && (!in->ignition_on);

    if (!can_be_armed)
    {
        /* Regle prioritaire : deverrouillage ou contact -> on desactive tout */
        s->status = ALARM_DISARMED;
    }
    else
    {
        switch (s->status)
        {
            case ALARM_DISARMED:
                /* On vient de verrouiller : debut des 20 s d'armement */
                s->status      = ALARM_ARMING;
                s->timer_start = now_ms;
                break;

            case ALARM_ARMING:
                /* Pendant l'armement, ouvrir une porte ne declenche rien */
                if ((now_ms - s->timer_start) >= ALARM_ARMING_DELAY_MS)
                {
                    s->status = ALARM_ARMED;
                }
                break;

            case ALARM_ARMED:
                if (door_opened)
                {
                    s->status      = ALARM_TRIGGERED;
                    s->timer_start = now_ms;
                }
                break;

            case ALARM_TRIGGERED:
                if ((now_ms - s->timer_start) >= ALARM_SIREN_TIME_MS)
                {
                    s->status = ALARM_ARMED;   /* la sirene s'arrete, on surveille de nouveau */
                }
                break;

            default:
                /* Etat inconnu (memoire corrompue) : on revient a un etat sur */
                s->status = ALARM_DISARMED;
                break;
        }
    }

    s->prev_door_open = in->door_open;

    out->status = s->status;
    out->siren  = (s->status == ALARM_TRIGGERED);
}
