/**************************************************************************
 *
 * Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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
#include <stdlib.h>
#include <signal.h>
#include "bacnet/config.h"
#include "bacnet/basic/binding/address.h"
#include "bacnet/bacdef.h"
#include "bacnet/basic/services.h"
#include "bacnet/basic/services.h"
#include "bacnet/datalink/dlenv.h"
#include "bacnet/bacdcode.h"
#include "bacnet/npdu.h"
#include "bacnet/apdu.h"
#include "bacnet/iam.h"
#include "bacnet/basic/tsm/tsm.h"
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/object/bacfile.h"
#include "bacnet/datalink/datalink.h"
#include "bacnet/dcc.h"
#include "bacnet/getevent.h"
#include "bacnet/lighting.h"
#include "bacport.h"
#include "bacnet/basic/sys/mstimer.h"
#include "bacnet/basic/sys/color_rgb.h"
#include "bacnet/basic/sys/filename.h"
#include "bacnet/basic/tsm/tsm.h"
#include "bacnet/version.h"
/* include the device object */
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/object/bi.h"
#include "bacnet/basic/object/bo.h"
#include "bacnet/basic/object/channel.h"
#include "bacnet/basic/object/color_object.h"
#include "bacnet/basic/object/color_temperature.h"
#include "blinkt.h"

/** @file blinkt/main.c  Example application using the BACnet Stack. */

/* (Doxygen note: The next two lines pull all the following Javadoc
 *  into the ServerDemo module.) */
/** @addtogroup ServerDemo */
/*@{*/

/** Buffer used for receiving */
static uint8_t Rx_Buf[MAX_MPDU] = { 0 };
/* current version of the BACnet stack */
static const char *BACnet_Version = BACNET_VERSION_TEXT;
/* task timer for various BACnet timeouts */
static struct mstimer BACnet_Task_Timer;
/* task timer for TSM timeouts */
static struct mstimer BACnet_TSM_Timer;
/* task timer for address binding timeouts */
static struct mstimer BACnet_Address_Timer;
/* task timer for fading colors */
static struct mstimer BACnet_Fade_Timer;

/** Initialize the handlers we will utilize.
 * @see Device_Init, apdu_set_unconfirmed_handler, apdu_set_confirmed_handler
 */
static void Init_Service_Handlers(void)
{
    Device_Init(NULL);
    /* we need to handle who-is to support dynamic device binding */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_HAS, handler_who_has);
    /* handle i-am to support binding to other devices */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);
    /* set the handler for all the services we don't implement */
    /* It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler(handler_unrecognized_service);
    /* Set the handlers for any confirmed services that we support. */
    /* We must implement read property - it's required! */
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_READ_PROPERTY, handler_read_property);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_READ_PROP_MULTIPLE, handler_read_property_multiple);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_WRITE_PROPERTY, handler_write_property);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_WRITE_PROP_MULTIPLE, handler_write_property_multiple);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_READ_RANGE, handler_read_range);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_REINITIALIZE_DEVICE, handler_reinitialize_device);
    apdu_set_unconfirmed_handler(
        SERVICE_UNCONFIRMED_UTC_TIME_SYNCHRONIZATION, handler_timesync_utc);
    apdu_set_unconfirmed_handler(
        SERVICE_UNCONFIRMED_TIME_SYNCHRONIZATION, handler_timesync);
    apdu_set_confirmed_handler(
        SERVICE_CONFIRMED_SUBSCRIBE_COV, handler_cov_subscribe);
    apdu_set_unconfirmed_handler(
        SERVICE_UNCONFIRMED_COV_NOTIFICATION, handler_ucov_notification);
    /* handle communication so we can shutup when asked */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL,
        handler_device_communication_control);
    /* handle the data coming back from private requests */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_PRIVATE_TRANSFER,
        handler_unconfirmed_private_transfer);
    /* configure the cyclic timers */
    mstimer_set(&BACnet_Task_Timer, 1000UL);
    mstimer_set(&BACnet_TSM_Timer, 50UL);
    mstimer_set(&BACnet_Address_Timer, 60UL*1000UL);
    mstimer_set(&BACnet_Fade_Timer, 100UL);
}

/**
 * Clean up the Blinkt! interface
 */
static void blinkt_cleanup(void)
{
    blinkt_stop();
}

/**
 * @brief Callback for tracking value
 * @param  object_instance - object-instance number of the object
 * @param  old_value - BACnetXYColor value prior to write
 * @param  value - BACnetXYColor value of the write
 */
static void Color_Write_Value_Handler(uint32_t object_instance,
    BACNET_XY_COLOR *old_value,
    BACNET_XY_COLOR *value)
{
    uint8_t red, green, blue;
    uint8_t brightness = 255;
    uint8_t index = 255;

    (void)old_value;
    if (object_instance > 0) {
        index = object_instance - 1;
    }
    if (index < blinkt_led_count()) {
        color_rgb_from_xy(&red, &green, &blue,
            value->x_coordinate, value->y_coordinate,
            brightness);
        blinkt_set_pixel(index, red, green, blue);
        blinkt_show();
        printf("RGB[%u]=%u,%u,%u\n", (unsigned)index,
            (unsigned)red, (unsigned)green, (unsigned)blue);
    }
}

/**
 * @brief Create the objects and configure the callbacks for BACnet objects
 */
static void bacnet_output_init(void)
{
    unsigned i = 0;
    uint8_t led_max;
    uint32_t object_instance = 1;

    led_max = blinkt_led_count();
    for (i = 0; i < led_max; i++) {
        Color_Create(object_instance);
        Color_Write_Enable(object_instance);
        object_instance++;
    }
    Color_Write_Present_Value_Callback_Set(Color_Write_Value_Handler);
}

