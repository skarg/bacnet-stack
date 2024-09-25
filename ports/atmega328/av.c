/**
 * @brief This module manages the BACnet Analog Value objects
 * @author Steve Karg <skarg@users.sourceforge.net>
 * @date 2007
 * @copyright SPDX-License-Identifier: MIT
 */
#include <stdbool.h>
#include <stdint.h>
#include "hardware.h"
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacenum.h"
#include "bacnet/bacapp.h"
#include "bacnet/config.h" /* the custom stuff */
#include "bacnet/wp.h"
#include "bacnet/basic/object/av.h"

#ifndef MAX_ANALOG_VALUES
#define MAX_ANALOG_VALUES 10
#endif

static float Present_Value[MAX_ANALOG_VALUES];
static const char *Object_Name[MAX_ANALOG_VALUES] = {
    "AV-0", "AV-1", "AV-2", "AV-3", "AV-4",
    "AV-5", "AV-6", "AV-7", "AV-8", "AV-9"
};
static uint16_t Engineering_Units[MAX_ANALOG_VALUES] = {
    UNITS_NO_UNITS, UNITS_NO_UNITS, UNITS_NO_UNITS, UNITS_NO_UNITS,
    UNITS_NO_UNITS, UNITS_NO_UNITS, UNITS_NO_UNITS, UNITS_NO_UNITS,
    UNITS_NO_UNITS, UNITS_NO_UNITS
};

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Analog_Value_Valid_Instance(uint32_t object_instance)
{
    if (object_instance < MAX_ANALOG_VALUES) {
        return true;
    }

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Analog_Value_Count(void)
{
    return MAX_ANALOG_VALUES;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Analog_Value_Index_To_Instance(unsigned index)
{
    return index;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Analog_Value_Instance_To_Index(uint32_t object_instance)
{
    return object_instance;
}

/**
 * For a given object instance-number, sets the object-name
 *
 * @param  object_instance - object-instance number of the object
 * @param  new_name - holds the object-name to be set
 *
 * @return  true if object-name was set
 */
bool Analog_Value_Name_Set(uint32_t object_instance, const char *value)
{
    if (object_instance < MAX_ANALOG_VALUES) {
        Object_Name[object_instance] = value;
    }

    return true;
}

/**
 * @brief Return the object name C string
 * @param object_instance [in] BACnet object instance number
 * @return object name or NULL if not found
 */
const char *Analog_Value_Name_ASCII(uint32_t object_instance)
{
    const char *object_name = "AV-X";

    if (object_instance < MAX_ANALOG_VALUES) {
        object_name = Object_Name[object_instance];
    }

    return object_name;
}

float Analog_Value_Present_Value(uint32_t object_instance)
{
    float value = 0.0;

    if (object_instance < MAX_ANALOG_VALUES) {
        value = Present_Value[object_instance];
    }

    return value;
}

bool Analog_Value_Present_Value_Set(uint32_t object_instance, float value,
    uint8_t priority)
{
    (void)priority;
    if (object_instance < MAX_ANALOG_VALUES) {
        Present_Value[object_instance] = value;
    }

    return true;
}

uint16_t Analog_Value_Units(uint32_t instance)
{
    uint16_t units = UNITS_NO_UNITS;

    if (instance < MAX_ANALOG_VALUES) {
        units = Engineering_Units[instance];
    }

    return units;
}

bool Analog_Value_Units_Set(uint32_t instance, uint16_t unit)
{
    if (instance < MAX_ANALOG_VALUES) {
        Engineering_Units[instance] = unit;
        return true;
    }

    return false;
}


/* return apdu len, or -1 on error */
int Analog_Value_Read_Property(BACNET_READ_PROPERTY_DATA *rpdata)
{
    int apdu_len = 0; /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    uint8_t *apdu;

    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(
                &apdu[0], OBJECT_ANALOG_VALUE, rpdata->object_instance);
            break;
        case PROP_OBJECT_NAME:
            characterstring_init_ansi(
                &char_string, Analog_Value_Name_ASCII(rpdata->object_instance));
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_ANALOG_VALUE);
            break;
        case PROP_PRESENT_VALUE:
            apdu_len = encode_application_real(
                &apdu[0], Analog_Value_Present_Value(
                rpdata->object_instance));
            break;
        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, false);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_EVENT_STATE:
            apdu_len =
                encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
            break;
        case PROP_OUT_OF_SERVICE:
            apdu_len = encode_application_boolean(&apdu[0], false);
            break;
        case PROP_UNITS:
            apdu_len = encode_application_enumerated(&apdu[0],
                Analog_Value_Units(rpdata->object_instance));
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

/* returns true if successful */
bool Analog_Value_Write_Property(BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    bool status = false; /* return value */
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;

    if (!Analog_Value_Valid_Instance(wp_data->object_instance)) {
        wp_data->error_class = ERROR_CLASS_OBJECT;
        wp_data->error_code = ERROR_CODE_UNKNOWN_OBJECT;
        return false;
    }
    /* decode the some of the request */
    len = bacapp_decode_application_data(
        wp_data->application_data, wp_data->application_data_len, &value);
    /* FIXME: len < application_data_len: more data? */
    if (len < 0) {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    if ((wp_data->object_property != PROP_PRIORITY_ARRAY) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            if (value.tag == BACNET_APPLICATION_TAG_REAL) {
                status = Analog_Value_Present_Value_Set(
                    wp_data->object_instance, value.type.Real,
                    wp_data->priority);
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        case PROP_UNITS:
            if (value.tag == BACNET_APPLICATION_TAG_ENUMERATED) {
                status = Analog_Value_Units_Set(
                    wp_data->object_instance, value.type.Enumerated);
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_OBJECT_TYPE:
        case PROP_STATUS_FLAGS:
        case PROP_EVENT_STATE:
        case PROP_OUT_OF_SERVICE:
        case PROP_DESCRIPTION:
        case PROP_PRIORITY_ARRAY:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;
    }

    return status;
}
