/*
 * door_lock.c
 * Les regles du verrouillage des portes.
 */
#include "door_lock.h"

void door_init(door_state_t *s)
{
    s->locked          = false;
    s->autolock_done   = false;
    s->prev_key_lock   = false;
    s->prev_key_unlock = false;
    s->prev_ignition   = false;
}

void door_update(door_state_t *s, const door_inputs_t *in, door_outputs_t *out)
{
    /* Detection d'APPUI (front montant) : le bouton est appuye maintenant
       ET il ne l'etait pas au dernier appel. Ainsi un appui long compte une seule fois. */
    const bool lock_pressed   = in->key_lock   && (!s->prev_key_lock);
    const bool unlock_pressed = in->key_unlock && (!s->prev_key_unlock);

    /* Le contact vient d'etre coupe (marche -> arret) */
    const bool ignition_turned_off = (!in->ignition_on) && s->prev_ignition;

    /* Regle 1 et 2 : boutons de la cle */
    if (lock_pressed)
    {
        s->locked = true;
    }
    else if (unlock_pressed)
    {
        s->locked = false;
    }
    else
    {
        /* aucun bouton */
    }

    /* Regle 3 : verrouillage automatique au-dessus de 15 km/h, UNE SEULE FOIS par trajet */
    if ((in->speed_kmh > DOOR_AUTOLOCK_SPEED_KMH) && (!s->autolock_done))
    {
        s->locked        = true;
        s->autolock_done = true;
    }

    /* Regle 4 : contact coupe -> deverrouillage, et nouveau trajet */
    if (ignition_turned_off)
    {
        s->locked        = false;
        s->autolock_done = false;
    }

    /* Memoriser l'etat actuel pour le prochain appel */
    s->prev_key_lock   = in->key_lock;
    s->prev_key_unlock = in->key_unlock;
    s->prev_ignition   = in->ignition_on;

    out->locked = s->locked;
}
