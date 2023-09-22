/*
 * Copyright (c) 2020 Legrand North America, LLC.
 *
 * SPDX-License-Identifier: MIT
 */

/* @file
 * @brief test BACnet integer encode/decode APIs
 */

#include <zephyr/ztest.h>
#include <bacnet/bacdef.h>
#include <bacnet/bactext.h>
#include <bacnet/lighting.h>

/**
 * @addtogroup bacnet_tests
 * @{
 */

/**
 * @brief Test
 */
static void testBACnetLightingCommand(BACNET_LIGHTING_COMMAND *data)
{
    bool status = false;
    BACNET_LIGHTING_COMMAND test_data;
    int len, apdu_len;
    uint8_t apdu[MAX_APDU] = { 0 };

    status = lighting_command_copy(&test_data, NULL);
    zassert_false(status, NULL);
    status = lighting_command_copy(NULL, data);
    zassert_false(status, NULL);
    status = lighting_command_copy(&test_data, data);
    zassert_true(status, NULL);
    status = lighting_command_same(&test_data, data);
    zassert_true(status, NULL);
    len = lighting_command_encode(apdu, data);
    apdu_len = lighting_command_decode(apdu, len, &test_data);
    zassert_true(len > 0, "lighting-command[%s] failed to encode!",
        bactext_lighting_operation_name(data->operation));
    zassert_true(apdu_len > 0, "lighting-command[%s] failed to decode!",
        bactext_lighting_operation_name(data->operation));
    status = lighting_command_same(&test_data, data);
}

#if defined(CONFIG_ZTEST_NEW_API)
ZTEST(lighting_tests, testBACnetLightingCommandAll)
#else
static void testBACnetLightingCommandAll(void)
#endif
{
    BACNET_LIGHTING_COMMAND data = { 0 };

    data.operation = BACNET_LIGHTS_NONE;
    data.use_target_level = false;
    data.use_ramp_rate = false;
    data.use_step_increment = false;
    data.use_fade_time = false;
    data.use_priority = false;
    data.target_level = 0.0;
    data.ramp_rate = 100.0;
    data.step_increment = 1.0;
    data.fade_time = 100;
    data.priority = 1;
    testBACnetLightingCommand(&data);

    data.operation = BACNET_LIGHTS_FADE_TO;
    data.use_target_level = true;
    data.target_level = 100.0;
    data.use_ramp_rate = false;
    data.use_step_increment = false;
    data.use_fade_time = true;
    data.fade_time = 100;
    data.use_priority = true;
    data.priority = 1;
    testBACnetLightingCommand(&data);

    data.operation = BACNET_LIGHTS_FADE_TO;
    data.use_target_level = true;
    data.target_level = 0.0;
    data.use_ramp_rate = false;
    data.use_step_increment = false;
    data.use_fade_time = false;
    data.use_priority = false;
    testBACnetLightingCommand(&data);
    
    data.operation = BACNET_LIGHTS_RAMP_TO;
    data.use_target_level = true;
    data.target_level = 0.0;
    data.use_step_increment = false;
    data.use_fade_time = false;
    data.use_ramp_rate = true;
    data.ramp_rate = 100.0;
    data.use_priority = true;
    data.priority = 1;
    testBACnetLightingCommand(&data);

    data.operation = BACNET_LIGHTS_RAMP_TO;
    data.use_target_level = true;
    data.target_level = 100.0;
    data.use_step_increment = false;
    data.use_fade_time = false;
    data.use_ramp_rate = false;
    data.use_priority = false;
    testBACnetLightingCommand(&data);

    data.operation = BACNET_LIGHTS_STEP_UP;
    data.use_target_level = false;
    data.use_ramp_rate = false;
    data.use_step_increment = true;
    data.step_increment = 1.0;
    data.use_fade_time = false;
    data.use_priority = false;
    testBACnetLightingCommand(&data);

    data.operation = BACNET_LIGHTS_STEP_UP;
    data.use_target_level = false;
    data.use_ramp_rate = false;
    data.use_step_increment = true;
    data.step_increment = 2.0;
    data.use_fade_time = false;
    data.use_priority = false;
    data.priority = 1;
    testBACnetLightingCommand(&data);

    data.operation = BACNET_LIGHTS_STEP_DOWN;
    data.use_target_level = false;
    data.use_ramp_rate = false;
    data.use_step_increment = true;
    data.step_increment = 1.0;
    data.use_fade_time = false;
    data.use_priority = false;
    testBACnetLightingCommand(&data);

    data.operation = BACNET_LIGHTS_STEP_DOWN;
    data.use_target_level = false;
    data.use_ramp_rate = false;
    data.use_step_increment = true;
    data.step_increment = 2.0;
    data.use_fade_time = false;
    data.use_priority = false;
    data.priority = 1;
    testBACnetLightingCommand(&data);

    data.operation = BACNET_LIGHTS_STEP_ON;
    data.use_target_level = false;
    data.use_ramp_rate = false;
    data.use_step_increment = true;
    data.step_increment = 1.0;
    data.use_fade_time = false;
    data.use_priority = false;
    testBACnetLightingCommand(&data);

    data.operation = BACNET_LIGHTS_STEP_ON;
    data.use_target_level = false;
    data.use_ramp_rate = false;
    data.use_step_increment = true;
    data.step_increment = 2.0;
    data.use_fade_time = false;
    data.use_priority = false;
    data.priority = 1;
    testBACnetLightingCommand(&data);

    data.operation = BACNET_LIGHTS_STEP_OFF;
    data.use_target_level = false;
    data.use_ramp_rate = false;
    data.use_step_increment = true;
    data.step_increment = 1.0;
    data.use_fade_time = false;
    data.use_priority = false;
    testBACnetLightingCommand(&data);

    data.operation = BACNET_LIGHTS_STEP_OFF;
    data.use_target_level = false;
    data.use_ramp_rate = false;
    data.use_step_increment = true;
    data.step_increment = 2.0;
    data.use_fade_time = false;
    data.use_priority = false;
    data.priority = 1;
    testBACnetLightingCommand(&data);

    data.operation = BACNET_LIGHTS_STOP;
    data.use_target_level = true;
    data.use_ramp_rate = true;
    data.use_step_increment = true;
    data.use_fade_time = true;
    data.use_priority = true;
    testBACnetLightingCommand(&data);
}
/**
 * @}
 */

