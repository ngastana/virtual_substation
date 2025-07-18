// #include <signal.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <syslog.h>
// #include "sv_publisher.h"
// #include "hal_thread.h"

// #define _USE_MATH_DEFINES
// #include <math.h>

// /* Handles para datos SV */
// static int amp1, amp2, amp3, amp4;
// static int amp1q, amp2q, amp3q, amp4q;
// static int vol1, vol2, vol3, vol4;
// static int vol1q, vol2q, vol3q, vol4q;

// static SVPublisher svPublisher;
// static SVPublisher_ASDU asdu;
// static int running = 0;
// static int svcbEnabled = 1;

// void sigint_handler(int signalId) {
//     running = 0;
// }

// /// Inicializa el SVPublisher usando parámetros de comunicación por defecto (NULL)
// static void setupSVPublisher(const char* svInterface) {
//     svPublisher = SVPublisher_create(NULL, svInterface);
//     if (!svPublisher) {
//         fprintf(stderr, "ERROR: SVPublisher_create returned NULL\n");
//         exit(EXIT_FAILURE);
//     }

//     asdu = SVPublisher_addASDU(svPublisher, "xxxxMUnn01", NULL, 1);

//     amp1  = SVPublisher_ASDU_addINT32(asdu);
//     amp1q = SVPublisher_ASDU_addQuality(asdu);
//     amp2  = SVPublisher_ASDU_addINT32(asdu);
//     amp2q = SVPublisher_ASDU_addQuality(asdu);
//     amp3  = SVPublisher_ASDU_addINT32(asdu);
//     amp3q = SVPublisher_ASDU_addQuality(asdu);
//     amp4  = SVPublisher_ASDU_addINT32(asdu);
//     amp4q = SVPublisher_ASDU_addQuality(asdu);

//     vol1  = SVPublisher_ASDU_addINT32(asdu);
//     vol1q = SVPublisher_ASDU_addQuality(asdu);
//     vol2  = SVPublisher_ASDU_addINT32(asdu);
//     vol2q = SVPublisher_ASDU_addQuality(asdu);
//     vol3  = SVPublisher_ASDU_addINT32(asdu);
//     vol3q = SVPublisher_ASDU_addQuality(asdu);
//     vol4  = SVPublisher_ASDU_addINT32(asdu);
//     vol4q = SVPublisher_ASDU_addQuality(asdu);

//     SVPublisher_ASDU_setSmpCntWrap(asdu, 4000);
//     SVPublisher_ASDU_setRefrTm(asdu, 0);

//     SVPublisher_setupComplete(svPublisher);
// }

// int main(int argc, char** argv) {
//     openlog("virtual_merging_unit", LOG_CONS | LOG_PID, LOG_USER);
//     syslog(LOG_INFO, "Virtual Merging Unit initialization");

//     /* Selección de interfaz: argumento o eth0 por defecto */
//     const char* svInterface = (argc > 1) ? argv[1] : "eth0";

//     /* Instalación de handler para SIGINT */
//     running = 1;
//     signal(SIGINT, sigint_handler);

//     printf("SETUP SV\n");
//     setupSVPublisher(svInterface);

//     /* Bucle de envío indefinido */
//     Quality q = QUALITY_VALIDITY_GOOD;
//     int vol = (int)(6350.0f * sqrt(2));
//     int ampia = 2;
//     int sampleCount = 0;
//     uint64_t nextCycleStart = Hal_getTimeInMs() + 1000;

//     while (running) {
//         /* Cálculo de muestras de voltaje y corriente */
//         int samplePoint = sampleCount % 80;
//         double angleA = (2 * M_PI / 80) * samplePoint;
//         double angleB = angleA - (2 * M_PI / 3);
//         double angleC = angleA - (4 * M_PI / 3);

//         int voltageA = (int)(vol * sin(angleA) * 100);
//         int voltageB = (int)(vol * sin(angleB) * 100);
//         int voltageC = (int)(vol * sin(angleC) * 100);
//         int voltageN = voltageA + voltageB + voltageC;

//         int currentA = (int)(ampia * sin(angleA) * 1000);
//         int currentB = (int)(ampia * sin(angleB) * 1000);
//         int currentC = (int)(ampia * sin(angleC) * 1000);
//         int currentN = currentA + currentB + currentC;

//         /* Publicación SV */
//         SVPublisher_ASDU_setINT32(asdu, amp1, currentA);
//         SVPublisher_ASDU_setQuality(asdu, amp1q, q);
//         SVPublisher_ASDU_setINT32(asdu, amp2, currentB);
//         SVPublisher_ASDU_setQuality(asdu, amp2q, q);
//         SVPublisher_ASDU_setINT32(asdu, amp3, currentC);
//         SVPublisher_ASDU_setQuality(asdu, amp3q, q);
//         SVPublisher_ASDU_setINT32(asdu, amp4, currentN);
//         SVPublisher_ASDU_setQuality(asdu, amp4q, q);

//         SVPublisher_ASDU_setINT32(asdu, vol1, voltageA);
//         SVPublisher_ASDU_setQuality(asdu, vol1q, q);
//         SVPublisher_ASDU_setINT32(asdu, vol2, voltageB);
//         SVPublisher_ASDU_setQuality(asdu, vol2q, q);
//         SVPublisher_ASDU_setINT32(asdu, vol3, voltageC);
//         SVPublisher_ASDU_setQuality(asdu, vol3q, q);
//         SVPublisher_ASDU_setINT32(asdu, vol4, voltageN);
//         SVPublisher_ASDU_setQuality(asdu, vol4q, q);

