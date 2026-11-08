/*
 * test_alarm.c
 * Verifie chaque fleche de la machine d'etats de l'alarme.
 */
#include "unity.h"
#include "alarm.h"

static alarm_state_t   state;
static alarm_inputs_t  in;
static alarm_outputs_t out;

void setUp(void)
{
    alarm_init(&state);
    in.locked      = false;
    in.ignition_on = false;
    in.door_open   = false;
}

void tearDown(void) {}

static void at(uint32_t t)
{
    alarm_update(&state, &in, t, &out);
}

/* Raccourci : verrouiller a 0 ms et attendre que l'alarme soit armee */
static void arm(void)
{
    in.locked = true;
    at(0U);
    at(20000U);
}

static void test_starts_disarmed(void)
{
    at(0U);
    TEST_ASSERT_EQUAL(ALARM_DISARMED, out.status);
    TEST_ASSERT_FALSE(out.siren);
}

static void test_lock_starts_arming(void)
{
    in.locked = true;
    at(0U);
    TEST_ASSERT_EQUAL(ALARM_ARMING, out.status);
}

static void test_armed_after_20s(void)
{
    in.locked = true;
    at(0U);
    at(19000U);
    TEST_ASSERT_EQUAL(ALARM_ARMING, out.status);
    at(20000U);
    TEST_ASSERT_EQUAL(ALARM_ARMED, out.status);
}

/* Pendant l'armement, ouvrir une porte ne declenche rien */
static void test_door_during_arming_ignored(void)
{
    in.locked = true;
    at(0U);
    in.door_open = true;
    at(5000U);
    TEST_ASSERT_FALSE(out.siren);
}

static void test_door_opened_when_armed_triggers(void)
{
    arm();
    in.door_open = true;
    at(25000U);
    TEST_ASSERT_EQUAL(ALARM_TRIGGERED, out.status);
    TEST_ASSERT_TRUE(out.siren);
}

static void test_siren_stops_after_30s(void)
{
    arm();
    in.door_open = true;
    at(25000U);
    at(54000U);
    TEST_ASSERT_TRUE(out.siren);
    at(55000U);
    TEST_ASSERT_FALSE(out.siren);
    TEST_ASSERT_EQUAL(ALARM_ARMED, out.status);
}

static void test_unlock_stops_siren(void)
{
    arm();
    in.door_open = true;
    at(25000U);
    in.locked = false;
    at(26000U);
    TEST_ASSERT_EQUAL(ALARM_DISARMED, out.status);
    TEST_ASSERT_FALSE(out.siren);
}

/* Portes verrouillees en roulant (contact mis) : l'alarme ne s'active pas */
static void test_no_arming_with_ignition_on(void)
{
    in.locked      = true;
    in.ignition_on = true;
    at(0U);
    at(30000U);
    TEST_ASSERT_EQUAL(ALARM_DISARMED, out.status);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_starts_disarmed);
    RUN_TEST(test_lock_starts_arming);
    RUN_TEST(test_armed_after_20s);
    RUN_TEST(test_door_during_arming_ignored);
    RUN_TEST(test_door_opened_when_armed_triggers);
    RUN_TEST(test_siren_stops_after_30s);
    RUN_TEST(test_unlock_stops_siren);
    RUN_TEST(test_no_arming_with_ignition_on);
    return UNITY_END();
}
