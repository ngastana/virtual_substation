/*
 * sv_subscriber_example.c
 *
 * Example program for Sampled Values (SV) subscriber
 *
 */
#include <signal.h>
#include <stdio.h>
#include <syslog.h>
#include "hal_thread.h"
#include "sv_subscriber.h"
#include "iec61850_server.h"
#include "virtual_ied_model.h"

#ifdef GPIO_SUPPORT_ENABLED
#include "gpio_ctrl.h"
#endif

#define OVERCURRENT_THRESHOLD 400
#define OVER_VOLTAGE_THRESHOLD 2.1

/********************************************/
/************ Global Variables **************/
/********************************************/

/* import IEC 61850 device model created from SCL-File */
extern IedModel iedModel;

IedServer iedServer;

static bool running = true;
bool oc_tripped = false;
bool ov_tripped = false;

#ifdef GPIO_SUPPORT_ENABLED
static uint8_t gpio_pin_number = 216;
#endif

void sigint_handler(int signalId)
{
    running = 0;
}

#ifdef GPIO_SUPPORT_ENABLED
void init_gpio_pin(uint8_t pin_number)
{
    export_pin(pin_number);

    set_pin_direction(pin_number, "out");
}
void change_pin_value(uint8_t pin_number, bool value)
{
    write_pin_value(pin_number, value);
}
#endif

/**
 * @brief Generates a Trip due to an instantaneous overcurrent situation
 * 
 */
void generateOcTrip()
{
    syslog(LOG_INFO, "New overcurrent situation not previously signaled detected. Generate Trip");
    IedServer_updateBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PTRC1_Tr_general, false);
    IedServer_updateBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PIOC1_OP_general, false);
    oc_tripped = true;
    
    #ifdef GPIO_SUPPORT_ENABLED
        write_pin_value (gpio_pin_number, true);
    #endif
}

/**
 * @brief Generates a Trip due to an overvoltage situation
 * 
 */
void generateOvTrip()
{
    syslog(LOG_INFO, "New overvoltage situation not previously signaled detected. Generate Trip");
    IedServer_updateBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PTRC1_Tr_general, true);
    IedServer_updateBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PTOV1_Str_general, true);

    ov_tripped = true;
    
    #ifdef GPIO_SUPPORT_ENABLED
        write_pin_value (gpio_pin_number, true);
    #endif
}

/**
 * @brief Checks if there is an instantaneous overcurrent situation in the specified phase
 * 
 * @param current 
 * @param dataAttribute data attribute to signal the overcurrent situation 
 */
bool checkOvercurrent(int32_t current, DataAttribute* dataAttribute)
{
    bool overcurrent = false;

    Quality q = QUALITY_VALIDITY_GOOD;
    uint64_t timestamp = Hal_getTimeInMs();
    Timestamp iecTimestamp;

    Timestamp_clearFlags(&iecTimestamp);
    Timestamp_setTimeInMilliseconds(&iecTimestamp, timestamp);
    Timestamp_setLeapSecondKnown(&iecTimestamp, true);

    if (abs(current) > OVERCURRENT_THRESHOLD )
    {
        overcurrent = true;
        IedServer_updateBooleanAttributeValue(iedServer, dataAttribute, true);
        IedServer_updateQuality(iedServer, IEDMODEL_GenericMeasurement_PIOC1_OP_q, q);
        IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PIOC1_OP_t, &iecTimestamp);
        
        if(!oc_tripped)
        {
            generateOcTrip();
	    printf("OC-Trip timestamp: %u.%u\n", (uint32_t) (timestamp / 1000), (uint32_t) (timestamp % 1000));
        }
    }
    else
    {
        IedServer_updateBooleanAttributeValue(iedServer, dataAttribute, false);
        IedServer_updateQuality(iedServer, IEDMODEL_GenericMeasurement_PIOC1_OP_q, q);
        IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PIOC1_OP_t, &iecTimestamp);
    }

   return overcurrent;    
}

/**
 * @brief Checks if th the message sequence number is the expected
 *
 * @param secId  Received message number
 */