//         SVPublisher_ASDU_setRefrTmNs(asdu, Hal_getTimeInNs());
//         SVPublisher_ASDU_setSmpCnt(asdu, (uint16_t)sampleCount);
//         SVPublisher_publish(svPublisher);

//         sampleCount = (sampleCount + 1) % 4000;

//         /* Sincronización: espera hasta el siguiente ciclo de 100 ms */
//         if ((sampleCount % 400) == 0) {
//             uint64_t now = Hal_getTimeInMs();
//             while (now < nextCycleStart + 100) {
//                 Thread_sleep(1);
//                 now = Hal_getTimeInMs();
//             }
//             nextCycleStart += 100;
//         }
//     }

//     /* Cleanup */
//     SVPublisher_destroy(svPublisher);
//     closelog();
//     return 0;
// }



#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include "sv_publisher.h"
#include "hal_thread.h"

#define _USE_MATH_DEFINES
#include <math.h>

/* Handles para datos SV */
static int amp1, amp2, amp3, amp4;
static int amp1q, amp2q, amp3q, amp4q;
static int vol1, vol2, vol3, vol4;
static int vol1q, vol2q, vol3q, vol4q;

static SVPublisher svPublisher;
static SVPublisher_ASDU asdu;
static int running = 0;
static int svcbEnabled = 1;

void sigint_handler(int signalId) {
    running = 0;
}

/// Inicializa el SVPublisher usando parámetros de comunicación por defecto (NULL)
static void setupSVPublisher(const char* svInterface) {
    svPublisher = SVPublisher_create(NULL, svInterface);
    if (!svPublisher) {
        fprintf(stderr, "ERROR: SVPublisher_create returned NULL\n");
        exit(EXIT_FAILURE);
    }

    asdu = SVPublisher_addASDU(svPublisher, "xxxxMUnn01", NULL, 1);

    amp1  = SVPublisher_ASDU_addINT32(asdu);
    amp1q = SVPublisher_ASDU_addQuality(asdu);
    amp2  = SVPublisher_ASDU_addINT32(asdu);
    amp2q = SVPublisher_ASDU_addQuality(asdu);
    amp3  = SVPublisher_ASDU_addINT32(asdu);
    amp3q = SVPublisher_ASDU_addQuality(asdu);
    amp4  = SVPublisher_ASDU_addINT32(asdu);
    amp4q = SVPublisher_ASDU_addQuality(asdu);

    vol1  = SVPublisher_ASDU_addINT32(asdu);
    vol1q = SVPublisher_ASDU_addQuality(asdu);
    vol2  = SVPublisher_ASDU_addINT32(asdu);
    vol2q = SVPublisher_ASDU_addQuality(asdu);
    vol3  = SVPublisher_ASDU_addINT32(asdu);
    vol3q = SVPublisher_ASDU_addQuality(asdu);
    vol4  = SVPublisher_ASDU_addINT32(asdu);
    vol4q = SVPublisher_ASDU_addQuality(asdu);

    SVPublisher_ASDU_setSmpCntWrap(asdu, 4000);
    SVPublisher_ASDU_setRefrTm(asdu, 0);

    SVPublisher_setupComplete(svPublisher);
}

int main(int argc, char** argv) {
    openlog("virtual_merging_unit", LOG_CONS | LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Virtual Merging Unit initialization");

    /* Selección de interfaz: argumento o eth0 por defecto */
    const char* svInterface = (argc > 1) ? argv[1] : "eth0";

    /* Instalación de handler para SIGINT */
    running = 1;
    signal(SIGINT, sigint_handler);

    printf("SETUP SV\n");
    setupSVPublisher(svInterface);

    /* Bucle de envío indefinido */
    Quality q = QUALITY_VALIDITY_GOOD;
    int vol = (int)(6350.0f * sqrt(2));
    int ampia = 2;
    int sampleCount = 0;
    uint64_t nextCycleStart = Hal_getTimeInMs() + 1000;

    while (running) {
        /* Cálculo de muestras de voltaje y corriente */
        int samplePoint = sampleCount % 80;
        double angleA = (2 * M_PI / 80) * samplePoint;
        double angleB = angleA - (2 * M_PI / 3);
        double angleC = angleA - (4 * M_PI / 3);

        int voltageA = (int)(vol * sin(angleA) * 100);
        int voltageB = (int)(vol * sin(angleB) * 100);
        int voltageC = (int)(vol * sin(angleC) * 100);
        int voltageN = voltageA + voltageB + voltageC;

        int currentA = (int)(ampia * sin(angleA) * 1000);
        int currentB = (int)(ampia * sin(angleB) * 1000);
        int currentC = (int)(ampia * sin(angleC) * 1000);
        int currentN = currentA + currentB + currentC;

        /* Publicación SV */
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
        SVPublisher_ASDU_setSmpCnt(asdu, (uint16_t)sampleCount);
        SVPublisher_publish(svPublisher);

        sampleCount = (sampleCount + 1) % 4000;

        /* Sincronización: espera hasta el siguiente ciclo de 100 ms */
        if ((sampleCount % 400) == 0) {
            uint64_t now = Hal_getTimeInMs();
            while (now < nextCycleStart + 100) {
                Thread_sleep(1);
                now = Hal_getTimeInMs();
            }
            nextCycleStart += 100;
        }
    }

    /* Cleanup */
    SVPublisher_destroy(svPublisher);
    closelog();
    return 0;
}
