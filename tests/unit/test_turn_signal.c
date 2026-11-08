/*
 * test_turn_signal.c
 * Verifie automatiquement chaque regle des clignotants.
 * Astuce : on INVENTE l'heure (0 ms, 340 ms, 670 ms...), pas besoin d'attendre.
 */
#include "unity.h"
#include "turn_signal.h"

static turn_state_t   state;
static turn_inputs_t  in;
static turn_outputs_t out;

/* Execute AVANT chaque test : situation de depart */
void setUp(void)
{
    turn_init(&state);
    in.lever         = LEVER_OFF;
    in.hazard        = false;
    in.bulb_ok_left  = true;
    in.bulb_ok_right = true;
}

/* Execute APRES chaque test (rien a faire ici) */
void tearDown(void) {}

/* Raccourci : appeler la fonction a l'heure t */
static void at(uint32_t t)
{
    turn_update(&state, &in, t, &out);
}

/* Test 1 : levier OFF -> les 2 lampes sont eteintes */
static void test_lever_off_all_lamps_off(void)
{
    at(0U);
    TEST_ASSERT_FALSE(out.lamp_left);
    TEST_ASSERT_FALSE(out.lamp_right);
}

/* Test 2 : levier a gauche -> la lampe gauche s'allume tout de suite */
static void test_left_lamp_on_immediately(void)
{
    in.lever = LEVER_LEFT;
    at(0U);
    TEST_ASSERT_TRUE(out.lamp_left);
    TEST_ASSERT_FALSE(out.lamp_right);
}

/* Test 3 : le rythme normal est respecte (333 ms allumee, 333 ms eteinte) */
static void test_left_lamp_blinks_at_normal_rate(void)
{
    in.lever = LEVER_LEFT;
    at(0U);
    at(300U);  TEST_ASSERT_TRUE(out.lamp_left);   /* encore allumee */
    at(340U);  TEST_ASSERT_FALSE(out.lamp_left);  /* eteinte        */
    at(670U);  TEST_ASSERT_TRUE(out.lamp_left);   /* rallumee       */
}

/* Test 4 : ampoule gauche grillee -> clignote 2 fois plus vite */
static void test_bulb_failure_blinks_fast(void)
{
    in.lever = LEVER_LEFT;
    in.bulb_ok_left = false;
    at(0U);
    at(170U);  TEST_ASSERT_FALSE(out.lamp_left);  /* deja eteinte a 170 ms */
    at(340U);  TEST_ASSERT_TRUE(out.lamp_left);   /* deja rallumee         */
}

/* Test 5 : warning + levier a droite -> les 2 cotes clignotent (le warning gagne) */
static void test_hazard_has_priority_over_lever(void)
{
    in.lever  = LEVER_RIGHT;
    in.hazard = true;
    at(0U);
    TEST_ASSERT_TRUE(out.lamp_left);
    TEST_ASSERT_TRUE(out.lamp_right);
    at(340U);
    TEST_ASSERT_FALSE(out.lamp_left);
    TEST_ASSERT_FALSE(out.lamp_right);
}

/* Test 6 : on relache le warning -> on revient au levier (droite seulement) */
static void test_hazard_released_returns_to_lever(void)
{
    in.lever  = LEVER_RIGHT;
    in.hazard = true;
    at(0U);
    in.hazard = false;
    at(100U);
    TEST_ASSERT_FALSE(out.lamp_left);
    TEST_ASSERT_TRUE(out.lamp_right);   /* nouveau cycle : allumee tout de suite */
}

/* Test 7 : changement de cote -> le nouveau cote s'allume tout de suite */
static void test_change_side_restarts_cycle(void)
{
    in.lever = LEVER_LEFT;
    at(0U);
    at(340U);                           /* gauche eteinte */
    in.lever = LEVER_RIGHT;
    at(350U);
    TEST_ASSERT_FALSE(out.lamp_left);
    TEST_ASSERT_TRUE(out.lamp_right);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_lever_off_all_lamps_off);
    RUN_TEST(test_left_lamp_on_immediately);
    RUN_TEST(test_left_lamp_blinks_at_normal_rate);
    RUN_TEST(test_bulb_failure_blinks_fast);
    RUN_TEST(test_hazard_has_priority_over_lever);
    RUN_TEST(test_hazard_released_returns_to_lever);
    RUN_TEST(test_change_side_restarts_cycle);
    return UNITY_END();
}
