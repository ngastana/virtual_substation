

/*H**********************************************************************
* FILENAME :        config.h             PACKAGE REF: virtual-104-gtw
*
* DESCRIPTION :
*       Configuration file reading. Relies on inih module.
*
* PUBLIC FUNCTIONS :
*       int     readConfiguration( configuration *config )
*
* NOTES :
*       Handles 104-gtw IED configuration read. This maps 61850 report variables
*		into IEC104 ASDU objects
*H*/

#ifndef CONFIG_H
#define CONFIG_H


#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "ini.h"
#include <string.h>
#include <syslog.h>

typedef struct
{
    int version;
    const char* name;
    const char* hostname;
    const char* device;
    const char* ln_name;
    const char* ds_name;
    const char* rp_name;
    int iec_count ;
    const char ids_iec [50][50];
    const char ids_104 [50][50];
    
    
} configuration;

int readConfiguration (configuration *config);
/*
int handler(void* user, const char* section, const char* name,
                   const char* value)
*/
#endif /* CONFIG_H */

/*
[IED]             ; Protocol configuration
name=virtualIED
device=GenericMeasure     
ln=LLN0 ; So far we asume tha there is only one line

[DATASETS]
count=2
ds00=Measurementents_ds
ds01=iocTrip_ds

[REPORTS]
count=1
rc01=Measurements
*/