#include "queues.h"

void processInputEvent(int* inputs, config *systemConfig);
int* parseInput(char* input);

void printQueue(node* head);
void printAllQueues(config *systemConfig);

void roundRobin(config *systemConfig);