bool checkMsgSequence (int32_t secId) 
{
	
	static int32_t nextId = -1;
	static struct timeval st;
	struct timeval  et;
	if (nextId == -1) {
		nextId = secId + 1;
    		gettimeofday(&st,NULL);
		return true;
	}
	else if (nextId == secId){
		
		if (nextId == 2000){
			gettimeofday(&et,NULL);
                        int elapsed = ((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec);
			printf  ("OK!! Expected nextId %d, Received id %d , in time %d microseconds\n",nextId,secId,elapsed);
    			gettimeofday(&st,NULL);
		}

		if (++nextId == 4000)
			nextId = 0 ;
		return true;
	}
	else {
		printf  ("Error!! Expected nextId %d, Received id %d\n",nextId,secId);
		nextId = -1;
		return false;
	}
}
/**
 * @brief Checks if there is an overvoltage situation in the specified phase
 * 
 * @param voltage  <code>int32_t</code> with the voltage value
 * @param dataAttribute data attribute to signal the ove:rvoltage situation 
 */
bool checkOvervoltage(int32_t voltage, DataAttribute* dataAttribute)
{
    bool overvoltage = false;

    static int oc_trp = 0;

    Quality q = QUALITY_VALIDITY_GOOD;
    uint64_t timestamp = Hal_getTimeInMs();
    Timestamp iecTimestamp;

    Timestamp_clearFlags(&iecTimestamp);
    Timestamp_setTimeInMilliseconds(&iecTimestamp, timestamp);
    Timestamp_setLeapSecondKnown(&iecTimestamp, true);

    if (voltage > OVER_VOLTAGE_THRESHOLD )
    {
        overvoltage = true;
        IedServer_updateBooleanAttributeValue(iedServer, dataAttribute, true);
        IedServer_updateQuality(iedServer, IEDMODEL_GenericMeasurement_PTOV1_Str_q, q);
        IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PTOV1_Str_t, &iecTimestamp);
    	    
        if(!ov_tripped)
        {
            generateOvTrip();
	    printf("OV-Trip timestamp: %u.%u\n", (uint32_t) (timestamp / 1000), (uint32_t) (timestamp % 1000));
        }
    }
    else
    {
        IedServer_updateBooleanAttributeValue(iedServer, dataAttribute, false);
        IedServer_updateQuality(iedServer, IEDMODEL_GenericMeasurement_PTOV1_Str_q, q);
        IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PTOV1_Str_t, &iecTimestamp);
    }

   return overvoltage;    
}


/**
 * @brief Updates current measuments
 * 
 * @param ia <code>int32_t</code> with current value of phase A
 * @param ib <code>int32_t</code> with current value of phase B
 * @param ic <code>int32_t</code> with current value of phase C
 * @param ineut <code>int32_t</code> with current value of phase neut
 */
void updateCurrentValues(int32_t ia, int32_t ib, int32_t ic, int32_t ineut)
{   
    Quality q = QUALITY_VALIDITY_GOOD;
    uint64_t timestamp = Hal_getTimeInMs();
    Timestamp iecTimestamp;

    Timestamp_clearFlags(&iecTimestamp);
    Timestamp_setTimeInMilliseconds(&iecTimestamp, timestamp);
    Timestamp_setLeapSecondKnown(&iecTimestamp, true);
        
    IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_A_phsA_cVal_mag_i, ia);
    IedServer_updateQuality(iedServer, IEDMODEL_GenericMeasurement_MMXU1_A_phsA_q, q);
    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_A_phsA_t, &iecTimestamp);
        
    IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_A_phsB_cVal_mag_i, ib);
    IedServer_updateQuality(iedServer, IEDMODEL_GenericMeasurement_MMXU1_A_phsB_q, q);
    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_A_phsB_t, &iecTimestamp);

    IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_A_phsC_cVal_mag_i, ic);
    IedServer_updateQuality(iedServer, IEDMODEL_GenericMeasurement_MMXU1_A_phsC_q, q);
    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_A_phsC_t, &iecTimestamp);

    IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_A_neut_cVal_mag_i, ineut);
    IedServer_updateQuality(iedServer, IEDMODEL_GenericMeasurement_MMXU1_A_neut_q, q);
    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_A_neut_t, &iecTimestamp);
}

/**
 * @brief Updates voltage measuments in all the phases
 * 
 * @param ia <code>int32_t</code> with voltage value of phase A
 * @param ib <code>int32_t</code> with voltage value of phase B
 * @param ic <code>int32_t</code> with voltage value of phase C
 * @param ineut <code>int32_t</code> with voltage value of phase neut
 */
void updateVoltageValues(int32_t va, int32_t vb, int32_t vc, int32_t vneut)
{   
    Quality q = QUALITY_VALIDITY_GOOD;
    uint64_t timestamp = Hal_getTimeInMs();
    Timestamp iecTimestamp;

    Timestamp_clearFlags(&iecTimestamp);
    Timestamp_setTimeInMilliseconds(&iecTimestamp, timestamp);
    Timestamp_setLeapSecondKnown(&iecTimestamp, true);
        
    IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_PhV_phsA_cVal_mag_i, va);
    IedServer_updateQuality(iedServer, IEDMODEL_GenericMeasurement_MMXU1_PhV_phsA_q, q);
    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_PhV_phsA_t, &iecTimestamp);
        
    IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_PhV_phsB_cVal_mag_i, vb);
    IedServer_updateQuality(iedServer, IEDMODEL_GenericMeasurement_MMXU1_PhV_phsB_q, q);
    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_PhV_phsB_t, &iecTimestamp);

    IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_PhV_phsC_cVal_mag_i, vc);
    IedServer_updateQuality(iedServer, IEDMODEL_GenericMeasurement_MMXU1_PhV_phsC_q, q);
    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_PhV_phsC_t, &iecTimestamp);

    IedServer_updateInt32AttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_PhV_neut_cVal_mag_i, vneut);
    IedServer_updateQuality(iedServer, IEDMODEL_GenericMeasurement_MMXU1_PhV_neut_q, q);
    IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_GenericMeasurement_MMXU1_PhV_neut_t, &iecTimestamp);
}

