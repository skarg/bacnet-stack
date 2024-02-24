/**
 * @file
 * @brief Unit test for object property read/write
 * @author Steve Karg <skarg@users.sourceforge.net>
 * @date February 2024
 *
 * SPDX-License-Identifier: MIT
 */
#include <zephyr/ztest.h>
#include <bacnet/bactext.h>
#include <bacnet/rp.h>
#include <bacnet/rpm.h>
#include <bacnet/wp.h>

/**
 * @brief
 *
 * @param rpdata
 * @param read_property
 * @param write_property
 * @param known_fail_property_list
 */
void bacnet_object_property_read_write_test(
    BACNET_READ_PROPERTY_DATA *rpdata,
    read_property_function read_property,
    write_property_function write_property,
    const int *known_fail_property_list)
{
    bool status = false;
    int len = 0;
    int test_len = 0;
    BACNET_WRITE_PROPERTY_DATA wpdata = { 0 };
    BACNET_APPLICATION_DATA_VALUE value = { 0 };

    len = read_property(rpdata);
    zassert_not_equal(len, BACNET_STATUS_ERROR, NULL);
    if (len >= 0) {
        test_len = bacapp_decode_known_property(rpdata->application_data,
            (uint8_t)rpdata->application_data_len, &value, rpdata->object_type,
            rpdata->object_property);
        if (len != test_len) {
            printf("property '%s': failed to decode! %d!=%d\n",
                bactext_property_name(rpdata->object_property), test_len, len);
        }
        if (property_list_member(
                known_fail_property_list, rpdata->object_property)) {
            /* FIXME: known fail to decode */
            test_len = len;
        }
        zassert_true(test_len == len, NULL);
        /* check WriteProperty properties */
        wpdata.object_type = rpdata->object_type;
        wpdata.object_instance = rpdata->object_instance;
        wpdata.object_property = rpdata->object_property;
        wpdata.array_index = rpdata->array_index;
        memcpy(&wpdata.application_data, rpdata->application_data, MAX_APDU);
        wpdata.application_data_len = len;
        wpdata.error_code = ERROR_CODE_SUCCESS;
        status = write_property(&wpdata);
        if (!status) {
            /* verify WriteProperty property is known */
            zassert_not_equal(wpdata.error_code, ERROR_CODE_UNKNOWN_PROPERTY,
                "property '%s': WriteProperty Unknown!\n",
                bactext_property_name(rpdata->object_property));
        }
    } else {
        printf("property '%s': failed to read(%d)!\n",
            bactext_property_name(rpdata->object_property), len);
    }
}

/**
 * @brief Test all the properties of an object for read/write
 *
 * @param object_type
 * @param object_instance
 * @param property_list
 * @param read_property
 * @param write_property
 * @param known_fail_property_list
 */
void bacnet_object_properties_read_write_test(BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    rpm_property_lists_function property_list,
    read_property_function read_property,
    write_property_function write_property,
    const int *known_fail_property_list)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    BACNET_READ_PROPERTY_DATA rpdata = { 0 };
    const int *pRequired = NULL;
    const int *pOptional = NULL;
    const int *pProprietary = NULL;
    unsigned count = 0;

    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = object_type;
    rpdata.object_instance = object_instance;
    property_list(&pRequired, &pOptional, &pProprietary);
    while ((*pRequired) != -1) {
        rpdata.object_property = *pRequired;
        rpdata.array_index = BACNET_ARRAY_ALL;
        bacnet_object_property_read_write_test(
            &rpdata, read_property, write_property, known_fail_property_list);
        pRequired++;
    }
    while ((*pOptional) != -1) {
        rpdata.object_property = *pOptional;
        rpdata.array_index = BACNET_ARRAY_ALL;
        bacnet_object_property_read_write_test(
            &rpdata, read_property, write_property, known_fail_property_list);
        pOptional++;
    }
    while ((*pProprietary) != -1) {
        rpdata.object_property = *pProprietary;
        rpdata.array_index = BACNET_ARRAY_ALL;
        bacnet_object_property_read_write_test(
            &rpdata, read_property, write_property, known_fail_property_list);
        pProprietary++;
    }
}
