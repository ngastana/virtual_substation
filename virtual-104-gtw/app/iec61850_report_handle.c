
#include "iec61850_report_handle.h"
#include "iec61850_client.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include "hal_thread.h"
#include <syslog.h>
#include "config.h"
#include "iec60870_point_map.h"

/***
*
*
***/

extern boolIec60870PointMap boolMapping;
extern anagIec60870PointMap anagMapping;
extern configuration config;

IECReportHandle class_init(char *hostn, int port)
{
	IECReportHandle rh; 
	
    rh.hostname = hostn;
    rh.port 	= port;
    rh.rcb		= NULL;
	rh.clientDataSet = NULL;
	rh.dataSetDirectory = NULL;

	return rh; 
}
/***
*
*
***/
static int initializeConnection (IECReportHandle * iecrh)
{
	iecrh->con =  IedConnection_create();   
	IedConnection_connect(iecrh->con, &iecrh->error, iecrh->hostname, iecrh->port);
	
	if (iecrh->error != IED_ERROR_OK){
		syslog(LOG_ERR, "Error openening connection");
		return -1;
	}
	else{
		syslog(LOG_INFO, "Connection stablished");
		return 0;
	}
		
}

void sigint_iechandler(int signalId)
{
    exit (-1);
}
/***
*
*
***/
static int closeConnection (IECReportHandle * iecrh)
{
        /* disable reporting */
        ClientReportControlBlock_setRptEna(iecrh->rcb, false);
        IedConnection_setRCBValues(iecrh->con, &iecrh->error, iecrh->rcb, RCB_ELEMENT_RPT_ENA, true);

        IedConnection_close(iecrh->con);

        if (iecrh->clientDataSet)
            ClientDataSet_destroy(iecrh->clientDataSet);

        if (iecrh->rcb)
            ClientReportControlBlock_destroy(iecrh->rcb);

        if (iecrh->dataSetDirectory)
            LinkedList_destroy(iecrh->dataSetDirectory);
}
/**
* function:setValue
* return: O if OK, 1 if error
*/
static int setValue (char *id_104, char* value) 
{

	int res = 0 ;
	value = "10";
	syslog(LOG_DEBUG,"Value for  104 ID %s with value %d\n", id_104,atoi(value));

	switch (atoi(id_104)){			
		case 28600 : boolMapping.boolStruct.POINT_28600_VALUE = atoi(value) ; return 0 ;break;
		case 28602 : boolMapping.boolStruct.POINT_28602_VALUE = atoi(value) ; return 0 ;break;
		case 28604 : boolMapping.boolStruct.POINT_28604_VALUE = atoi(value) ; return 0 ;break;
		case 28606 : boolMapping.boolStruct.POINT_28606_VALUE = atoi(value) ; return 0 ;break;
		case 28608 : boolMapping.boolStruct.POINT_28608_VALUE = atoi(value) ; return 0 ;break;
		case 28610 : boolMapping.boolStruct.POINT_28610_VALUE = atoi(value) ; return 0 ;break;
		case 28612 : boolMapping.boolStruct.POINT_28612_VALUE = atoi(value) ; return 0 ;break;
		case 28614 : boolMapping.boolStruct.POINT_28614_VALUE = atoi(value) ; return 0 ;break;
		
		case 28960 : anagMapping.anagStruct.POINT_28960_VALUE = atof(value) ; return 0 ;break;
		case 28961 : anagMapping.anagStruct.POINT_28961_VALUE = atof(value) ; return 0 ;break;
		case 28962 : anagMapping.anagStruct.POINT_28962_VALUE = atof(value) ; return 0 ;break;
		case 28963 : anagMapping.anagStruct.POINT_28963_VALUE = atof(value) ; return 0 ;break;
		case 28972 : anagMapping.anagStruct.POINT_28972_VALUE = atof(value) ; return 0 ;break;
		case 28973 : anagMapping.anagStruct.POINT_28973_VALUE = atof(value) ; return 0 ;break;
		case 28974 : anagMapping.anagStruct.POINT_28974_VALUE = atof(value) ; return 0 ;break;
		case 28975 : anagMapping.anagStruct.POINT_28975_VALUE = atof(value) ; return 0 ;break;
		default:
			return 1;
	}
}

