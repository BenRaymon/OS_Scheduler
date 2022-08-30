#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

typedef struct request {
    int id;
    int time;
    int devices;
} request;

typedef struct release {
    int id;
    int time;
    int devices;
} release;

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
    int allocated_devices;
    request *request;
} process;

typedef struct node{
    job* job;
    process* proc;
    struct node* next;
} node;

typedef struct config {
    int start_time;
    int total_memory;
    int available_memory;
    int total_devices;
    int available_devices;
    int quantum;
} config;

job* createJob(int* inputs);
request* createRequest(int* inputs);
release* createRelease(int* inputs);
process* createProc();
