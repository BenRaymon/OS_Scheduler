#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>


typedef struct Job {
    int arrival_time;
    int job_id;
    int memory;
    int devices;
    int burst;
    int priority;
} job;

typedef struct node{
    job* job;
    struct node* next;
    struct node* prev;
} node;

typedef struct Configuration {
    int start_time;
    int memory;
    int devices;
    int quantum;
} config;

typedef struct Request {
    int job_id;
    int time;
    int devices;
} request;

typedef struct Release {
    int job_id;
    int time;
    int devices;
} release;


int* parseInput(char* input);

void insertHQ1(node* newNode);
void insertHQ2(node* newNode);

void printList(node* head);