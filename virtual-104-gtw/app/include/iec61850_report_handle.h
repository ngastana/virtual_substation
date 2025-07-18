#ifndef IEC_61850_REPORT_HANDLE_H_
#define IEC_61850_REPORT_HANDLE_H_


#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <signal.h>

#include "iec61850_client.h"

typedef uint64_t nsSinceEpoch;
typedef uint64_t msSinceEpoch;

void* iecReportHandle(void* p);


typedef struct {
    //Data Variables
    char 	*hostname	;
    int  	port;
    
    IedClientError error;
	IedConnection con ;
	ClientReportControlBlock rcb;
    ClientDataSet clientDataSet ;
    LinkedList dataSetDirectory ;

  
} IECReportHandle;

IECReportHandle class_init (char* , int);


// // Function pointer prototypes used by these classes
// typedef int sub_func_t (int);
// typedef float sub_funcf_t (int,int);
// 
// /* class type definition 
//   (emulated class type definition; C doesn't really have class types) */
// typedef struct {
//     //Data Variables
//     int a;
// 
//     /*Function (also known as Method) pointers
//       (note that different functions have the same function pointer prototype)*/
//     sub_func_t* add;
//     sub_func_t* subt;
//     sub_func_t* mult;
//     sub_funcf_t* div;  
// } class_name;
// 
// 
// // class init prototypes
// // These inits connect the function pointers to specific functions
// // and initialize the variables.
// class_name* class_init_ptr (int, sub_func_t*, sub_func_t*, sub_func_t*, sub_funcf_t*);
// class_name class_init (int, sub_func_t*, sub_func_t*, sub_func_t*, sub_funcf_t*);


#endif