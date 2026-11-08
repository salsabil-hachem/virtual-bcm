/*
 * alarm.h
 * Alarme antivol du BCM virtuel.
 */
#ifndef ALARM_H
#define ALARM_H

#include <stdbool.h>
#include <stdint.h>

#define ALARM_ARMING_DELAY_MS  (20000U)  /* 20 s pour s'eloigner avant que l'alarme soit active */
#define ALARM_SIREN_TIME_MS    (30000U)  /* la sirene sonne 30 s                                */

/* Les 4 etats de l'alarme */
typedef enum
{
    ALARM_DISARMED,   /* desactivee                     */
    ALARM_ARMING,     /* en cours d'activation (20 s)   */
    ALARM_ARMED,      /* active, surveille les portes   */
    ALARM_TRIGGERED   /* declenchee : sirene + feux     */
} alarm_status_t;

/* ENTREES */
typedef struct
{
    bool locked;        /* portes verrouillees ? (vient du module door_lock) */
    bool ignition_on;   /* contact mis ?                                     */
    bool door_open;     /* une porte est-elle ouverte ?                      */
} alarm_inputs_t;

/* SORTIES */
typedef struct
{
    alarm_status_t status;
    bool           siren;   /* klaxon / sirene */
} alarm_outputs_t;

/* MEMOIRE */
typedef struct
{
    alarm_status_t status;
    uint32_t       timer_start;
    bool           prev_door_open;
} alarm_state_t;

void alarm_init(alarm_state_t *s);
void alarm_update(alarm_state_t *s, const alarm_inputs_t *in,
                  uint32_t now_ms, alarm_outputs_t *out);

#endif /* ALARM_H */
