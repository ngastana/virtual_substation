
#include "config.h"


int entries_count = 0 ;

#define MAX_SUBSTRINGS 100
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    
static int splitString(const char* input, const char* delimiter, char* substrings[]) {
    int numSubstrings = 0;
    
    char* token = strtok(input, delimiter);
    while (token != NULL && numSubstrings < MAX_SUBSTRINGS) {
        substrings[numSubstrings] = token;
        numSubstrings++;
        token = strtok(NULL, delimiter);
    }
    
    return numSubstrings;
}

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)user;
	char iec_entry[50];
	char* substrings[MAX_SUBSTRINGS];
	
    
    if (MATCH("IED", "version")) {
        pconfig->version = atoi(value);
    }else if (MATCH("IED", "hostname")) {
        pconfig->hostname = strdup(value); 
    }else if (MATCH("IED", "name")) {
        pconfig->name = strdup(value);
    } else if (MATCH("IED", "device")) {
        pconfig->device = strdup(value);
    } else if (MATCH("IED", "ln")) {
        pconfig->ln_name = strdup(value);
    } else if (MATCH("DATASETS", "ds00")) {
        pconfig->ds_name = strdup(value);
    } else if (MATCH("REPORTS", "rp00")) {
        pconfig->rp_name = strdup(value);
    }else if (MATCH("IEC_ENTRIES", "count")) {
        pconfig->iec_count = atoi(value);
    }
    else {
    	for (int i = 0; i<pconfig->iec_count ;i++){
    		sprintf (iec_entry,"e%d",i);
    		if (MATCH("IEC_ENTRIES", iec_entry)) {
    			splitString(value, ";", substrings);
    			strcpy(pconfig->ids_iec[entries_count] , substrings[0]);
    			strcpy(pconfig->ids_104[entries_count] , substrings[1]);
       			entries_count++;
    			return 1;
    		}
    	}
        return 0;  /* unknown section/name, error */
    }
    return 1;
}


int readConfiguration (configuration *config)
{
	//openlog("virtual_104_gtw", LOG_CONS | LOG_PID, LOG_USER);
	
    if (ini_parse("./app.ini", handler, config) < 0) {
    	syslog(LOG_ERR, "Error reading config ini file");
    	return 1 ;
    }
    else{
    	syslog(LOG_INFO,"Config loaded from 'app.ini': version=%d, name=%s, device=%s iec_count=%d\n",
        config->version, config->name, config->device,config->iec_count);
        for (int i=0;i<config->iec_count;i++)
        	syslog(LOG_INFO, "IEC Entry %s", config->ids_iec [i]);
        
        
        return 0 ;
    }
    	
    
}