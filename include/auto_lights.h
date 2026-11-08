/*
 * auto_lights.h
 * Phares automatiques du BCM virtuel.
 */
#ifndef AUTO_LIGHTS_H
#define AUTO_LIGHTS_H

#include <stdbool.h>
#include <stdint.h>

#define LIGHTS_ON_BELOW_LUX    (1000U)  /* il fait sombre en dessous de 1000 lux */
#define LIGHTS_OFF_ABOVE_LUX   (3000U)  /* il fait jour au-dessus de 3000 lux    */
#define LIGHTS_ON_DELAY_MS     (2000U)  /* sombre pendant 2 s -> allumer         */
#define LIGHTS_OFF_DELAY_MS    (5000U)  /* jour pendant 5 s   -> eteindre        */

/* ENTREES */
typedef struct
{
    uint16_t light_lux;     /* luminosite mesuree par le capteur */
    bool     ignition_on;   /* contact mis ?                     */
} lights_inputs_t;

/* SORTIE */
typedef struct
{
    bool headlights;        /* phares allumes ? */
} lights_outputs_t;

/* MEMOIRE */
typedef struct
{
    bool     headlights;
    bool     timing;        /* un chronometre est-il en cours ?      */
    uint32_t timer_start;   /* heure de depart du chronometre (ms)   */
} lights_state_t;

void lights_init(lights_state_t *s);
void lights_update(lights_state_t *s, const lights_inputs_t *in,
                   uint32_t now_ms, lights_outputs_t *out);

#endif /* AUTO_LIGHTS_H */
