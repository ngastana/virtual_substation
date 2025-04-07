#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <syslog.h>
#include <time.h>
#include "goose_publisher.h"

// Variable global para manejar la terminación con CTRL+C
static int running = 1;

// Manejador de la señal SIGINT para permitir una terminación limpia
void sigint_handler(int sig) 
{
    running = 0;
}

int main(int argc, char **argv) {
    openlog("virtual_goose", LOG_PERROR | LOG_PID, LOG_USER);
    syslog(LOG_INFO, "QUE EMPIEZEN LOS GOOSES: delay = 5");

    unsigned int delay = 5;
    for (int i = 0; i < 3; i++)
    {
        if (argc > 1) {
            delay = atoi(argv[1]);
        }
        syslog(LOG_INFO, "Esperando %u segundos antes de enviar el mensaje GOOSE", delay);
        sleep(delay);

        GooseMessage msg;
        msg.goId = "virtualGoose";
        msg.trip = (i % 2 == 0);
        msg.timestamp = (unsigned long) time(NULL);
        msg.stNum = 1;
        msg.sqNum = 1;

        syslog(LOG_INFO, "Enviando mensaje GOOSE atípico...");
        if (send_goose_message(&msg) != 0) {
            syslog(LOG_ERR, "Error al enviar el mensaje GOOSE");
        } else {
            syslog(LOG_INFO, "Mensaje GOOSE enviado correctamente");
        }
    }
    closelog();
    return 0;
}
