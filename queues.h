#include "objs.h"

void appendQueue(node **head, node *aNode);
void insertSJF(node **head, node *newNode);
void insertFIFO(node **head, node *newNode);

void printJobQueue(node *head);
void printProcQueue(node *head);
void printFinishedJobs(node *head);
void printTAT();
void printRunningProc(node *head);

node *findNode(node **head, config *systemConfig);
node *findProc(node **head, int keyID);

job *deleteJobNode(node **head, int keyID);
node *removeProcNode(node **head, int keyID);

void moveJobToReadyQueue(job *aJob, config *systemConfig);
