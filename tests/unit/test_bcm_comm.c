/*
 * test_bcm_comm.c
 * Verifie la traduction octets <-> entrees/sorties du BCM.
 */
#include "unity.h"
#include "bcm_comm.h"

static bcm_comm_t comm;

void setUp(void)
{
    comm_init(&comm);
}

void tearDown(void) {}

static void test_dash_inputs_decoded(void)
{
    const uint8_t data[5] = { 1U, 1U, 0U, 0U, 1U };   /* levier gauche, warning, contact */
    TEST_ASSERT_TRUE(comm_receive(&comm, CAN_ID_DASH_INPUTS, data, 5U, 0U));
    TEST_ASSERT_EQUAL(LEVER_LEFT, comm.inputs.lever);
    TEST_ASSERT_TRUE(comm.inputs.hazard);
    TEST_ASSERT_TRUE(comm.inputs.ignition_on);
}

static void test_invalid_lever_rejected(void)
{
    const uint8_t data[5] = { 7U, 0U, 0U, 0U, 0U };
    TEST_ASSERT_FALSE(comm_receive(&comm, CAN_ID_DASH_INPUTS, data, 5U, 0U));
    TEST_ASSERT_EQUAL(LEVER_OFF, comm.inputs.lever);
}

static void test_too_short_message_rejected(void)
{
    const uint8_t data[2] = { 1U, 0U };
    TEST_ASSERT_FALSE(comm_receive(&comm, CAN_ID_DASH_INPUTS, data, 2U, 0U));
}

/* 0x78 0x00 = 120 km/h (octet faible en premier) */
static void test_speed_little_endian(void)
{
    const uint8_t data[2] = { 0x78U, 0x00U };
    TEST_ASSERT_TRUE(comm_receive(&comm, CAN_ID_VEH_SPEED, data, 2U, 0U));
    TEST_ASSERT_EQUAL_UINT16(120U, comm.inputs.speed_kmh);
}

/* 0xE8 0x03 = 1000 lux */
static void test_light_decoded(void)
{
    const uint8_t data[2] = { 0xE8U, 0x03U };
    TEST_ASSERT_TRUE(comm_receive(&comm, CAN_ID_ENV_LIGHT, data, 2U, 0U));
    TEST_ASSERT_EQUAL_UINT16(1000U, comm.inputs.light_lux);
}

static void test_bulb_failure_decoded(void)
{
    const uint8_t data[2] = { 1U, 0U };
    TEST_ASSERT_TRUE(comm_receive(&comm, CAN_ID_LAMP_DIAG, data, 2U, 0U));
    TEST_ASSERT_FALSE(comm.inputs.bulb_ok_left);
    TEST_ASSERT_TRUE(comm.inputs.bulb_ok_right);
}

static void test_unknown_id_ignored(void)
{
    const uint8_t data[1] = { 0U };
    TEST_ASSERT_FALSE(comm_receive(&comm, 0x555U, data, 1U, 0U));
}

static void test_safe_mode_when_no_dash_at_startup(void)
{
    TEST_ASSERT_FALSE(comm_dash_lost(&comm, 100U));
    TEST_ASSERT_TRUE(comm_dash_lost(&comm, 600U));
}

static void test_safe_mode_after_500ms_silence(void)
{
    const uint8_t data[5] = { 0U, 0U, 0U, 0U, 1U };
    (void)comm_receive(&comm, CAN_ID_DASH_INPUTS, data, 5U, 1000U);
    TEST_ASSERT_FALSE(comm_dash_lost(&comm, 1400U));
    TEST_ASSERT_TRUE(comm_dash_lost(&comm, 1500U));
}

static void test_safe_mode_forces_headlights_and_fault(void)
{
    bcm_outputs_t out = { false, false, false, false, false, ALARM_DISARMED };
    uint8_t lamps[CAN_DLC_BCM_LAMPS];
    uint8_t status[CAN_DLC_BCM_STATUS];
    comm_encode_lamps(&out, true, lamps);
    comm_encode_status(&out, true, status);
    TEST_ASSERT_EQUAL_UINT8(1U, lamps[2]);
    TEST_ASSERT_EQUAL_UINT8(FAULT_DASH_LOST, status[2]);
}

static void test_encode_lamps(void)
{
    bcm_outputs_t out = { true, false, true, true, true, ALARM_TRIGGERED };
    uint8_t lamps[CAN_DLC_BCM_LAMPS];
    uint8_t status[CAN_DLC_BCM_STATUS];
    comm_encode_lamps(&out, false, lamps);
    comm_encode_status(&out, false, status);
    const uint8_t expected_lamps[4]  = { 1U, 0U, 1U, 1U };
    const uint8_t expected_status[3] = { 1U, 3U, 0U };
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_lamps, lamps, 4);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_status, status, 3);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_dash_inputs_decoded);
    RUN_TEST(test_invalid_lever_rejected);
    RUN_TEST(test_too_short_message_rejected);
    RUN_TEST(test_speed_little_endian);
    RUN_TEST(test_light_decoded);
    RUN_TEST(test_bulb_failure_decoded);
    RUN_TEST(test_unknown_id_ignored);
    RUN_TEST(test_safe_mode_when_no_dash_at_startup);
    RUN_TEST(test_safe_mode_after_500ms_silence);
    RUN_TEST(test_safe_mode_forces_headlights_and_fault);
    RUN_TEST(test_encode_lamps);
    return UNITY_END();
}