/**
 * @brief Test
 */
static void testBACnetColorCommand(BACNET_COLOR_COMMAND *data)
{
    bool status = false;
    BACNET_COLOR_COMMAND test_data = { 0 };
    int len = 0, apdu_len = 0;
    uint8_t apdu[MAX_APDU] = { 0 };
    BACNET_ERROR_CODE error_code;

    status = color_command_copy(&test_data, NULL);
    zassert_false(status, NULL);
    status = color_command_copy(NULL, data);
    zassert_false(status, NULL);
    status = color_command_copy(&test_data, data);
    zassert_true(status, NULL);
    status = color_command_same(&test_data, data);
    zassert_true(status, NULL);
    len = color_command_encode(apdu, data);
    apdu_len =
        color_command_decode(apdu, len, &error_code, &test_data);
    zassert_true(len > 0, "color-command[%s] failed to encode!",
        bactext_color_operation_name(data->operation));
    zassert_true(apdu_len > 0, "color-command[%s] failed to decode!",
        bactext_color_operation_name(data->operation));
    status = color_command_same(&test_data, data);
}

#if defined(CONFIG_ZTEST_NEW_API)
ZTEST(lighting_tests, testBACnetColorCommandAll)
#else
static void testBACnetColorCommandAll(void)
#endif
{
    BACNET_COLOR_COMMAND data = { 0 };

    data.operation = BACNET_COLOR_OPERATION_NONE;
    data.target.color_temperature = 0;
    data.transit.fade_time = 0;
    testBACnetColorCommand(&data);
    data.operation = BACNET_COLOR_OPERATION_STOP;
    data.target.color_temperature = 0;
    data.transit.fade_time = 0;
    testBACnetColorCommand(&data);
    data.operation = BACNET_COLOR_OPERATION_FADE_TO_COLOR;
    data.target.color.x_coordinate = 0.0;
    data.target.color.y_coordinate = 0.0;
    data.transit.fade_time = 0;
    testBACnetColorCommand(&data);
    data.operation = BACNET_COLOR_OPERATION_FADE_TO_COLOR;
    data.target.color.x_coordinate = 0.0;
    data.target.color.y_coordinate = 0.0;
    data.transit.fade_time = 2000;
    testBACnetColorCommand(&data);
    data.operation = BACNET_COLOR_OPERATION_FADE_TO_CCT;
    data.target.color_temperature = 1800;
    data.transit.fade_time = 0;
    testBACnetColorCommand(&data);
    data.operation = BACNET_COLOR_OPERATION_FADE_TO_CCT;
    data.target.color_temperature = 1800;
    data.transit.fade_time = 2000;
    testBACnetColorCommand(&data);
    data.operation = BACNET_COLOR_OPERATION_RAMP_TO_CCT;
    data.target.color_temperature = 1800;
    data.transit.ramp_rate = 0;
    testBACnetColorCommand(&data);
    data.operation = BACNET_COLOR_OPERATION_RAMP_TO_CCT;
    data.target.color_temperature = 1800;
    data.transit.ramp_rate = 10;
    testBACnetColorCommand(&data);
    data.operation = BACNET_COLOR_OPERATION_STEP_UP_CCT;
    data.target.color_temperature = 1800;
    data.transit.step_increment = 0;
    testBACnetColorCommand(&data);
    data.operation = BACNET_COLOR_OPERATION_STEP_UP_CCT;
    data.target.color_temperature = 1800;
    data.transit.step_increment = 1;
    testBACnetColorCommand(&data);
    data.operation = BACNET_COLOR_OPERATION_STEP_DOWN_CCT;
    data.target.color_temperature = 1800;
    data.transit.step_increment = 0;
    testBACnetColorCommand(&data);
    data.operation = BACNET_COLOR_OPERATION_STEP_DOWN_CCT;
    data.target.color_temperature = 1800;
    data.transit.step_increment = 1;
    testBACnetColorCommand(&data);
}