/**
* function:set104Value
* return: O if OK, 1 if error
*/
static int set104Value (char *name, char* value) 
{
	syslog(LOG_DEBUG,"Starting set104Value for  %s with value %s\n", name,value);
	
	int res = 1 ;
	for (int i=0;i<config.iec_count ; i++){
		//syslog(LOG_DEBUG,"Base tag name %s configuration entry %s",name,config.ids_iec[i]);
		if (strstr (name, config.ids_iec[i]) != NULL){
			res = setValue(config.ids_104[i],value);
		}	
	}
	
	if (res == 0)
		syslog(LOG_INFO,"Value set for %s",name);
	else
		syslog(LOG_DEBUG,"IEC Entry not found: %s", name);
	
	return res;
}
/***
*
*
***/
void reportCallbackFunction(void* parameter, ClientReport report)
{
    LinkedList dataSetDirectory = (LinkedList) parameter;
    MmsValue* dataSetValues = ClientReport_getDataSetValues(report);
    
    syslog(LOG_INFO,"received report for %s with rptId %s\n", ClientReport_getRcbReference(report), ClientReport_getRptId(report));

    if (ClientReport_hasTimestamp(report)) {
        time_t unixTime = ClientReport_getTimestamp(report) / 1000;

		char timeBuf[30];
		ctime_r(&unixTime, timeBuf);
        printf("  report contains timestamp (%u): %s", (unsigned int) unixTime, timeBuf);
    }
    
    if (dataSetDirectory) {
        for (int i = 0; i < LinkedList_size(dataSetDirectory); i++) {
            ReasonForInclusion reason = ClientReport_getReasonForInclusion(report, i);

            if (reason != IEC61850_REASON_NOT_INCLUDED) {

                char valBuffer[500];
                sprintf(valBuffer, "no value");
                if (dataSetValues) {
                    MmsValue* value = MmsValue_getElement(dataSetValues, i);
                    if (value) {
                        MmsValue_printToBuffer(value, valBuffer, 500);
                    }
                }
                LinkedList entry = LinkedList_get(dataSetDirectory, i);
                char* entryName = (char*) entry->data;
                /*Finally we set the values ready for 104 protocol*/
                set104Value (entryName, valBuffer);
                syslog(LOG_INFO,"  %s (included for reason %i): %s\n", entryName, reason, valBuffer);
            }
        }
    }
}

/***
*
*
***/
static int reportConfiguration (IECReportHandle * iecrh)
{	
        /* read data set directory */
        iecrh->dataSetDirectory = IedConnection_getDataSetDirectory(iecrh->con, &iecrh->error, "virtualIEDGenericMeasurement/LLN0.measurements_ds", NULL);

        if (iecrh->error != IED_ERROR_OK) {
            syslog(LOG_ERR,"Reading data set directory failed!\n");
       		return -1;
        }

        /* read data set */
        iecrh->clientDataSet = IedConnection_readDataSetValues(iecrh->con, &iecrh->error, "virtualIEDGenericMeasurement/LLN0.measurements_ds", NULL);

        if (iecrh->clientDataSet == NULL) {
            syslog(LOG_ERR,"failed to read dataset\n");
           	return -1;
        }

        /* Read RCB values */
        iecrh->rcb = IedConnection_getRCBValues(iecrh->con, &iecrh->error, "virtualIEDGenericMeasurement/LLN0.RP.Measurements01", NULL);

        if (iecrh->error != IED_ERROR_OK) {
            syslog(LOG_ERR,"getRCBValues service error!\n");
        	return -1;
        }

        /* prepare the parameters of the RCP */
        ClientReportControlBlock_setResv(iecrh->rcb, true);
        ClientReportControlBlock_setTrgOps(iecrh->rcb, TRG_OPT_DATA_CHANGED | TRG_OPT_QUALITY_CHANGED | TRG_OPT_GI);
        ClientReportControlBlock_setDataSetReference(iecrh->rcb, "virtualIEDGenericMeasurement/LLN0$measurements_ds"); /* NOTE the "$" instead of "." ! */
        ClientReportControlBlock_setRptEna(iecrh->rcb, true);
        ClientReportControlBlock_setGI(iecrh->rcb, true);

		IedConnection_installReportHandler(iecrh->con, "virtualIEDGenericMeasurement/LLN0.BR.Measurements01", ClientReportControlBlock_getRptId(iecrh->rcb), reportCallbackFunction,
                (void*) iecrh->dataSetDirectory);
        IedConnection_setRCBValues(iecrh->con, &iecrh->error, iecrh->rcb, RCB_ELEMENT_RESV | RCB_ELEMENT_DATSET | RCB_ELEMENT_TRG_OPS | RCB_ELEMENT_RPT_ENA , true);
	
		return 0 ;
}
/***
*
*
***/
static int reportPolling (IECReportHandle * iecrh)
{
	signal(SIGINT, sigint_iechandler);
	while (true) {
		sched_yield(); 
    	Thread_sleep(1000);
    	
		IedConnection_setRCBValues(iecrh->con, &iecrh->error, iecrh->rcb, RCB_ELEMENT_GI, true);
        IedConnectionState conState = IedConnection_getState(iecrh->con);

        if (conState != IED_STATE_CONNECTED) {
            printf("Connection closed by server!\n");
            break;
        }
    }
}
/***
*
*
***/
int Gi = 10;
void* iecReportHandle(void* p){

    char* hostname=config.hostname;
    int tcpPort = 102;
    int res = 0 ;

 	IECReportHandle iecrh = class_init(hostname,tcpPort);
 	
 	if (initializeConnection (&iecrh) == -1){
 		printf ("Not able to stablish connection...\n");
 		exit (-1) ;	
 	}
 	if (reportConfiguration (&iecrh) == -1){
 		printf ("Not able to confugre..\n");
 		exit (-1) ;
 	}
 	if (reportPolling (&iecrh) == -1) {
 		printf ("Not able to start Polling..\n");
 		exit (-1) ;
 	}
 	 	 
  	pthread_exit(&Gi);
}
