#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include <cs104_slave.h>

#include "hal_thread.h"
#include "hal_time.h"

#include "iec60870_point_map.h"

#include "iec61850_client.h"
#include "iec61850_report_handle.h"
#include "config.h"
#include <syslog.h>

#include <time.h>


static bool running = true;
bool isScada = false;
configuration config;
// extern PointMap pointMapping;
extern boolIec60870PointMap boolMapping;
extern anagIec60870PointMap anagMapping;



void sigint_handler(int signalId)
{
    running = false;
}

void printCP56Time2a(CP56Time2a time)
{
    printf("%02i:%02i:%02i %02i/%02i/%04i", CP56Time2a_getHour(time),
                             CP56Time2a_getMinute(time),
                             CP56Time2a_getSecond(time),
                             CP56Time2a_getDayOfMonth(time),
                             CP56Time2a_getMonth(time),
                             CP56Time2a_getYear(time) + 2000);
}

/* Callback handler to log sent or received messages (optional) */
static void rawMessageHandler(void* parameter, IMasterConnection conneciton, uint8_t* msg, int msgSize, bool sent)
{
    if (sent)
        printf("SEND: ");
    else
        printf("RCVD: ");

    int i;
    for (i = 0; i < msgSize; i++) {
        printf("%02x ", msg[i]);
    }

    printf("\n");
}

static bool clockSyncHandler (void* parameter, IMasterConnection connection, CS101_ASDU asdu, CP56Time2a newTime)
{
    printf("Process time sync command with time "); printCP56Time2a(newTime); printf("\n");

    uint64_t newSystemTimeInMs = CP56Time2a_toMsTimestamp(newTime);

    /* Set time for ACT_CON message */
    CP56Time2a_setFromMsTimestamp(newTime, Hal_getTimeInMs());

    /* update system time here */

    return true;
}

static bool interrogationHandler(void* parameter, IMasterConnection connection, CS101_ASDU asdu, uint8_t qoi)
{
    printf("Received interrogation for group %i\n", qoi);

    if (qoi == 20) { /* only handle station interrogation */

        CS101_AppLayerParameters alParams = IMasterConnection_getApplicationLayerParameters(connection);
        IMasterConnection_sendACT_CON(connection, asdu, false);
        InformationObject io = NULL;
        
        /* The CS101 specification only allows information objects without timestamp in GI responses */
        CS101_ASDU newAsdu = CS101_ASDU_create(alParams, false, CS101_COT_INTERROGATED_BY_STATION, 0, 1, false, false);

        for (int i = 0 ; i< 4; i++) {
			io = (InformationObject) MeasuredValueScaled_create(NULL, 28960 + i, anagMapping.anagVals[i], IEC60870_QUALITY_GOOD);
			CS101_ASDU_addInformationObject(newAsdu, io);
			InformationObject_destroy(io);		
		}
		for (int i = 0 ; i< 4; i++) {
			io = (InformationObject) MeasuredValueScaled_create(NULL, 28972 + i, anagMapping.anagVals[i], IEC60870_QUALITY_GOOD);
			CS101_ASDU_addInformationObject(newAsdu, io);
			InformationObject_destroy(io);		
		}

        IMasterConnection_sendASDU(connection, newAsdu);
        CS101_ASDU_destroy(newAsdu);
        /*
        newAsdu = CS101_ASDU_create(alParams, false, CS101_COT_INTERROGATED_BY_STATION, 0, 1, false, false);
        
        io = (InformationObject) SinglePointInformation_create(NULL, 104, true, IEC60870_QUALITY_GOOD);
        CS101_ASDU_addInformationObject(newAsdu, io);
        InformationObject_destroy(io);

		for (int i = 0 ; i< 32; i++) {
			io = (InformationObject) SinglePointInformation_create(NULL, 28600 + i*2, boolMapping.boolVals[i], IEC60870_QUALITY_GOOD);
			CS101_ASDU_addInformationObject(newAsdu, io);
			InformationObject_destroy(io);		
		}   
		     
        IMasterConnection_sendASDU(connection, newAsdu);
        CS101_ASDU_destroy(newAsdu);
        */
        IMasterConnection_sendACT_TERM(connection, asdu);
    }
    else {
        IMasterConnection_sendACT_CON(connection, asdu, true);
    }

    return true;
}

