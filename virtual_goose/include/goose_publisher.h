#ifndef GOOSE_PUBLISHER_H
#define GOOSE_PUBLISHER_H

typedef struct 
{
    const char* goId;          // Identificador del mensaje GOOSE.
    int trip;                  // Valor que determina el disparo (0 = false, 1 = true).
    unsigned long timestamp;   // Marca de tiempo del mensaje.
    unsigned int stNum;        // Número de estado.
    unsigned int sqNum;        // Número de secuencia.
} GooseMessage;

int send_goose_message(const GooseMessage *msg);

#endif // GOOSE_PUBLISHER_H
