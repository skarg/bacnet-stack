/*
 * Copyright (C) 2020 Legrand North America, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <stdint.h>

/* BACnet Stack defines - first */
#include "bacnet/bacdef.h"
/* BACnet Stack core API */
#include "bacnet/bacdcode.h"
#include "bacnet/apdu.h"
#include "bacnet/dcc.h"
#include "bacnet/iam.h"
#include "bacnet/npdu.h"
#include "bacnet/getevent.h"
#include "bacnet/version.h"
/* BACnet Stack basic services */
#include "bacnet/basic/sys/mstimer.h"
#include "bacnet/basic/services.h"
#include "bacnet/basic/tsm/tsm.h"
#include "bacnet/datalink/datalink.h"
#include "bacnet/basic/binding/address.h"
/* BACnet Stack basic objects */
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/object/lc.h"
#include "bacnet/basic/object/trendlog.h"
#if defined(INTRINSIC_REPORTING)
#include "bacnet/basic/object/nc.h"
#endif /* defined(INTRINSIC_REPORTING) */
#if (BACNET_PROTOCOL_REVISION >= 17)
#include "bacnet/basic/object/netport.h"
#endif

/* Logging module registration is already done in ports/zephyr/main.c */
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(bacnet, CONFIG_BACNETSTACK_LOG_LEVEL);

/* local buffer for incoming PDUs to process */
static uint8_t PDUBuffer[MAX_MPDU];
/* 1s timer for basic non-critical timed tasks */
static struct mstimer BACnet_Task_Timer;
/* task timer for object functionality */
static struct mstimer BACnet_Object_Timer;
/* uptimer for BACnet task */
static unsigned long BACnet_Uptime_Seconds;
/* packet counter for BACnet task */
static unsigned long BACnet_Packet_Count;
/* Device ID to track changes */
static uint32_t Device_ID = 0xFFFFFFFF;

/** Initialize the handlers we will utilize.
 * @see Device_Init, apdu_set_unconfirmed_handler, apdu_set_confirmed_handler
 */
static void bacnet_init(void)
{
	Device_Init(NULL);
	/* we need to handle who-is to support dynamic device binding */
	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS,
				     handler_who_is);
	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_HAS,
				     handler_who_has);
	/* set the handler for all the services we don't implement */
	/* It is required to send the proper reject message... */
	apdu_set_unrecognized_service_handler_handler(
		handler_unrecognized_service);
	/* Set the handlers for any confirmed services that we support. */
	/* We must implement read property - it's required! */
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
				   handler_read_property);
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
				   handler_read_property_multiple);
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
				   handler_write_property);
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROP_MULTIPLE,
				   handler_write_property_multiple);
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_REINITIALIZE_DEVICE,
				   handler_reinitialize_device);
	/* handle communication so we can shutup when asked */
	apdu_set_confirmed_handler(
		SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL,
		handler_device_communication_control);
    /* start the 1 second timer for non-critical cyclic tasks */
    mstimer_set(&BACnet_Task_Timer, 1000L);
    /* start the timer for more time sensitive object specific cyclic tasks */
    mstimer_set(&BACnet_Object_Timer, 100UL);
}

/**
 * @brief non-blocking BACnet task
 */
static void bacnet_task(void)
{
    bool hello_world = false;
    uint16_t pdu_len = 0;
    BACNET_ADDRESS src = { 0 };
    uint32_t elapsed_milliseconds = 0;
    uint32_t elapsed_seconds = 0;

    /* hello, World! */
    if (Device_ID != Device_Object_Instance_Number()) {
        Device_ID = Device_Object_Instance_Number();
        hello_world = true;
    }
    if (hello_world) {
        Send_I_Am(&Handler_Transmit_Buffer[0]);
    }
    /* handle non-time-critical cyclic tasks */
    if (mstimer_expired(&BACnet_Task_Timer)) {
        /* 1 second tasks */
        mstimer_reset(&BACnet_Task_Timer);
        /* presume that the elapsed time is the interval time */
        elapsed_milliseconds = mstimer_interval(&BACnet_Task_Timer);
        elapsed_seconds = elapsed_milliseconds/1000;
        BACnet_Uptime_Seconds += elapsed_seconds;
        dcc_timer_seconds(elapsed_seconds);
        datalink_maintenance_timer(elapsed_seconds);
        handler_cov_timer_seconds(elapsed_seconds);
    }
    while (!handler_cov_fsm()) {
        /* waiting for COV processing to be IDLE */
    }
    /* object specific cyclic tasks */
    if (mstimer_expired(&BACnet_Object_Timer)) {
        mstimer_reset(&BACnet_Object_Timer);
        elapsed_milliseconds = mstimer_interval(&BACnet_Object_Timer);
        Device_Timer(elapsed_milliseconds);
    }
    /* handle the messaging */
    pdu_len = datalink_receive(&src, &PDUBuffer[0], sizeof(PDUBuffer), 0);
    if (pdu_len) {
        npdu_handler(&src, &PDUBuffer[0], pdu_len);
        BACnet_Packet_Count++;
		LOG_INF("BACnet Packet Received! %lu packets", BACnet_Packet_Count);
    }
}

int main(void)
{
	LOG_INF("\n*** BACnet Profile B-SS Sample ***\n");
	LOG_INF("BACnet Stack Version " BACNET_VERSION_TEXT);
	LOG_INF("BACnet Device ID: %u", Device_Object_Instance_Number());
	LOG_INF("BACnet Device Max APDU: %d", MAX_APDU);

    bacnet_init();
	datalink_init(NULL);
    for (;;) {
        k_sleep(K_MSEC(10));
        bacnet_task();
    }

    return 0;
}