static bool asduHandler(void* parameter, IMasterConnection connection, CS101_ASDU asdu)
{
    if (CS101_ASDU_getTypeID(asdu) == C_SC_NA_1) {
        printf("El SCADA ha enviado un comando\n");

        if  (CS101_ASDU_getCOT(asdu) == CS101_COT_ACTIVATION) {
            InformationObject io = CS101_ASDU_getElement(asdu, 0);

            if (InformationObject_getObjectAddress(io) == 64185) {
                SingleCommand sc = (SingleCommand) io;

                printf("IOA: %i . Se solicita cambiar el estado del interruptor BRK52 a %i\n", InformationObject_getObjectAddress(io), SingleCommand_getState(sc));

                boolMapping.boolStruct.POINT_28604_VALUE = SingleCommand_getState(sc);

                //CS101_ASDU_addInformationObject(asdu, (InformationObject)SinglePointInformation_create(NULL, 28604, pointMapping.POINT_28604_VALUE, IEC60870_QUALITY_GOOD));

                if(SingleCommand_getState(sc) == false)
                {
                	
        			for (int i = 0 ; i< 6; i++) 
						anagMapping.anagVals[i] = 10.0;
                }
                else
                {
        			for (int i = 0 ; i< 6; i++) 
						anagMapping.anagVals[i] = 59.5;
//                 	pointMapping.POINT_6241_VALUE = 58.1; pointMapping.POINT_28960_VALUE = 233.4;	pointMapping.POINT_28961_VALUE = 53.7;	
//                 	pointMapping.POINT_28962_VALUE = 22.2;					pointMapping.POINT_28963_VALUE = 138.0;
                }

                CS101_ASDU_setCOT(asdu, CS101_COT_ACTIVATION_CON);
            }
            else
            {
            	printf("El IOA [%i] que has enviado no se corresponde con el BRK52 sobre el que te hemos pedido operar\n", InformationObject_getObjectAddress(io));
                CS101_ASDU_setCOT(asdu, CS101_COT_UNKNOWN_IOA);
            }

            InformationObject_destroy(io);
        }
        else
        {
        	printf("El COMANDO [%i] que has enviado no se corresponde con el comando que debes enviar\n", CS101_ASDU_getTypeID(asdu));
            CS101_ASDU_setCOT(asdu, CS101_COT_UNKNOWN_COT);
        }

        IMasterConnection_sendASDU(connection, asdu);

        CS101_AppLayerParameters alParams = IMasterConnection_getApplicationLayerParameters(connection);
        CS101_ASDU newAsdu = CS101_ASDU_create(alParams, false, CS101_COT_SPONTANEOUS, 0, 1, false, false);
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject)SinglePointInformation_create(NULL, 28604, boolMapping.boolStruct.POINT_28604_VALUE, IEC60870_QUALITY_GOOD));
        IMasterConnection_sendASDU(connection, newAsdu);
        CS101_ASDU_destroy(newAsdu);

        return true;
    }

    return false;
}

static bool connectionRequestHandler(void* parameter, const char* ipAddress)
{
    printf("Enhorabuena!!! Has conseguido que el SCADA [%s] se conecte al honeypot\n", ipAddress);

    if (strcmp(ipAddress, "10.10.30.11") == 0) {
    	isScada = true;
    }
    else {
    	isScada = false;
    }
    return true;
}

static void connectionEventHandler(void* parameter, IMasterConnection con, CS104_PeerConnectionEvent event)
{
    if (event == CS104_CON_EVENT_CONNECTION_OPENED) {
        printf("Connection opened (%p)\n", con);
    }
    else if (event == CS104_CON_EVENT_CONNECTION_CLOSED) {
        printf("Connection closed (%p)\n", con);
    }
    else if (event == CS104_CON_EVENT_ACTIVATED) {
        printf("Connection activated (%p)\n", con);
    }
    else if (event == CS104_CON_EVENT_DEACTIVATED) {
        printf("Connection deactivated (%p)\n", con);
    }
}

