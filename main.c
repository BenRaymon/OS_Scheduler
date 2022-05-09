#include "test.h"

int currentTime = 0;
node *holdQueueOne;
node *holdQueueTwo;
node *readyQueue;
node *waitingQueue;
node *finishedQueue;
node *runningProc;
//node *incomingRequests;

int main(int argc, char *argv[]) {

    int* inputs = NULL;

    holdQueueOne = NULL;
    holdQueueTwo = NULL;

    readyQueue = NULL;
    waitingQueue = NULL;
    finishedQueue= NULL;
    
    runningProc = NULL;
    //incomingRequests = NULL;

    
    config *systemConfig = malloc(sizeof(config));

    FILE *file = fopen(argv[2], "r" );


    if (file != NULL) {
        char line[1000];
        
        while(currentTime < 9999){
            
            currentTime += 1;

            /* Process the next line of input from file */
            if(inputs == NULL && fgets(line,sizeof(line),file)!= NULL){
                inputs = parseInput(line);
                
                //inputs[1] will hold the arrival time
                //wait for arrival to process the external event
                if(!(inputs[1] > currentTime && inputs[1] != 9999)){
                    processInputEvent(inputs, systemConfig);
                    inputs = NULL;
                }
            } else if (inputs && inputs[1] <= currentTime){
                processInputEvent(inputs, systemConfig);
                inputs = NULL;
            }
            else {
                //If there is nothing left to do, exit
                if(readyQueue == NULL && runningProc == NULL && inputs == NULL){
                    currentTime = 100000;
                }
            }

            /* CHECK HOLD QUEUES */
            // Can any jobs in HQ be moved to RQ?
            // If yes, remove node from HQ and move to RQ
            //HQ1 higher priority, search HQ1 first then HQ2
            node *tempNode;
            tempNode = findNode(&holdQueueOne, systemConfig);
            if(tempNode){
                job *aJob = deleteJobNode(&holdQueueOne, tempNode->job->job_id);
                if(aJob){
                    moveJobToReadyQueue(aJob, systemConfig);
                    tempNode->job = NULL;
                }
            }

            tempNode = findNode(&holdQueueTwo, systemConfig);
            if(tempNode){
                job *aJob = deleteJobNode(&holdQueueTwo, tempNode->job->job_id);
                if(aJob){
                    moveJobToReadyQueue(aJob, systemConfig);
                    tempNode->job = NULL;
                }
            }
            

            /* SCHEDULE PROCESSES */
            //BEFORE SCHEDULING
            printf("TIME: %d\n", currentTime);
            printf("BEFORE\n");
            printAllQueues(systemConfig);
            // ROUND ROBIN SCHEDULING - SINGLE TICK ON CPU
            roundRobin(systemConfig);
            //AFTER SCHEDULING
            printf("AFTER\n");
            printAllQueues(systemConfig);
           
        }
        fclose(file);
    }
}

//create a process for a given job
//create node to hold process
//add new node to ready queue
void moveJobToReadyQueue(job *aJob, config *systemConfig){
    process *aProc = createProc(aJob); //create proc for this job
    node *aNode = malloc(sizeof(node)); //create new node
    aNode->proc = aProc; //set the proc of the node
    free(aJob); //free the job that is no longer needed
    systemConfig->available_memory = systemConfig->available_memory - aProc->allocated_memory; //reduce system mem accordingly
    appendQueue(&readyQueue, aNode); //add node to ready queue
}

//ROUND ROBIN SCHEDULING ALGORITHM
//if no running proc, moves proc from RQ to CPU
//gives proc on CPU one tick
//removes proc from CPU if quantum up or execution is complete
//  if proc is removed from CPU, new proc is put on CPU
void roundRobin(config *systemConfig){
    //take a process off the ready queue and move it to the CPU
    if(readyQueue && runningProc == NULL){
        runningProc = readyQueue;
        readyQueue = readyQueue->next;
    }
    //Run the process for 1 tick on the CPU
    if(runningProc){
        runningProc->proc->running_time += 1;
        
        if(runningProc->proc->running_time == runningProc->proc->burst){
            //process is done, take off CPU
            appendQueue(&finishedQueue, runningProc);
            runningProc->proc->completion_time = currentTime;
            runningProc->proc->turnaround_time = currentTime - runningProc->proc->arrival_time;
            runningProc->proc->wait_time = runningProc->proc->turnaround_time - runningProc->proc->running_time;
            systemConfig->available_memory += runningProc->proc->allocated_memory;

            runningProc -> next = NULL;
            runningProc = NULL;

            //put new proc on CPU
            if(readyQueue){
                runningProc = readyQueue;
                readyQueue = readyQueue->next;
            }

        } else if(runningProc->proc->running_time % systemConfig->quantum == 0){
            //quantum is up, time to go back to the ready queue
            appendQueue(&readyQueue,runningProc);
            runningProc->next = NULL;
            runningProc = NULL;

            //put new proc on CPU
            if(readyQueue){
                runningProc = readyQueue;
                readyQueue = readyQueue->next;
            }
        }
    }
}


