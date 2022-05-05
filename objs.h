#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

typedef struct job {
    int arrival_time;
    int job_id;
    int memory;
    int devices;
    int burst;
    int priority;
} job;

typedef struct process {
    int pid;
    int burst;
    int running_time;
    int arrival_time;
    int completion_time;
    int wait_time;
    int turnaround_time;
    int allocated_memory;
    int devices;
} process;

typedef struct node{
    job* job;
    process* proc;
    struct node* next;
} node;

typedef struct configuration {
    int start_time;
    int total_memory;
    int available_memory;
    int devices;
    int quantum;
} config;

typedef struct request {
    int job_id;
    int time;
    int devices;
} request;

typedef struct release {
    int job_id;
    int time;
    int devices;
} release;

job* createJob(int* inputs);
request* createRequest(int* inputs);
release* createRelease(int* inputs);
process* createProc();
