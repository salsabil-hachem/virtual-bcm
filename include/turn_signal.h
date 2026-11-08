/*
 * turn_signal.h
 * Clignotants et warning du BCM virtuel.
 *
 * Principe : on donne des ENTREES (levier, warning, ampoules) et l'HEURE,
 * la fonction calcule les SORTIES (lampe gauche / droite allumee ou non).
 */
#ifndef TURN_SIGNAL_H
#define TURN_SIGNAL_H

#include <stdbool.h>
#include <stdint.h>

/* Duree d'une demi-periode (lampe allumee OU eteinte), en millisecondes */
#define TURN_HALF_PERIOD_MS       (333U)  /* normal : 1,5 clignotement par seconde */
#define TURN_HALF_PERIOD_FAST_MS  (167U)  /* ampoule grillee : 2 fois plus vite    */

/* Positions possibles du levier */
typedef enum
{
    LEVER_OFF,
    LEVER_LEFT,
    LEVER_RIGHT
} lever_t;

/* Ce qui clignote en ce moment */
typedef enum
{
    TURN_MODE_NONE,   /* rien ne clignote          */
    TURN_MODE_LEFT,   /* clignotant gauche         */
    TURN_MODE_RIGHT,  /* clignotant droit          */
    TURN_MODE_HAZARD  /* warning : les deux cotes  */
} turn_mode_t;

/* ENTREES : ce que fait le conducteur + etat des ampoules */
typedef struct
{
    lever_t lever;
    bool    hazard;         /* bouton warning appuye ?      */
    bool    bulb_ok_left;   /* ampoule gauche en bon etat ? */
    bool    bulb_ok_right;  /* ampoule droite en bon etat ? */
} turn_inputs_t;

/* SORTIES : ce qu'on envoie aux lampes */
typedef struct
{
    bool lamp_left;
    bool lamp_right;
} turn_outputs_t;

/* MEMOIRE de la fonction entre deux appels (la machine d'etats) */
typedef struct
{
    turn_mode_t mode;          /* mode actuel                         */
    bool        lamp_on;       /* phase actuelle : allumee ou eteinte */
    uint32_t    phase_start;   /* heure du debut de la phase (ms)     */
} turn_state_t;

/* A appeler une fois au demarrage */
void turn_init(turn_state_t *s);

/* A appeler regulierement (par exemple toutes les 10 ms) */
void turn_update(turn_state_t *s, const turn_inputs_t *in,
                 uint32_t now_ms, turn_outputs_t *out);

#endif /* TURN_SIGNAL_H */
