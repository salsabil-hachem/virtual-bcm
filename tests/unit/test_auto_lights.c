/*
 * test_auto_lights.c
 * Verifie les regles des phares automatiques.
 */
#include "unity.h"
#include "auto_lights.h"

static lights_state_t   state;
static lights_inputs_t  in;
static lights_outputs_t out;

void setUp(void)
{
    lights_init(&state);
    in.light_lux   = 10000U;   /* plein jour */
    in.ignition_on = true;
}

void tearDown(void) {}

static void at(uint32_t t)
{
    lights_update(&state, &in, t, &out);
}

/* Raccourci : il fait sombre de 0 a 2,1 s -> phares allumes */
static void get_dark(void)
{
    in.light_lux = 500U;
    at(0U);
    at(2100U);
}

static void test_daylight_lights_off(void)
{
    at(0U);
    TEST_ASSERT_FALSE(out.headlights);
}

/* Sombre pendant 1 s seulement (un pont) : on n'allume pas */
static void test_short_darkness_ignored(void)
{
    in.light_lux = 500U;
    at(0U);
    at(1000U);
    TEST_ASSERT_FALSE(out.headlights);
}

/* Sombre pendant plus de 2 s : on allume */
static void test_long_darkness_turns_on(void)
{
    get_dark();
    TEST_ASSERT_TRUE(out.headlights);
}

/* Un retour de lumiere remet le chronometre a zero */
static void test_light_interruption_restarts_timer(void)
{
    in.light_lux = 500U;
    at(0U);
    at(1500U);
    in.light_lux = 10000U;
    at(1600U);           /* sortie du pont */
    in.light_lux = 500U;
    at(1700U);
    at(3000U);           /* seulement 1,3 s de noir depuis 1700 ms */
    TEST_ASSERT_FALSE(out.headlights);
}

/* Hysteresis : entre 1000 et 3000 lux, les phares restent allumes */
static void test_hysteresis_keeps_lights_on(void)
{
    get_dark();
    in.light_lux = 2000U;
    at(5000U);
    at(20000U);
    TEST_ASSERT_TRUE(out.headlights);
}

/* Jour pendant 4 s : pas encore ; pendant 5 s : on eteint */
static void test_daylight_turns_off_after_5s(void)
{
    get_dark();
    in.light_lux = 10000U;
    at(3000U);
    at(7000U);
    TEST_ASSERT_TRUE(out.headlights);
    at(8000U);
    TEST_ASSERT_FALSE(out.headlights);
}

static void test_ignition_off_turns_off(void)
{
    get_dark();
    in.ignition_on = false;
    at(2200U);
    TEST_ASSERT_FALSE(out.headlights);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_daylight_lights_off);
    RUN_TEST(test_short_darkness_ignored);
    RUN_TEST(test_long_darkness_turns_on);
    RUN_TEST(test_light_interruption_restarts_timer);
    RUN_TEST(test_hysteresis_keeps_lights_on);
    RUN_TEST(test_daylight_turns_off_after_5s);
    RUN_TEST(test_ignition_off_turns_off);
    return UNITY_END();
}
