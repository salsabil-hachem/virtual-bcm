/*
 * bcm.c
 * Le chef d'orchestre : appelle chaque fonction dans le bon ordre
 * et decide des priorites entre elles.
 */
#include "bcm.h"

void bcm_init(bcm_t *bcm)
{
    turn_init(&bcm->turn);
    door_init(&bcm->door);
    lights_init(&bcm->lights);
    alarm_init(&bcm->alarm);
}

void bcm_update(bcm_t *bcm, const bcm_inputs_t *in, uint32_t now_ms, bcm_outputs_t *out)
{
    /* 1. Portes */
    const door_inputs_t door_in = { in->key_lock, in->key_unlock, in->speed_kmh, in->ignition_on };
    door_outputs_t door_out;
    door_update(&bcm->door, &door_in, &door_out);

    /* 2. Alarme (elle a besoin de savoir si les portes sont verrouillees) */
    const alarm_inputs_t alarm_in = { door_out.locked, in->ignition_on, in->door_open };
    alarm_outputs_t alarm_out;
    alarm_update(&bcm->alarm, &alarm_in, now_ms, &alarm_out);

    /* 3. Clignotants. PRIORITE : si l'alarme sonne, elle force le warning */
    turn_inputs_t turn_in = { in->lever, in->hazard, in->bulb_ok_left, in->bulb_ok_right };
    if (alarm_out.siren)
    {
        turn_in.hazard = true;
    }
    turn_outputs_t turn_out;
    turn_update(&bcm->turn, &turn_in, now_ms, &turn_out);

    /* 4. Phares automatiques */
    const lights_inputs_t lights_in = { in->light_lux, in->ignition_on };
    lights_outputs_t lights_out;
    lights_update(&bcm->lights, &lights_in, now_ms, &lights_out);

    /* 5. Rassembler toutes les sorties */
    out->lamp_left    = turn_out.lamp_left;
    out->lamp_right   = turn_out.lamp_right;
    out->headlights   = lights_out.headlights;
    out->doors_locked = door_out.locked;
    out->siren        = alarm_out.siren;
    out->alarm_status = alarm_out.status;
}
