#include "queues.h"

void processInputEvent(int* inputs, config *systemConfig);
int* parseInput(char* input);

void printQueue(node* head);
void printAllQueues();
void printAllJobs();

void roundRobin(config *systemConfig);