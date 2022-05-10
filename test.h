#include "queues.h"
#include <stdbool.h>

void processInputEvent(int* inputs, config *systemConfig);
int* parseInput(char* input);

void printQueue(node* head);
void printAllQueues(config *systemConfig);

void roundRobin(config *systemConfig);


bool checkRequest(process *proc, config *systemConfig);