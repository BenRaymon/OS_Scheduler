#include "objs.h"

/*
 *  creates a process with the data from a given job
 *  
 *  @param aJob: the job to base the process off of
 *  @return process: the process that was created
 */
process *createProc(job* aJob){
    process *aProc = malloc(sizeof(process));
    aProc->pid = aJob->job_id;
    aProc->arrival_time = aJob->arrival_time;
    aProc->burst = aJob->burst;
    aProc->allocated_memory = aJob->memory;
    aProc->devices = aJob->devices;
    aProc->allocated_devices = 0;
    aProc->running_time = 0;
    aProc->wait_time = 0;
    aProc->turnaround_time = 0;
    aProc->request = NULL;
    return aProc;
}

/*
 *  creates a job with the data from a list of inputs
 *  
 *  @param inputs: a list of integers representing the data of a job
 *  @return job: the job that was created
 */
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

/*
 *  creates a request with the data from a list of inputs
 *  
 *  @param inputs: a list of integers representing the data of a request
 *  @return request: the request that was created
 */
request *createRequest(int* inputs){
    request *aRequest = malloc(sizeof(request));
    aRequest->time = inputs[1];
    aRequest->id = inputs[2];
    aRequest->devices = inputs[3];
    return aRequest;
}

/*
 *  creates a release with the data from a list of inputs
 *  
 *  @param inputs: a list of integers representing the data of a release
 *  @return release: the release that was created
 */
release *createRelease(int* inputs){
    release *aRelease = malloc(sizeof(release));
    aRelease->time = inputs[1];
    aRelease->id = inputs[2];
    aRelease->devices = inputs[3];
    return aRelease;
}