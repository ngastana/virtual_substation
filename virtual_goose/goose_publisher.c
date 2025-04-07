#include <stdio.h>
#include "goose_publisher.h"

int send_goose_message(const GooseMessage *msg) 
{
    printf("SIMULANDO GOOSE-s:\n");
    printf("  goId: %s\n", msg->goId);
    printf("  trip: %d\n", msg->trip);
    printf("  timestamp: %lu\n", msg->timestamp);
    printf("  stNum: %u\n", msg->stNum);
    printf("  sqNum: %u\n", msg->sqNum);
    return 0;
}
