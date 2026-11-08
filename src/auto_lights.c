/*
 * auto_lights.c
 * Les regles des phares automatiques (hysteresis + temporisation).
 */
#include "auto_lights.h"

void lights_init(lights_state_t *s)
{
    s->headlights  = false;
    s->timing      = false;
    s->timer_start = 0U;
}

void lights_update(lights_state_t *s, const lights_inputs_t *in,
                   uint32_t now_ms, lights_outputs_t *out)
{
    if (!in->ignition_on)
    {
        /* Voiture arretee : phares eteints, chronometre remis a zero */
        s->headlights = false;
        s->timing     = false;
    }
    else
    {
        bool     want_change;   /* la lumiere demande-t-elle de changer d'etat ? */
        uint32_t delay;         /* combien de temps elle doit le demander        */

        if (s->headlights)
        {
            want_change = (in->light_lux > LIGHTS_OFF_ABOVE_LUX);  /* il fait jour ? */
            delay       = LIGHTS_OFF_DELAY_MS;
        }
        else
        {
            want_change = (in->light_lux < LIGHTS_ON_BELOW_LUX);   /* il fait sombre ? */
            delay       = LIGHTS_ON_DELAY_MS;
        }

        if (!want_change)
        {
            /* Rien a changer (ou zone entre 1000 et 3000 lux) : on arrete le chronometre */
            s->timing = false;
        }
        else if (!s->timing)
        {
            /* Debut de la demande : on lance le chronometre */
            s->timing      = true;
            s->timer_start = now_ms;
        }
        else if ((now_ms - s->timer_start) >= delay)
        {
            /* La demande a dure assez longtemps : on change */
            s->headlights = !s->headlights;
            s->timing     = false;
        }
        else
        {
            /* On attend encore */
        }
    }

    out->headlights = s->headlights;
}