#if defined(CONFIG_ZTEST_NEW_API)
ZTEST(lighting_tests, testBACnetXYColor)
#else
static void testBACnetXYColor(void)
#endif
{
    uint8_t apdu[MAX_APDU] = { 0 };
    BACNET_XY_COLOR value = { 0 };
    BACNET_XY_COLOR test_value = { 0 };
    int len = 0, test_len = 0, null_len = 0;
    uint8_t tag_number = 1;
    bool status = false;

    value.x_coordinate = 1.0;
    value.y_coordinate = 1.0;

    null_len = xy_color_encode(NULL, &value);
    len = xy_color_encode(apdu, &value);
    zassert_equal(null_len, len, NULL);
    test_len = xy_color_decode(apdu, sizeof(apdu), &test_value);
    zassert_equal(test_len, len, NULL);
    status = xy_color_same(&value, &test_value);
    zassert_true(status, NULL);

    null_len = xy_color_context_encode(NULL, tag_number, &value);
    len = xy_color_context_encode(apdu, tag_number, &value);
    zassert_equal(null_len, len, NULL);
    test_len =
        xy_color_context_decode(apdu, sizeof(apdu), tag_number, &test_value);
    zassert_equal(test_len, len, NULL);
    status = xy_color_same(&value, &test_value);
    zassert_true(status, NULL);
}

#if defined(CONFIG_ZTEST_NEW_API)
ZTEST_SUITE(lighting_tests, NULL, NULL, NULL, NULL, NULL);
#else
void test_main(void)
{
    ztest_test_suite(lighting_tests,
        ztest_unit_test(testBACnetLightingCommandAll),
        ztest_unit_test(testBACnetColorCommandAll),
        ztest_unit_test(testBACnetXYColor));

    ztest_run_test_suite(lighting_tests);
}
#endif
