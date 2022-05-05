#include "objs.h"

process *createProc(job* aJob){
    process *aProc = malloc(sizeof(process));
    aProc->pid = aJob->job_id;
    aProc->arrival_time = aJob->arrival_time;
    aProc->burst = aJob->burst;
    aProc->allocated_memory = aJob->memory;
    aProc->devices = aJob->devices;
    aProc->running_time = 0;
    aProc->wait_time = 0;
    aProc->turnaround_time = 0;
    return aProc;
}

job *createJob(int* inputs){
    job *aJob = malloc(sizeof(job));
    aJob->arrival_time = inputs[1];
    aJob->job_id = inputs[2];
    aJob->memory = inputs[3];
    aJob->devices = inputs[4];
    aJob->burst = inputs[5];
    aJob->priority = inputs[6];
    return aJob;
}

request *createRequest(int* inputs){
    request *aRequest = malloc(sizeof(request));
    aRequest->time = inputs[1];
    aRequest->job_id = inputs[2];
    aRequest->devices = inputs[3];
    return aRequest;
}

release *createRelease(int* inputs){
    release *aRelease = malloc(sizeof(release));
    aRelease->time = inputs[1];
    aRelease->job_id = inputs[2];
    aRelease->devices = inputs[3];
    return aRelease;
}