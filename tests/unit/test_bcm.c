/*
 * test_bcm.c
 * Verifie que les fonctions travaillent bien ENSEMBLE (scenarios complets).
 */
#include "unity.h"
#include "bcm.h"

static bcm_t         bcm;
static bcm_inputs_t  in;
static bcm_outputs_t out;

void setUp(void)
{
    bcm_init(&bcm);
    in.lever         = LEVER_OFF;
    in.hazard        = false;
    in.bulb_ok_left  = true;
    in.bulb_ok_right = true;
    in.key_lock      = false;
    in.key_unlock    = false;
    in.ignition_on   = true;
    in.speed_kmh     = 0U;
    in.light_lux     = 10000U;
    in.door_open     = false;
}

void tearDown(void) {}

static void at(uint32_t t)
{
    bcm_update(&bcm, &in, t, &out);
}

/* Scenario 1 : on demarre et on roule -> portes verrouillees, pas d'alarme */
static void test_driving_locks_doors_without_alarm(void)
{
    at(0U);
    in.speed_kmh = 30U;
    at(100U);
    TEST_ASSERT_TRUE(out.doors_locked);
    TEST_ASSERT_EQUAL(ALARM_DISARMED, out.alarm_status);
}

/* Scenario 2 : on gare, on coupe le contact -> portes deverrouillees */
static void test_parking_unlocks_doors(void)
{
    in.speed_kmh = 30U;
    at(0U);
    in.speed_kmh   = 0U;
    in.ignition_on = false;
    at(100U);
    TEST_ASSERT_FALSE(out.doors_locked);
}

/* Scenario 3 : vol -> l'alarme fait clignoter les 2 cotes, meme levier a gauche */
static void test_alarm_flashes_both_sides(void)
{
    in.ignition_on = false;
    in.lever       = LEVER_LEFT;
    at(0U);
    in.key_lock = true;          /* le conducteur verrouille avec la cle */
    at(10U);
    in.key_lock = false;
    at(20U);
    at(20100U);                  /* 20 s plus tard : alarme armee */
    in.door_open = true;         /* un voleur ouvre la porte */
    at(21000U);
    TEST_ASSERT_TRUE(out.siren);
    TEST_ASSERT_TRUE(out.lamp_left);
    TEST_ASSERT_TRUE(out.lamp_right);
}

/* Scenario 4 : tunnel -> les phares s'allument */
static void test_tunnel_turns_headlights_on(void)
{
    in.light_lux = 200U;
    at(0U);
    at(2500U);
    TEST_ASSERT_TRUE(out.headlights);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_driving_locks_doors_without_alarm);
    RUN_TEST(test_parking_unlocks_doors);
    RUN_TEST(test_alarm_flashes_both_sides);
    RUN_TEST(test_tunnel_turns_headlights_on);
    return UNITY_END();
}
