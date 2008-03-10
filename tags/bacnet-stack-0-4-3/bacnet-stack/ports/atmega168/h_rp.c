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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "bacerror.h"
#include "apdu.h"
#include "npdu.h"
#include "abort.h"
#include "rp.h"
/* demo objects */
#include "device.h"
#include "av.h"
#if 0
#include "ai.h"
#include "bi.h"
#include "bv.h"
#endif

/* Encodes the property APDU and returns the length,
   or sets the error, and returns -1 */
int Encode_Property_APDU(
    uint8_t * apdu,
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    BACNET_PROPERTY_ID property,
    int32_t array_index,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code)
{
    int apdu_len = -1;

    /* initialize the default return values */
    *error_class = ERROR_CLASS_OBJECT;
    *error_code = ERROR_CODE_UNKNOWN_OBJECT;
    /* handle each object type */
    switch (object_type) {
        case OBJECT_DEVICE:
            if (Device_Valid_Object_Instance_Number(object_instance)) {
                apdu_len =
                    Device_Encode_Property_APDU(&apdu[0], property,
                    array_index, error_class, error_code);
            }
            break;
        case OBJECT_ANALOG_VALUE:
            if (Analog_Value_Valid_Instance(object_instance)) {
                apdu_len =
                    Analog_Value_Encode_Property_APDU(&apdu[0],
                    object_instance, property, array_index, error_class,
                    error_code);
            }
            break;
        default:
            *error_class = ERROR_CLASS_OBJECT;
            *error_code = ERROR_CODE_UNSUPPORTED_OBJECT_TYPE;
            break;
    }

    return apdu_len;
}

void handler_read_property(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    BACNET_READ_PROPERTY_DATA data;
    int len = 0;
    int ack_len = 0;
    int property_len = 0;
    int pdu_len = 0;
    BACNET_NPDU_DATA npdu_data;
    bool error = false;
    int bytes_sent = 0;
    BACNET_ERROR_CLASS error_class = ERROR_CLASS_OBJECT;
    BACNET_ERROR_CODE error_code = ERROR_CODE_UNKNOWN_OBJECT;
    BACNET_ADDRESS my_address;

    len = rp_decode_service_request(service_request, service_len, &data);
    /* encode the NPDU portion of the packet */
    datalink_get_my_address(&my_address);
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], src, &my_address,
        &npdu_data);
    if (len < 0) {
        /* bad decoding - send an abort */
        len =
            abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_OTHER, true);
    } else if (service_data->segmented_message) {
        /* we don't support segmentation - send an abort */
        len =
            abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_SEGMENTATION_NOT_SUPPORTED,
            true);
    } else {
        /* most cases will be error */
        error = true;
        ack_len =
            rp_ack_encode_apdu_init(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, &data);
        /* FIXME: add buffer len as passed into function or use smart buffer */
        property_len =
            Encode_Property_APDU(&Handler_Transmit_Buffer[pdu_len + ack_len],
            data.object_type, data.object_instance, data.object_property,
            data.array_index, &error_class, &error_code);
        if (len >= 0) {
            len =
                rp_ack_encode_apdu_object_property_end(&Handler_Transmit_Buffer
                [pdu_len + property_len + ack_len]);
            len += ack_len + property_len;
            error = false;
        }
    }
    if (error) {
        switch (len) {
                /* BACnet APDU too small to fit data, so proper response is Abort */
            case -2:
                len =
                    abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id,
                    ABORT_REASON_SEGMENTATION_NOT_SUPPORTED, true);
                break;
            case -1:
            default:
                len =
                    bacerror_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id, SERVICE_CONFIRMED_READ_PROPERTY,
                    error_class, error_code);
                break;
        }
    }
    pdu_len += len;
    bytes_sent =
        datalink_send_pdu(src, &npdu_data, &Handler_Transmit_Buffer[0],
        pdu_len);

    return;
}
