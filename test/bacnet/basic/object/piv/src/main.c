/*
 * Copyright (c) 2020 Legrand North America, LLC.
 *
 * SPDX-License-Identifier: MIT
 */

/* @file
 * @brief test BACnet integer encode/decode APIs
 */

#include <zephyr/ztest.h>
#include <bacnet/basic/object/piv.h>

/**
 * @addtogroup bacnet_tests
 * @{
 */

/**
 * @brief Test
 */
static void testPositiveInteger_Value(void)
{
    BACNET_READ_PROPERTY_DATA rpdata;
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0, test_len;
    BACNET_OBJECT_TYPE decoded_type = 0;
    uint32_t decoded_instance = 0;

    PositiveInteger_Value_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_POSITIVE_INTEGER_VALUE;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = PositiveInteger_Value_Read_Property(&rpdata);
    zassert_not_equal(len, 0, NULL);
    test_len = bacnet_object_id_application_decode(
        apdu, len, &decoded_type, &decoded_instance);
    zassert_not_equal(test_len, BACNET_STATUS_ERROR, NULL);
    zassert_equal(decoded_type, rpdata.object_type, NULL);
    zassert_equal(decoded_instance, rpdata.object_instance, NULL);

    return;
}
/**
 * @}
 */

void test_main(void)
{
    ztest_test_suite(piv_tests, ztest_unit_test(testPositiveInteger_Value));

    ztest_run_test_suite(piv_tests);
}
