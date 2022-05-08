#include "objs.h"

void appendQueue(node **head, node *aNode);
void insertSJF(node **head, node *newNode);
void insertFIFO(node **head, node *newNode);


node *findNode(node **head, config *systemConfig);
job *deleteJobNode(node **head, int keyID);

void moveJobToReadyQueue(job *aJob, config *systemConfig);