/**
 * @brief Callback handler for received SV messages. It controls if an overcurrent situation is reached
 * 
 * @param subscriber handle to SVSubscriber
 * @param parameter 
 * @param asdu Handle to a SV ASDU
 */
static void
svUpdateListener (SVSubscriber subscriber, void* parameter, SVSubscriber_ASDU asdu)
{
    //syslog(LOG_DEBUG, "svUpdateListener called");

    bool isOCDetected = false;
    bool isOVDetected = false;

    static int count =  0;    

    const char* svID = SVSubscriber_ASDU_getSvId(asdu);
    if (svID != NULL)
    {
        syslog(LOG_INFO, "  svID=(%s)", svID);
    }
    int32_t secId = SVSubscriber_ASDU_getSmpCnt(asdu);
    //syslog(LOG_DEBUG, "  smpCnt: %i", secId);
    //syslog(LOG_DEBUG, "  confRev: %u", SVSubscriber_ASDU_getConfRev(asdu));

    checkMsgSequence (secId);

    if (SVSubscriber_ASDU_getDataSize(asdu) >= 64) 
    {    
        int32_t ia = SVSubscriber_ASDU_getINT32(asdu,0) / 1000.0;
        int32_t ib = SVSubscriber_ASDU_getINT32(asdu,8) / 1000.0;
        int32_t ic = SVSubscriber_ASDU_getINT32(asdu,16) / 1000.0;
        int32_t ineut = SVSubscriber_ASDU_getINT32(asdu,24) / 1000.0;

	//syslog(LOG_DEBUG, "IA: %.2f || IB: %.2f || IC: %.2f || Ineut: %.2f", (float)ia, (float)ib, (float)ic, (float)ineut);

        updateCurrentValues(ia, ib, ic, ineut);

        checkOvercurrent(ia, IEDMODEL_GenericMeasurement_PIOC1_OP_phsA);
        checkOvercurrent(ib, IEDMODEL_GenericMeasurement_PIOC1_OP_phsB);
        checkOvercurrent(ic, IEDMODEL_GenericMeasurement_PIOC1_OP_phsC);
        checkOvercurrent(ineut, IEDMODEL_GenericMeasurement_PIOC1_OP_neut);

        int32_t va = SVSubscriber_ASDU_getINT32(asdu,32) / 1000.0;
        int32_t vb = SVSubscriber_ASDU_getINT32(asdu,40) / 1000.0;
        int32_t vc = SVSubscriber_ASDU_getINT32(asdu,48) / 1000.0;
        int32_t vneut = SVSubscriber_ASDU_getINT32(asdu,56) / 1000.0;
        
        updateVoltageValues(va, vb, vc, vneut);

	checkOvervoltage(va, IEDMODEL_GenericMeasurement_PTOV1_Str_phsA);
        checkOvervoltage(vb, IEDMODEL_GenericMeasurement_PTOV1_Str_phsB);
        checkOvervoltage(vc, IEDMODEL_GenericMeasurement_PTOV1_Str_phsC);
        checkOvervoltage(vneut, IEDMODEL_GenericMeasurement_PTOV1_Str_neut);

        syslog(LOG_DEBUG,"VA: %.2f || VB: %.2f || VC: %.2f || Vneut: %.2f", (float)va, (float)vb, (float)vc, (float)vneut);

        //if (!IedServer_getBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PIOC1_OP_phsA) && 
        //    !IedServer_getBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PIOC1_OP_phsB) && 
        //    !IedServer_getBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PIOC1_OP_phsC) &&
        //    !IedServer_getBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PIOC1_OP_neut) && oc_tripped)
        if (count == 2000)
	{
            syslog(LOG_INFO, "Overcurrent situation cleared. Restablishing PTCR1 Trip Value");

            IedServer_updateBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PTRC1_Tr_general, false);
            IedServer_updateBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PIOC1_OP_general, false);

            oc_tripped = false;

            #ifdef GPIO_SUPPORT_ENABLED
                write_pin_value (gpio_pin_number, false);
            #endif
        }

        //if (!IedServer_getBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PTOV1_Str_phsA) && 
        //    !IedServer_getBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PTOV1_Str_phsB) && 
        //    !IedServer_getBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PTOV1_Str_phsC) &&
        //    !IedServer_getBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PTOV1_Str_neut) && ov_tripped)
        if (count++>=2000)
	{
            syslog(LOG_INFO, "Overvoltage situation cleared. Restablishing PIOC1 and PTCR1 general Value");

            IedServer_updateBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PTRC1_Tr_general, false);
            IedServer_updateBooleanAttributeValue(iedServer, IEDMODEL_GenericMeasurement_PTOV1_Str_general, false);
            

            ov_tripped = false;

            #ifdef GPIO_SUPPORT_ENABLED
                write_pin_value (gpio_pin_number, false);
            #endif
	    count   = 0;
        }
    }
}