/**
 * @brief Manage the cyclic tasks for BACnet objects
 */
static void bacnet_output_task(void)
{
    unsigned i = 0;
    uint8_t led_max;
    uint32_t object_instance = 1;
    unsigned long milliseconds;

    if (mstimer_expired(&BACnet_Fade_Timer)) {
        mstimer_reset(&BACnet_Fade_Timer);
        milliseconds = mstimer_interval(&BACnet_Fade_Timer);
        led_max = blinkt_led_count();
        for (i = 0; i < led_max; i++) {
            Color_Object_Timer(object_instance, milliseconds);
            object_instance++;
        }
    }
}

/**
 * @brief Print the terse usage info
 * @param filename - this application file name
 */
static void print_usage(const char *filename)
{
    printf("Usage: %s [device-instance]\n", filename);
    printf("       [--device N][--test]\n");
    printf("       [--version][--help]\n");
}

/**
 * @brief Print the verbose usage info
 * @param filename - this application file name
 */
static void print_help(const char *filename)
{
    printf("BACnet Blinkt! server device.\n");
    printf("device-instance:\n"
           "--device N:\n"
           "BACnet Device Object Instance number of this device.\n"
           "This number will be used when other devices\n"
           "try and bind with this device using Who-Is and\n"
           "I-Am services.\n");
    printf("\n");
    printf("--test:\n"
           "Test the Blinkt! RGB LEDs with a cycling pattern.\n");
    printf("\n");
    printf("Example:\n"
           "%s 9009\n", filename);
}

/** Main function of server demo.
 *
 * @see Device_Set_Object_Instance_Number, dlenv_init, Send_I_Am,
 *      datalink_receive, npdu_handler,
 *      dcc_timer_seconds, datalink_maintenance_timer,
 *      handler_cov_task,
 *      tsm_timer_milliseconds
 *
 * @param argc [in] Arg count.
 * @param argv [in] Takes one argument: the Device Instance #.
 * @return 0 on success.
 */
int main(int argc, char *argv[])
{
    BACNET_ADDRESS src = { 0 }; /* address where message came from */
    uint16_t pdu_len = 0;
    unsigned timeout_ms = 1;
    unsigned long seconds = 0;
    bool blinkt_test = false;
    unsigned int target_args = 0;
    uint32_t device_id = BACNET_MAX_INSTANCE;
    int argi = 0;
    char *filename = NULL;

    filename = filename_remove_path(argv[0]);
    for (argi = 1; argi < argc; argi++) {
        if (strcmp(argv[argi], "--help") == 0) {
            print_usage(filename);
            print_help(filename);
            return 0;
        }
        if (strcmp(argv[argi], "--version") == 0) {
            printf("%s %s\n", filename, BACNET_VERSION_TEXT);
            printf("Copyright (C) 2023 by Steve Karg and others.\n"
                   "This is free software; see the source for copying "
                   "conditions.\n"
                   "There is NO warranty; not even for MERCHANTABILITY or\n"
                   "FITNESS FOR A PARTICULAR PURPOSE.\n");
            return 0;
        }
        if (strcmp(argv[argi], "--device") == 0) {
            if (++argi < argc) {
                device_id = strtol(argv[argi], NULL, 0);
            }
        } else if (strcmp(argv[argi], "--test") == 0) {
            blinkt_test = true;
        } else {
            if (target_args == 0) {
                device_id = strtol(argv[argi], NULL, 0);
                target_args++;
            }
        }
    }
    if (device_id > BACNET_MAX_INSTANCE) {
        fprintf(stderr, "device=%u - it must be less than %u\n",
            device_id, BACNET_MAX_INSTANCE);
        return 1;
    }
    Device_Set_Object_Instance_Number(device_id);
    printf("BACnet Raspberry Pi Blinkt! Demo\n"
           "BACnet Stack Version %s\n"
           "BACnet Device ID: %u\n"
           "Max APDU: %d\n",
        BACnet_Version, Device_Object_Instance_Number(), MAX_APDU);
    /* load any static address bindings to show up
       in our device bindings list */
    address_init();
    Init_Service_Handlers();
    dlenv_init();
    atexit(datalink_cleanup);
    blinkt_init();
    atexit(blinkt_cleanup);
    bacnet_output_init();
    /* configure the timeout values */
    /* broadcast an I-Am on startup */
    Send_I_Am(&Handler_Transmit_Buffer[0]);
    /* loop forever */
    for (;;) {
        /* input */
        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout_ms);
        /* process */
        if (pdu_len) {
            npdu_handler(&src, &Rx_Buf[0], pdu_len);
        }
        if (mstimer_expired(&BACnet_Task_Timer)) {
            mstimer_reset(&BACnet_Task_Timer);
            /* 1 second tasks */
            dcc_timer_seconds(1);
            datalink_maintenance_timer(1);
            dlenv_maintenance_timer(1);
            handler_cov_timer_seconds(1);
        }
        if (mstimer_expired(&BACnet_TSM_Timer)) {
            mstimer_reset(&BACnet_TSM_Timer);
            tsm_timer_milliseconds(mstimer_interval(&BACnet_TSM_Timer));
        }
        handler_cov_task();
        if (mstimer_expired(&BACnet_Address_Timer)) {
            mstimer_reset(&BACnet_Address_Timer);
            /* address cache */
            seconds = mstimer_interval(&BACnet_Address_Timer)/1000;
            address_cache_timer(seconds);
        }
        /* output/input */
        if (blinkt_test) {
            blinkt_test_task();
        } else {
            bacnet_output_task();
        }
    }

    return 0;
}

/* @} */

/* End group ServerDemo */
