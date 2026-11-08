/*
 * turn_signal.c
 * Les regles des clignotants et du warning.
 */
#include "turn_signal.h"

/* Etape 1 : quel mode le conducteur demande-t-il ? (le warning gagne toujours) */
static turn_mode_t requested_mode(const turn_inputs_t *in)
{
    turn_mode_t mode = TURN_MODE_NONE;

    if (in->hazard)
    {
        mode = TURN_MODE_HAZARD;
    }
    else if (in->lever == LEVER_LEFT)
    {
        mode = TURN_MODE_LEFT;
    }
    else if (in->lever == LEVER_RIGHT)
    {
        mode = TURN_MODE_RIGHT;
    }
    else
    {
        mode = TURN_MODE_NONE;
    }
    return mode;
}

/* Etape 2 : quelle vitesse ? (plus vite si l'ampoule du cote actif est grillee) */
static uint32_t half_period(turn_mode_t mode, const turn_inputs_t *in)
{
    uint32_t half = TURN_HALF_PERIOD_MS;

    if ((mode == TURN_MODE_LEFT) && (!in->bulb_ok_left))
    {
        half = TURN_HALF_PERIOD_FAST_MS;
    }
    else if ((mode == TURN_MODE_RIGHT) && (!in->bulb_ok_right))
    {
        half = TURN_HALF_PERIOD_FAST_MS;
    }
    else
    {
        /* vitesse normale (le warning ne change pas de vitesse) */
    }
    return half;
}

void turn_init(turn_state_t *s)
{
    s->mode        = TURN_MODE_NONE;
    s->lamp_on     = false;
    s->phase_start = 0U;
}

void turn_update(turn_state_t *s, const turn_inputs_t *in,
                 uint32_t now_ms, turn_outputs_t *out)
{
    const turn_mode_t mode = requested_mode(in);

    if (mode == TURN_MODE_NONE)
    {
        /* Rien demande : tout s'eteint */
        s->mode    = TURN_MODE_NONE;
        s->lamp_on = false;
    }
    else if (mode != s->mode)
    {
        /* Nouveau mode (debut ou changement de cote) :
           on repart d'un cycle propre, le 1er flash est IMMEDIAT */
        s->mode        = mode;
        s->lamp_on     = true;
        s->phase_start = now_ms;
    }
    else
    {
        /* Meme mode qu'avant : on inverse la lampe chaque fois
           qu'une demi-periode est ecoulee */
        const uint32_t half = half_period(mode, in);
        while ((now_ms - s->phase_start) >= half)
        {
            s->lamp_on      = !s->lamp_on;
            s->phase_start += half;
        }
    }

    /* Sorties : la lampe s'allume si elle est dans sa phase "allumee"
       ET si son cote est concerne */
    out->lamp_left  = s->lamp_on && ((s->mode == TURN_MODE_LEFT)  || (s->mode == TURN_MODE_HAZARD));
    out->lamp_right = s->lamp_on && ((s->mode == TURN_MODE_RIGHT) || (s->mode == TURN_MODE_HAZARD));
}
