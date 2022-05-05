#include "queues.h"

void processInputEvent(int* inputs, config *systemConfig);
int* parseInput(char* input);

void printList(node* head);
void printAll();

void roundRobin(config *systemConfig);