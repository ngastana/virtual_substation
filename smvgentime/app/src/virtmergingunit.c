/*
 * virtmergingunit.c
 *
 * Publicador de SV con appID y tiempo de ejecución metidos por el usuario.
 * Se imprime también el tiempo entre 2 SMVs y entre 4000 SVs. 
 *
 */

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include "iec61850_server.h"
#include "sv_publisher.h"
#include "hal_thread.h"

#define _USE_MATH_DEFINES
#include <math.h>

/* Include the generated header with the model access handles */

static int running = 0;
static int svcbEnabled = 1;

void sigint_handler(int signalId)
{
    running = 0;
}

static int amp1;
static int amp2;
static int amp3;
static int amp4;

static int amp1q;
static int amp2q;
static int amp3q;
static int amp4q;

static int vol1;
static int vol2;
static int vol3;
static int vol4;

static int vol1q;
static int vol2q;
static int vol3q;
static int vol4q;

static SVPublisher svPublisher;
static SVPublisher_ASDU asdu;

static void
setupSVPublisher(const char* svInterface, CommParameters* parameters)
{   
    svPublisher = SVPublisher_create(parameters, svInterface);


    if (svPublisher) {

        asdu = SVPublisher_addASDU(svPublisher, "xxxxMUnn01", NULL, 1);

        amp1 = SVPublisher_ASDU_addINT32(asdu);
        amp1q = SVPublisher_ASDU_addQuality(asdu);
        amp2 = SVPublisher_ASDU_addINT32(asdu);
        amp2q = SVPublisher_ASDU_addQuality(asdu);
        amp3 = SVPublisher_ASDU_addINT32(asdu);
        amp3q = SVPublisher_ASDU_addQuality(asdu);
        amp4 = SVPublisher_ASDU_addINT32(asdu);
        amp4q = SVPublisher_ASDU_addQuality(asdu);

        vol1 = SVPublisher_ASDU_addINT32(asdu);
        vol1q = SVPublisher_ASDU_addQuality(asdu);
        vol2 = SVPublisher_ASDU_addINT32(asdu);
        vol2q = SVPublisher_ASDU_addQuality(asdu);
        vol3 = SVPublisher_ASDU_addINT32(asdu);
        vol3q = SVPublisher_ASDU_addQuality(asdu);
        vol4 = SVPublisher_ASDU_addINT32(asdu);
        vol4q = SVPublisher_ASDU_addQuality(asdu);

        SVPublisher_ASDU_setSmpCntWrap(asdu, 4000);
        SVPublisher_ASDU_setRefrTm(asdu, 0);

        SVPublisher_setupComplete(svPublisher);
    }
}

int 
main(int argc, char** argv)
{
    openlog("virtual_merging_unit", LOG_CONS | LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Virtual Merging Unit initialization");

    char* svInterface;

    CommParameters parameters;
    uint16_t appId;
    char *p;
    uint64_t sleep_ns;

    if (argc > 3) {
        svInterface = argv[1];
        appId = strtol(argv[2], &p, 16);
        sleep_ns = strtol(argv[3], &p, 10);
    }
    else {
        svInterface = "eth0";
        appId = 0x4000;
        sleep_ns = 100000;
    }

    running = 1;

    signal(SIGINT, sigint_handler);

    printf("SETUP SV");
    parameters.dstAddress[0]=0x01;
    parameters.dstAddress[1]=0x0c;
    parameters.dstAddress[2]=0xcd;
    parameters.dstAddress[3]=0x01;
    parameters.dstAddress[4]=0x00;
    parameters.dstAddress[5]=0x01;
    parameters.vlanPriority = 4;
    parameters.vlanId = 0;
    parameters.appId = appId;

    setupSVPublisher(svInterface, &parameters);

    if (svPublisher) 
    {
        Quality q = QUALITY_VALIDITY_GOOD;

        int vol = (int) (6350.f * sqrt(2));
        int ampia = 2;
        int amp = 0;
        float phaseAngle = 0.f;

        int voltageA;
        int voltageB;
        int voltageC;
        int voltageN;
        int currentA;
        int currentB;
        int currentC;
        int currentN;

        int sampleCount = 0;
        uint64_t timeval0, timeval, vuelta, vuelta_ant;

        while (running) {	
            /* update measurement values */
            int samplePoint = sampleCount % 80;

            double angleA = (2 * M_PI / 80) * samplePoint;
            double angleB = (2 * M_PI / 80) * samplePoint - ( 2 * M_PI / 3);
            double angleC = (2 * M_PI / 80) * samplePoint - ( 4 * M_PI / 3);

            voltageA = (vol * sin(angleA)) * 100;
            voltageB = (vol * sin(angleB)) * 100;
            voltageC = (vol * sin(angleC)) * 100;
            voltageN = voltageA + voltageB + voltageC;

            currentA = (ampia * sin(angleA - phaseAngle)) * 1000;
            currentB = (amp * sin(angleB - phaseAngle)) * 1000;
            currentC = (amp * sin(angleC - phaseAngle)) * 1000;
            currentN = currentA + currentB + currentC;

            if (svcbEnabled) {

                SVPublisher_ASDU_setINT32(asdu, amp1, currentA);
                SVPublisher_ASDU_setQuality(asdu, amp1q, q);
                SVPublisher_ASDU_setINT32(asdu, amp2, currentB);
                SVPublisher_ASDU_setQuality(asdu, amp2q, q);
                SVPublisher_ASDU_setINT32(asdu, amp3, currentC);
                SVPublisher_ASDU_setQuality(asdu, amp3q, q);
                SVPublisher_ASDU_setINT32(asdu, amp4, currentN);
                SVPublisher_ASDU_setQuality(asdu, amp4q, q);

                SVPublisher_ASDU_setINT32(asdu, vol1, voltageA);
                SVPublisher_ASDU_setQuality(asdu, vol1q, q);
                SVPublisher_ASDU_setINT32(asdu, vol2, voltageB);
                SVPublisher_ASDU_setQuality(asdu, vol2q, q);
                SVPublisher_ASDU_setINT32(asdu, vol3, voltageC);
                SVPublisher_ASDU_setQuality(asdu, vol3q, q);
                SVPublisher_ASDU_setINT32(asdu, vol4, voltageN);
                SVPublisher_ASDU_setQuality(asdu, vol4q, q);

                SVPublisher_ASDU_setRefrTmNs(asdu, Hal_getTimeInNs());

                SVPublisher_ASDU_setSmpCnt(asdu, (uint16_t) sampleCount);

                SVPublisher_publish(svPublisher);
            }

	    sampleCount = ((sampleCount + 1) % 4000);

	    printf("Tiempo entre 2 SMVs: %ld\n",(((uint64_t)Hal_getTimeInNs())-timeval0));
	    timeval0 = Hal_getTimeInNs();
	    timeval = timeval0-sleep_ns*100;
	    while (timeval < timeval0 + sleep_ns) {
	        timeval = Hal_getTimeInNs();
            }

            if (sampleCount == 100) {
                vuelta_ant = vuelta;
                vuelta = Hal_getTimeInNs();
                printf("Tiempo entre 4000 SMVs: %ld\n",(vuelta-vuelta_ant));
            }
        }

        /* Cleanup - free all resources */
        SVPublisher_destroy(svPublisher);
        closelog();
    }
    else {
        printf("Cannot start SV publisher!\n");
    }
} /* main() */
