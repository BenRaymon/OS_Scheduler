#include "queues.h"
#include <stdbool.h>

void processInputEvent(int* inputs, config *systemConfig);
int* parseInput(char* input);

void printQueue(node* head);
void printAllQueues(config *systemConfig);

//scheduling
void roundRobin(config *systemConfig);

//deadlocks
bool checkRequest(process *proc, config *systemConfig);
int isSafe(config *systemConfig);
void deadlockHandling(config *systemConfig);

void checkWaitQueue(config *systemConfig);
void checkHoldQueues(config *systemConfig);
