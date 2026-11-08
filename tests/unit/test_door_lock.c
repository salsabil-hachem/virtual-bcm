/*
 * test_door_lock.c
 * Verifie les regles du verrouillage des portes.
 */
#include "unity.h"
#include "door_lock.h"

static door_state_t   state;
static door_inputs_t  in;
static door_outputs_t out;

void setUp(void)
{
    door_init(&state);
    in.key_lock    = false;
    in.key_unlock  = false;
    in.speed_kmh   = 0U;
    in.ignition_on = true;
    door_update(&state, &in, &out);   /* 1er appel : le contact est mis */
}

void tearDown(void) {}

/* Raccourci : un appel de la fonction */
static void step(void)
{
    door_update(&state, &in, &out);
}

/* Raccourci : appuyer puis relacher un bouton */
static void press(bool *button)
{
    *button = true;
    step();
    *button = false;
    step();
}

static void test_starts_unlocked(void)
{
    TEST_ASSERT_FALSE(out.locked);
}

static void test_lock_button_locks(void)
{
    press(&in.key_lock);
    TEST_ASSERT_TRUE(out.locked);
}

static void test_unlock_button_unlocks(void)
{
    press(&in.key_lock);
    press(&in.key_unlock);
    TEST_ASSERT_FALSE(out.locked);
}

static void test_no_autolock_at_10_kmh(void)
{
    in.speed_kmh = 10U;
    step();
    TEST_ASSERT_FALSE(out.locked);
}

static void test_autolock_at_20_kmh(void)
{
    in.speed_kmh = 20U;
    step();
    TEST_ASSERT_TRUE(out.locked);
}

/* Le conducteur deverrouille en roulant : on ne reverrouille PAS en boucle */
static void test_autolock_only_once_per_trip(void)
{
    in.speed_kmh = 20U;
    step();
    press(&in.key_unlock);
    step();
    TEST_ASSERT_FALSE(out.locked);
}

static void test_ignition_off_unlocks(void)
{
    in.speed_kmh = 20U;
    step();
    in.speed_kmh   = 0U;
    in.ignition_on = false;
    step();
    TEST_ASSERT_FALSE(out.locked);
}

/* Nouveau trajet : l'autolock fonctionne de nouveau */
static void test_new_trip_autolocks_again(void)
{
    in.speed_kmh = 20U;
    step();
    in.speed_kmh   = 0U;
    in.ignition_on = false;
    step();
    in.ignition_on = true;
    step();
    in.speed_kmh = 30U;
    step();
    TEST_ASSERT_TRUE(out.locked);
}

/* Bouton maintenu appuye : compte comme UN seul appui */
static void test_held_button_counts_once(void)
{
    in.key_lock = true;
    step();
    step();
    in.key_lock = false;
    press(&in.key_unlock);
    TEST_ASSERT_FALSE(out.locked);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_starts_unlocked);
    RUN_TEST(test_lock_button_locks);
    RUN_TEST(test_unlock_button_unlocks);
    RUN_TEST(test_no_autolock_at_10_kmh);
    RUN_TEST(test_autolock_at_20_kmh);
    RUN_TEST(test_autolock_only_once_per_trip);
    RUN_TEST(test_ignition_off_unlocks);
    RUN_TEST(test_new_trip_autolocks_again);
    RUN_TEST(test_held_button_counts_once);
    return UNITY_END();
}
