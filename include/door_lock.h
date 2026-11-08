/*
 * door_lock.h
 * Verrouillage des portes du BCM virtuel.
 */
#ifndef DOOR_LOCK_H
#define DOOR_LOCK_H

#include <stdbool.h>
#include <stdint.h>

#define DOOR_AUTOLOCK_SPEED_KMH  (15U)   /* verrouillage auto au-dessus de 15 km/h */

/* ENTREES */
typedef struct
{
    bool     key_lock;      /* bouton "lock" de la cle appuye ?   */
    bool     key_unlock;    /* bouton "unlock" de la cle appuye ? */
    uint16_t speed_kmh;     /* vitesse de la voiture              */
    bool     ignition_on;   /* contact mis (moteur en marche) ?   */
} door_inputs_t;

/* SORTIE */
typedef struct
{
    bool locked;            /* portes verrouillees ? */
} door_outputs_t;

/* MEMOIRE */
typedef struct
{
    bool locked;
    bool autolock_done;     /* deja verrouille automatiquement pendant ce trajet ? */
    bool prev_key_lock;     /* etat du bouton au dernier appel (detection d'appui) */
    bool prev_key_unlock;
    bool prev_ignition;
} door_state_t;

void door_init(door_state_t *s);
void door_update(door_state_t *s, const door_inputs_t *in, door_outputs_t *out);

#endif /* DOOR_LOCK_H */
