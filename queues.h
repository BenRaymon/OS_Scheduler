#include "objs.h"

void appendQueue(node **head, node *aNode);
void insertSJF(node **head, node *newNode);
void insertFIFO(node **head, node *newNode);

void printJobQueue(node *head);
void printProcQueue(node *head);
void printFinishedJobs(node *head);
void printRunningProc(node *head);

node *findNode(node **head, config *systemConfig);
job *deleteJobNode(node **head, int keyID);

void moveJobToReadyQueue(job *aJob, config *systemConfig);
