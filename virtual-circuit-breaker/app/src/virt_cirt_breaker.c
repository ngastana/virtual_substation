/*
 * goose_subscriber_example.c
 *
 * This is an example for a standalone GOOSE subscriber
 *
 * Has to be started as root in Linux.
 */
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <syslog.h>

#include "goose_receiver.h"
#include "goose_subscriber.h"
#include "hal_thread.h"
#include "linked_list.h"
#include "virtual_xcbr_model.h"

#ifdef GPIO_SUPPORT_ENABLED
#include "gpio_ctrl.h"
#endif

static int running = 1;

#ifdef GPIO_SUPPORT_ENABLED
static uint8_t gpio_pin_number = 3;
#endif

#ifdef GPIO_SUPPORT_ENABLED
void init_gpio_pin(uint8_t pin_number)
{
    export_pin(pin_number);

    set_pin_direction(pin_number, "out");

    write_pin_value(pin_number, true);
}
void change_pin_value(uint8_t pin_number, bool value)
{
    write_pin_value(pin_number, value);
}
#endif

static void sigint_handler(int signalId)
{
    running = 0;
}

static void tripGooseListener(GooseSubscriber subscriber, void* parameter)
{

    static struct timeval st;
    gettimeofday(&st,NULL);
    int elapsed = st.tv_sec;
    
    syslog(LOG_DEBUG, "  stNum: %u sqNum: %u\n", GooseSubscriber_getStNum(subscriber),
            GooseSubscriber_getSqNum(subscriber));
    syslog(LOG_DEBUG,"  timeToLive: %u\n", GooseSubscriber_getTimeAllowedToLive(subscriber));

    uint64_t timestamp = GooseSubscriber_getTimestamp(subscriber);

    syslog(LOG_DEBUG, "  timestamp: %u.%u\n", (uint32_t) (timestamp / 1000), (uint32_t) (timestamp % 1000));
    syslog(LOG_DEBUG, "  message is %s\n", GooseSubscriber_isValid(subscriber) ? "valid" : "INVALID");

    MmsValue* values = GooseSubscriber_getDataSetValues(subscriber);
    MmsValue* trip_value = MmsValue_getElement(values, 0);
    bool isTrip = MmsValue_getBoolean(trip_value);

    if(isTrip)
    {
        printf("GOOSE [%s] received with trip value [true]. Opening XCBR\n", GooseSubscriber_getGoId(subscriber));
        //syslog(LOG_INFO, "GOOSE [%s] received with trip value [true]. Opening XCBR", GooseSubscriber_getGoId(subscriber));
        fflush(stdout);

        #ifdef GPIO_SUPPORT_ENABLED
            change_pin_value(gpio_pin_number, false);
        #endif
    }
    else
    {
        syslog(LOG_INFO, "GOOSE [%s] received with trip value [false]. Closing XCBR", GooseSubscriber_getGoId(subscriber));

        #ifdef GPIO_SUPPORT_ENABLED
            change_pin_value(gpio_pin_number, true);
        #endif
    }
}

int main(int argc, char** argv)
{
    uint8_t ethernetIfcID [20] = "eth0";
    uint16_t app_id = 0x1000;

    //openlog("virtual_xcbr", LOG_CONS | LOG_PID, LOG_USER);
    openlog("virtual_xcbr", LOG_PERROR | LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Virtual XCBR IED initialization");

    GooseReceiver receiver = GooseReceiver_create();

    if (argc == 2) {
        sscanf(argv[1], "%s", ethernetIfcID);
	syslog(LOG_INFO,"Selected interface id: %s", ethernetIfcID);
        syslog(LOG_INFO,"Selected GOOSE APPID to be subscribed: 0x%x", app_id);
    }
    else if (argc == 3)
    {
        sscanf(argv[1], "%s", ethernetIfcID);
	syslog(LOG_INFO,"Selected interface id: %s", ethernetIfcID);
        sscanf(argv[2],"%hx",&app_id);
        syslog(LOG_INFO,"Selected GOOSE APPID to be subscribed: 0x%x", app_id);
    }
    #ifdef GPIO_SUPPORT_ENABLED
    else if (argc == 4)
    {
        sscanf(argv[1], "%s", ethernetIfcID);
	syslog(LOG_INFO,"Selected interface id: %s", ethernetIfcID);
        sscanf(argv[2],"%hx",&app_id);
        syslog(LOG_INFO,"Selected SV APPID to be subscribed: 0x%x", app_id);
        sscanf(argv[3],"%d",&gpio_pin);
        syslog(LOG_INFO,"Selected GPIO PIN Number: %d", gpio_pin_number);
    }
    #endif
    else {
        syslog(LOG_WARNING,"No interface id selected. Using interface %s", ethernetIfcID);

        #ifdef GPIO_SUPPORT_ENABLED
            syslog(LOG_WARNING,"No GPIO PIN Number selected. Using  %d", gpio_pin_number);
        #endif
    }

    #ifdef GPIO_SUPPORT_ENABLED
        init_gpio_pin(gpio_pin_number);
    #endif

    GooseSubscriber subscriber = GooseSubscriber_create("virtualIEDGenericMeasurement/LLN0$GO$gcbOcTrip", NULL);

    uint8_t dstMac1[6] = {0x01,0x0c,0xcd,0x01,0x00,0x00};
    GooseSubscriber_setDstMac(subscriber, dstMac1);
    GooseSubscriber_setAppId(subscriber, app_id);
    GooseSubscriber_setListener(subscriber, tripGooseListener, NULL);

    GooseReceiver_addSubscriber(receiver, subscriber);

    GooseReceiver_start(receiver);

    if (GooseReceiver_isRunning(receiver)) {
        signal(SIGINT, sigint_handler);

        while (running) {
            Thread_sleep(100);
        }
    }
    else {
        syslog(LOG_ERR,"Failed to start GOOSE subscriber. Reason can be that the Ethernet interface doesn't exist or root permission are required.\n");
    }

    GooseReceiver_stop(receiver);

    GooseReceiver_destroy(receiver);
    
    closelog();
    return 0;
}
