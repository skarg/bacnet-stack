/**
 * @file
 * @brief Unit test for object
 * @author Steve Karg <skarg@users.sourceforge.net>
 * @date June 2022
 *
 * SPDX-License-Identifier: MIT
 */
#include <ztest.h>
#include <bacnet/bactext.h>
#include <bacnet/basic/object/color_object.h>


bool Device_Valid_Object_Name(
    BACNET_CHARACTER_STRING * object_name,
    BACNET_OBJECT_TYPE *object_type,
    uint32_t * object_instance)
{
    return true;
}

void Device_Inc_Database_Revision(
    void)
{

}


/**
 * @addtogroup bacnet_tests
 * @{
 */

/**
 * @brief Test
 */
static void testColorObject(void)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    int test_len = 0;
    BACNET_READ_PROPERTY_DATA rpdata = {0};
    BACNET_APPLICATION_DATA_VALUE value = {0};
    const int *required_property = NULL;
    const uint32_t instance = 123;

    Color_Init();
    Color_Create(instance);

    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_COLOR;
    rpdata.object_instance = instance;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;

    Color_Property_Lists(&required_property, NULL, NULL);
    while ((*required_property) >= 0) {
        rpdata.object_property = *required_property;
        rpdata.array_index = BACNET_ARRAY_ALL;
        len = Color_Read_Property(&rpdata);
        zassert_true(len >= 0, NULL);
        if (len >= 0) {
            test_len = bacapp_decode_known_property(rpdata.application_data,
                len, &value, rpdata.object_property);
            if (len != test_len) {
                printf("property '%s': failed to decode!\n",
                    bactext_property_name(rpdata.object_property));
            }
            if ((rpdata.object_property == PROP_PRESENT_VALUE) ||
                (rpdata.object_property == PROP_COLOR_COMMAND) ||
                (rpdata.object_property == PROP_DEFAULT_COLOR) ||
                (rpdata.object_property == PROP_TRACKING_VALUE)) {
                /* FIXME: how to detect decoding REAL,REAL? */
                test_len = len;
            }
            zassert_equal(len, test_len, NULL);
        }
        required_property++;
    }

    return;
}
/**
 * @}
 */


void test_main(void)
{
    ztest_test_suite(color_object_tests,
     ztest_unit_test(testColorObject)
     );

    ztest_run_test_suite(color_object_tests);
}
