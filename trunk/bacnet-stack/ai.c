/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

// Analog Input Objects customize for your use

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "config.h" // the custom stuff

#define MAX_ANALOG_INPUTS 7

// we simply have 0-n object instances.  Yours might be
// more complex, and then you need validate that the
// given instance exists
bool Analog_Input_Valid_Instance(uint32_t object_instance)
{
  if (object_instance < MAX_ANALOG_INPUTS)
    return true;

  return false;
}

// we simply have 0-n object instances.  Yours might be
// more complex, and then count how many you have
unsigned Analog_Input_Count(void)
{
  return MAX_ANALOG_INPUTS;
}

// we simply have 0-n object instances.  Yours might be
// more complex, and then you need to return the instance
// that correlates to the correct index
uint32_t Analog_Input_Index_To_Instance(unsigned index)
{
  return index;
}

int Analog_Input_Encode_Property_APDU(
  uint8_t *apdu,
  uint32_t object_instance,
  BACNET_PROPERTY_ID property,
  int32_t array_index,
  BACNET_ERROR_CLASS *error_class,
  BACNET_ERROR_CODE *error_code)
{
  int apdu_len = 0; // return value
  BACNET_BIT_STRING bit_string;
  char text_string[32] = {""};
  float value = 3.141592;
  
  (void)array_index;
  switch (property)
  {
    case PROP_OBJECT_IDENTIFIER:
      apdu_len = encode_tagged_object_id(&apdu[0], OBJECT_ANALOG_INPUT,
        object_instance);
      break;
    case PROP_OBJECT_NAME:
    case PROP_DESCRIPTION:
      sprintf(text_string,"ANALOG INPUT %u",object_instance);
      apdu_len = encode_tagged_character_string(&apdu[0], text_string);
      break;
    case PROP_OBJECT_TYPE:
      apdu_len = encode_tagged_enumerated(&apdu[0], OBJECT_ANALOG_INPUT);
      break;
    case PROP_PRESENT_VALUE:
      apdu_len = encode_tagged_real(&apdu[0], value);
      break;
    case PROP_STATUS_FLAGS:
        bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
        bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
        bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
        bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, false);
        apdu_len = encode_tagged_bitstring(&apdu[0], &bit_string);
      break;
    case PROP_EVENT_STATE:
      apdu_len = encode_tagged_enumerated(&apdu[0],EVENT_STATE_NORMAL);
      break;
    case PROP_OUT_OF_SERVICE:
      apdu_len = encode_tagged_boolean(&apdu[0],false);
      break;
    case PROP_UNITS:
      apdu_len = encode_tagged_enumerated(&apdu[0],UNITS_PERCENT);
      break;
    default:
      *error_class = ERROR_CLASS_PROPERTY;
      *error_code = ERROR_CODE_UNKNOWN_PROPERTY;
      break;
  }

  return apdu_len;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

void testAnalogInput(Test * pTest)
{
  uint8_t apdu[MAX_APDU] = { 0 };
  int len = 0;
  uint32_t len_value = 0;
  uint8_t tag_number = 0;
  BACNET_OBJECT_TYPE decoded_type = OBJECT_ANALOG_OUTPUT;
  uint32_t decoded_instance = 0;
  uint32_t instance = 123;
  BACNET_ERROR_CLASS error_class;
  BACNET_ERROR_CODE error_code;


  // FIXME: we should do a lot more testing here...
  len = Analog_Input_Encode_Property_APDU(
    &apdu[0],
    instance,
    PROP_OBJECT_IDENTIFIER,
    BACNET_ARRAY_ALL,
    &error_class,
    &error_code);
  ct_test(pTest, len != 0);
  len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
  ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
  len = decode_object_id(&apdu[len],
        (int *) &decoded_type, &decoded_instance);
  ct_test(pTest, decoded_type == OBJECT_ANALOG_INPUT);
  ct_test(pTest, decoded_instance == instance);

  return;
}

#ifdef TEST_ANALOG_INPUT
int main(void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Analog Input", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testAnalogInput);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif                          /* TEST_ANALOG_INPUT */
#endif                          /* TEST */