int
main(int argc, char** argv)
{
    openlog("virtual_ied", LOG_CONS | LOG_PID, LOG_USER);
    
    //openlog("virtual_ied", LOG_PERROR | LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Virtual IED initialization");
    uint8_t ethernetIfcID [20] = "eth0";
    uint8_t ethernetIfcID_Proc [20] = "eth1";
    uint16_t app_id = 0x4000;

    SVReceiver receiver = SVReceiver_create();

    if (argc == 2) {
        sscanf(argv[1], "%s", ethernetIfcID);
		syslog(LOG_INFO,"Selected interface id: %s", ethernetIfcID);
    }
    else if (argc == 3)
    {
        sscanf(argv[1], "%s", ethernetIfcID);
		syslog(LOG_INFO,"Selected interface id: %s", ethernetIfcID);
        sscanf(argv[2],"%hx",&app_id);
        syslog(LOG_INFO,"Selected SV APPID to be subscribed: 0x%x", app_id);
    }
    #ifdef GPIO_SUPPORT_ENABLED
    else if (argc == 4)
    {
        sscanf(argv[1], "%s", ethernetIfcID);
		syslog(LOG_INFO,"Selected interface id: %s", ethernetIfcID);
        sscanf(argv[2],"%hx",&app_id);
        syslog(LOG_INFO,"Selected SV APPID to be subscribed: 0x%x", app_id);
        sscanf(argv[3],"%s",&gpio_pin_number);
        syslog(LOG_INFO,"Selected GPIO PIN Number: %d", gpio_pin_number);
    }
    #endif
    else {
        syslog(LOG_WARNING,"No interface id selected. Using interface %s", ethernetIfcID);
        syslog(LOG_WARNING,"No SV APPID selected. Using  0x%x", app_id);
        #ifdef GPIO_SUPPORT_ENABLED
            syslog(LOG_WARNING,"No GPIO PIN Number selected. Using  %d", gpio_pin_number);
        #endif
    }

    iedServer = IedServer_create(&iedModel);

    SVReceiver_setInterfaceId(receiver, ethernetIfcID);
    IedServer_setGooseInterfaceId(iedServer, ethernetIfcID_Proc);

    /* MMS server will be instructed to start listening to client connections. */
    IedServer_start(iedServer, 102);

    if (!IedServer_isRunning(iedServer)) {
        printf("Starting server failed! Exit.\n");
        IedServer_destroy(iedServer);
        exit(-1);
    }

    IedServer_enableGoosePublishing(iedServer);

    #ifdef GPIO_SUPPORT_ENABLED
        init_gpio_pin(gpio_pin_number);
    #endif

    /* Create a subscriber listening to SV messages with APPID 4000h */
    SVSubscriber subscriber = SVSubscriber_create(NULL, app_id);

    /* Install a callback handler for the subscriber */
    SVSubscriber_setListener(subscriber, svUpdateListener, NULL);

    /* Connect the subscriber to the receiver */
    SVReceiver_addSubscriber(receiver, subscriber);

    /* Start listening to SV messages - starts a new receiver background thread */
    SVReceiver_start(receiver);

    if (SVReceiver_isRunning(receiver)) {
        signal(SIGINT, sigint_handler);

        while (running)
            Thread_sleep(1);

        /* Stop listening to SV messages */
        SVReceiver_stop(receiver);

        /* Stop MMS server - close TCP server socket and all client sockets */
        IedServer_stop(iedServer);
    }
    else {
        syslog(LOG_ERR, "Failed to start SV subscriber. Reason can be that the Ethernet interface doesn't exist or root permission are required.");
        /* Stop MMS server - close TCP server socket and all client sockets */
        IedServer_stop(iedServer);
    }

    /* Cleanup and free resources */
    SVReceiver_destroy(receiver);
    closelog();
    return 0;
}
