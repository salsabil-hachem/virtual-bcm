/*
 * bcm.h
 * Le BCM complet : rassemble toutes les fonctions.
 */
#ifndef BCM_H
#define BCM_H

#include <stdbool.h>
#include <stdint.h>
#include "turn_signal.h"
#include "door_lock.h"
#include "auto_lights.h"
#include "alarm.h"

/* TOUTES les entrees du BCM (ce que recoit le boitier) */
typedef struct
{
    lever_t  lever;
    bool     hazard;
    bool     bulb_ok_left;
    bool     bulb_ok_right;
    bool     key_lock;
    bool     key_unlock;
    bool     ignition_on;
    uint16_t speed_kmh;
    uint16_t light_lux;
    bool     door_open;
} bcm_inputs_t;

/* TOUTES les sorties du BCM (ce qu'il commande) */
typedef struct
{
    bool           lamp_left;
    bool           lamp_right;
    bool           headlights;
    bool           doors_locked;
    bool           siren;
    alarm_status_t alarm_status;
} bcm_outputs_t;

/* MEMOIRE du BCM = la memoire de chaque fonction */
typedef struct
{
    turn_state_t   turn;
    door_state_t   door;
    lights_state_t lights;
    alarm_state_t  alarm;
} bcm_t;

void bcm_init(bcm_t *bcm);
void bcm_update(bcm_t *bcm, const bcm_inputs_t *in, uint32_t now_ms, bcm_outputs_t *out);

#endif /* BCM_H */