int main(int argc, char** argv)
{

	openlog("virtual_104_gtw", LOG_PERROR | LOG_PID, LOG_USER);
	
	syslog(LOG_INFO, "Starting 104 gtw application");
	
    if (readConfiguration (&config) != 0) {
    	syslog(LOG_INFO, "Finish");
    	closelog();
        return 1;
    }
    
    /* Add Ctrl-C handler */
    signal(SIGINT, sigint_handler);
    /* create a new slave/server instance with default connection parameters and
     * default message queue size */
    CS104_Slave slave = CS104_Slave_create(10, 10);
    CS104_Slave_setLocalAddress(slave, "0.0.0.0");

    /* Set mode to a single redundancy group
     * NOTE: library has to be compiled with CONFIG_CS104_SUPPORT_SERVER_MODE_SINGLE_REDUNDANCY_GROUP enabled (=1)
     */
   	
   	CS104_Slave_setServerMode(slave, CS104_MODE_SINGLE_REDUNDANCY_GROUP);
    /* get the connection parameters - we need them to create correct ASDUs */

    CS101_AppLayerParameters alParams = CS104_Slave_getAppLayerParameters(slave);

    /* set the callback handler for the clock synchronization command */
    CS104_Slave_setClockSyncHandler(slave, clockSyncHandler, NULL);

    /* set the callback handler for the interrogation command */
    CS104_Slave_setInterrogationHandler(slave, interrogationHandler, NULL);

    /* set handler for other message types */
    CS104_Slave_setASDUHandler(slave, asduHandler, NULL);

    /* set handler to handle connection requests (optional) */
    CS104_Slave_setConnectionRequestHandler(slave, connectionRequestHandler, NULL);

    /* set handler to track connection events (optional) */
    CS104_Slave_setConnectionEventHandler(slave, connectionEventHandler, NULL);

    /* uncomment to log messages */
    //CS104_Slave_setRawMessageHandler(slave, rawMessageHandler, NULL);

    // Point map init
    //initializePointMapValues();
    initializeArrayPointMapValues();

    CS104_Slave_start(slave);
// Lanzamos la parte relativa al IEC61850
    pthread_t id;
    int j = 1;
    pthread_create(&id, NULL, iecReportHandle, &j);
    
    if (CS104_Slave_isRunning(slave) == false) {
        syslog(LOG_ERR, "Starting server failed");
        closelog();
        return 1;
    }
    else   {
    	syslog(LOG_INFO, "Starting 104 server success! ");
    }
         
    while (running) {
        Thread_sleep(1000);

        if(isScada == true)
        {
			CS101_ASDU newAsdu = CS101_ASDU_create(alParams, false, CS101_COT_PERIODIC, 0, 1, false, false);
			InformationObject io = NULL; 
// 			InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, 6240, anagMapping.anagStruct.POINT_6240_VALUE, IEC60870_QUALITY_GOOD);
// 			CS101_ASDU_addInformationObject(newAsdu, io);
// 			InformationObject_destroy(io);
// 			
// 			io = (InformationObject) MeasuredValueShort_create(NULL, 6241, anagMapping.anagStruct.POINT_6241_VALUE, IEC60870_QUALITY_GOOD);
// 			CS101_ASDU_addInformationObject(newAsdu, io);
// 			InformationObject_destroy(io);

		
	        for (int i = 2 ; i< 6; i++) {
				io = (InformationObject) MeasuredValueShort_create(NULL, 28960 + i, anagMapping.anagVals[i], IEC60870_QUALITY_GOOD);
				CS101_ASDU_addInformationObject(newAsdu, io);
				InformationObject_destroy(io);		
			}
			
			for (int i = 0 ; i< 13; i++) {
				io = (InformationObject) MeasuredValueShort_create(NULL, 28972 + i, anagMapping.anagVals[i], IEC60870_QUALITY_GOOD);
				CS101_ASDU_addInformationObject(newAsdu, io);
				InformationObject_destroy(io);		
			}

			/* Add ASDU to slave event queue - don't release the ASDU afterwards!
			 * The ASDU will be released by the Slave instance when the ASDU
			 * has been sent.
			 */
			CS104_Slave_enqueueASDU(slave, newAsdu);

			CS101_ASDU_destroy(newAsdu);

        }
    }

    CS104_Slave_stop(slave);
	
// 	int* ptr;
//   	pthread_join(id, (void**)&ptr);
//   	printf("Value recevied by parent from IEC61850 Handle: ");
//  	printf("%i\n", *ptr);

    closelog();
    return 0;

}