//create the necessary objects based on the input event
void processInputEvent(int *inputs, config *systemConfig){
    if(inputs[0] == 'C'){
        fprintf(stdout, "System Configuration\n");

        //Create configuration
        systemConfig->start_time = inputs[1];
        systemConfig->total_memory = inputs[2];
        systemConfig->available_memory = inputs[2];
        systemConfig->devices = inputs[3];
        systemConfig->quantum = inputs[4];

        free(inputs);
        inputs = NULL;

    } else if (inputs[0] == 'A'){
        fprintf(stdout, "Job Arrival\n");
        
        //Create Job
        job *aJob = createJob(inputs);
        
        free(inputs);
        inputs = NULL;

        if(aJob->memory > systemConfig->total_memory){
            //NOT ENOUGH TOTAL MEMORY
        } else if (aJob->devices > systemConfig->devices){
            //NOT ENOUGH DEVICES
        } else if (aJob->memory <= systemConfig->available_memory){
            //THERE IS ENOUGH AVAILABLE MEMORY, CREATE A PROCESS AND PUT IN READY QUEUE
            moveJobToReadyQueue(aJob, systemConfig);

        } else {
            //THERE IS NOT ENOUGH AVAILABLE MEMORY, PUT IN HOLD QUEUE
            node *aNode = malloc(sizeof(node));
            aNode -> job = aJob;

            if(aJob->priority == 1){
                insertSJF(&holdQueueOne, aNode);
            } else if (aJob->priority == 2){
                insertFIFO(&holdQueueTwo, aNode);
            }
        }

        
    } else if (inputs[0] == 'Q'){
        fprintf(stdout, "Request for Devices\n");

        //Create request
        request *aRequest = createRequest(inputs);
        //appendQueue(&incomingRequests, aRequest);

        free(inputs); 
        inputs = NULL;

    } else if (inputs[0] == 'L'){
        fprintf(stdout, "Release of Devices\n");

        //Create release
        release *aRelease = createRelease(inputs);

        free(inputs);
        inputs = NULL;

    } else if (inputs[0] == 'D'){
        fprintf(stdout, "System Status\n");
    }
}

//return array of integers for a given string input line
int *parseInput(char* input){
    char* token = strtok(input, " ");
    char* subtext = malloc(sizeof(char) * 10); 
    int* inputs = malloc(7 * sizeof(int));

    inputs[0] = input[0];

    int counter = 1;
    //Split input string by space character
    while (token = strtok(0, " ")) {

        if (counter == 1){
            //first input is always just an integer
            inputs[counter] = atoi(token);
        } else {
            //extract integer (remove the = part)
            memcpy(subtext,&token[2], strlen(token));
            inputs[counter] = atoi(subtext);
        }

        counter++;
    }

    inputs[counter] = -1; //value to note end of list defined as -1
    free(subtext);

    return inputs;
}




/* Print all the Queues */
void printAllQueues(config *systemConfig){

    printf("At time %d: \n", currentTime);
    printf("Current Available Main Memory=%d\n", systemConfig->available_memory);
    printf("Current Devices=%d\n", systemConfig->devices);

    printf("\nCompleted Jobs: \n");
    printFinishedJobs(finishedQueue);

    printf("\nHold Queue 1:\n");
    printJobQueue(holdQueueOne);
    
    printf("\nHold Queue 2:\n");
    printJobQueue(holdQueueTwo);
    
    printf("\nReady Queue: \n");
    printProcQueue(readyQueue);
    
    printf("\nProcess running on CPU: \n");
    printRunningProc(runningProc);

    printf("\nWait Queue: \n");
    printProcQueue(readyQueue);
    
}