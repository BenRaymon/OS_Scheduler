#include "test.h"

int currentTime = 0;
bool alreadyScheduled = false;
node *holdQueueOne;
node *holdQueueTwo;
node *readyQueue;
node *waitingQueue;
node *finishedQueue;
node *runningProc;

config *systemConfig;
//TODO: REMOVE SYSTEMCONFIG AS AN ARGUMENT FROM EVERY FUNCTION
int* inputs;

int main(int argc, char *argv[]) {

    inputs = NULL;
    holdQueueOne = NULL;
    holdQueueTwo = NULL;
    readyQueue = NULL;
    waitingQueue = NULL;
    finishedQueue= NULL;
    runningProc = NULL;
    systemConfig = malloc(sizeof(config));
    
    FILE *file = fopen(argv[2], "r" );

    if (file != NULL) {
        char line[1000];
        
        while(currentTime < 9999){
            
            currentTime += 1;

            //process internal events before external events
            //if there is a proc in the ready queue or if there is a proc on the cpu
            //run this process before reading the next input line
            if(readyQueue != NULL || runningProc){     
                deadlockHandling(systemConfig);
                alreadyScheduled = true;
            }

            /* Process the next line of input from file */
            while(inputs == NULL || inputs[1] <= currentTime && inputs[1] != 9999){
                if(inputs == NULL && fgets(line,sizeof(line),file)!= NULL){
                    inputs = parseInput(line);
                    
                    //inputs[1] will hold the arrival time
                    //wait for arrival to process the external event
                    if(!(inputs[1] > currentTime && inputs[1] != 9999)){
                        processInputEvent(inputs, systemConfig);
                        inputs = NULL;

                        if(fgets(line,sizeof(line),file)!= NULL)
                            inputs = parseInput(line);
                    }
                } else if (inputs && inputs[1] <= currentTime){
                    processInputEvent(inputs, systemConfig);
                    inputs = NULL;

                    if(fgets(line,sizeof(line),file)!= NULL)
                        inputs = parseInput(line);
                }
                else {
                    //If there is nothing left to do, exit
                    if(readyQueue == NULL && runningProc == NULL && inputs == NULL){
                        currentTime = 10000;
                    }
                }
            }

            //Wait queue is checked when a process terminates and releases devices (inside round robin function)
            //or when a running process releases devices (happens inside processInputEvent function) 

            /* CHECK HOLD QUEUES */
            checkHoldQueues(systemConfig);

            //if the scheduler has not yet run for this time tick, execute proc on CPU
            if(!alreadyScheduled){
                deadlockHandling(systemConfig);
            }
            
        }
        //Print status at the end
        printAllQueues(systemConfig);
        printTAT();
        fclose(file);
        freeAll();
    }
}

/*
 *  Clear all finished queues, input objects, and system configuration
 *  
 *  @return void
 */
void freeAll(){
    node *temp = finishedQueue;
    while(temp != NULL){
        if(temp->proc){
            if(temp->proc->request)
                free(temp->proc->request);
            free(temp->proc);
            
            node *tempNext = temp->next;
            free(temp);
            temp = tempNext;
            
        }
    }

    if(inputs)
        free(inputs);

    free(systemConfig);
}


/*
 *  If there is enough memory available in the system for any jobs in 
 *  either of the hold queues, then move them to the ready queue.
 *  memory is allocated in the system for those jobs
 *  
 *  @param systemConfig: the current configuration of the system
 *  @return void
 */
void checkHoldQueues(config *systemConfig){
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
}

/*
 *  If there are enough devices available in the system for any 
 *  processes in the wait queue, then move them to the ready queue
 *  devices are allocated in the system for those processes
 *  
 *  @param systemConfig: the current configuration of the system
 *  @return void
 */
void checkWaitQueue(config *systemConfig){
    node *temp = waitingQueue;
    while(temp != NULL){
        //safety alg only checks ready queue and currently running proc
        //temporarily set the runningProc to the process that needs to be checked
        node *running = runningProc;
        runningProc = temp;

        //if the request is granted move proc to ready queue
        if(checkRequest(temp->proc, systemConfig)){
            node *tempNext = temp->next; //save temp->next before removing temp from list
            node *aNode = removeProcNode(&waitingQueue, temp->proc->pid);
            appendQueue(&readyQueue, aNode);
            temp = tempNext; //restore next node
        }
        else 
            temp = temp->next;
        
        //restore the runningProc
        runningProc = running;
    }
}


void deadlockHandling(config *systemConfig){
    if(runningProc && runningProc->proc->request){
        //if there is an incoming request, satisfied or not, the process moves off the CPU
        if(checkRequest(runningProc->proc, systemConfig)){
            //if the request is satisified, move proc to RQ
            //roundRobin(systemConfig);
            appendQueue(&readyQueue, runningProc);
            runningProc->next = NULL;
            runningProc=NULL;
        } else {
            //if the request is not satisfied, proc moves to WQ
            appendQueue(&waitingQueue, runningProc); 
            runningProc->next = NULL;
            runningProc = NULL;
        }
    } else
        roundRobin(systemConfig);
}


bool checkRequest(process *proc, config *systemConfig){
    //if request <= need
    if(proc->request->devices <= proc->devices - proc->allocated_devices){
        //if request <= available
        if(proc->request->devices <= systemConfig->available_devices){
            //pretend to allocate
            systemConfig->available_devices -= proc->request->devices; //available = available - request
            proc->allocated_devices += proc->request->devices; //allocated = allocated + request
            //need = need - request (dynamic through on demand calculations)

            //is the system safe
            if(isSafe(systemConfig)){
                //if it is safe the resources are allocated
                //process no longer has a request because it was granted
                free(proc->request);
                proc->request = NULL; 
                //Request is granted, system is safe
                return true;
            } else {
                //otherwise don't allocate resources
                systemConfig->available_devices += proc->request->devices;
                proc->allocated_devices -= proc->request->devices;
                //Request denied because it puts the system in an unsafe state
                return false;
            }
        }  
        //Request denied because there are not enough available devices              
        return false;
    } 
    //Request denied because request !< need
    return false;
}

/*
 *  create a node with a processs for a given job
 *  and put the node in the ready queue
 *  allocate memory for the process in the system
 *
 *  @param aJob: the job that needs to be turned into a process for the ready queue
 *  @param systemConfig: the current configuration of the system
 *  @return void
 */
void moveJobToReadyQueue(job *aJob, config *systemConfig){
    process *aProc = createProc(aJob); //create proc for this job
    node *aNode = malloc(sizeof(node)); //create new node
    aNode->proc = aProc; //set the proc of the node
    aNode->next = NULL;
    systemConfig->available_memory = systemConfig->available_memory - aProc->allocated_memory; //reduce system mem accordingly
    appendQueue(&readyQueue, aNode); //add node to ready queue

    free(aJob);
}

/*
 *  Round Robin scheduler
 *  Put a process on the CPU if there is no currently running proc
 *  Execute the process on the CPU for one time tick
 *  Remove the process from the CPU if the quantum expires or the execution completes
 *  and deallocate any memory or devices if a process completes execution
 *  Puts a new process on the CPU if one is taken off
 * 
 *  @param systemConfig: the current configuration of the system
 *  @return void
 */
void roundRobin(config *systemConfig){
    //take a process off the ready queue and move it to the CPU
    if(readyQueue && runningProc == NULL){
        runningProc = readyQueue;
        readyQueue = readyQueue->next;
        runningProc -> next = NULL;
    }
    //Run the process for 1 tick on the CPU
    if(runningProc){
        runningProc->proc->running_time += 1;
        
        if(runningProc->proc->running_time == runningProc->proc->burst){
            //process is done, take off CPU
            appendQueue(&finishedQueue, runningProc);
            //set time variables for the completed process
            runningProc->proc->completion_time = currentTime;
            runningProc->proc->turnaround_time = currentTime - runningProc->proc->arrival_time;
            runningProc->proc->wait_time = runningProc->proc->turnaround_time - runningProc->proc->running_time;
            //release memory and devices
            systemConfig->available_memory += runningProc->proc->allocated_memory;
            systemConfig->available_devices += runningProc->proc->allocated_devices;

            //if a process finishes and releases devices
            //check if any proc on the wait queue can be moved
            if(runningProc->proc->allocated_devices != 0 && waitingQueue){
                printf("CHECK %d %d", currentTime, systemConfig->available_devices);
                printAllQueues(systemConfig);
                checkWaitQueue(systemConfig);
            }

            runningProc->proc->allocated_memory = 0;
            runningProc->proc->allocated_devices = 0;

            runningProc -> next = NULL;
            runningProc = NULL;

            //put new proc on CPU
            // if(readyQueue){
            //     runningProc = readyQueue;
            //     readyQueue = readyQueue->next;
            //     runningProc->next = NULL;
            // }

        } else if(runningProc->proc->running_time % systemConfig->quantum == 0){
            //quantum is up, time to go back to the ready queue
            appendQueue(&readyQueue,runningProc);
            runningProc->next = NULL;
            runningProc = NULL;

            //put new proc on CPU
            // if(readyQueue){
            //     runningProc = readyQueue;
            //     readyQueue = readyQueue->next;
            //     runningProc->next = NULL;
            // }
        }
    }
}

/*
 *  parse input object and create necessary system object
 *  
 *  @param inputs: current set of input values
 *  @param systemConfig: the current configuration of the system
 *  @return void
 */
void processInputEvent(int *inputs, config *systemConfig){
    if(inputs[0] == 'C'){
        fprintf(stdout, "System Configuration\n");

        //Create configuration
        systemConfig->start_time = inputs[1];
        systemConfig->total_memory = inputs[2];
        systemConfig->available_memory = inputs[2];
        systemConfig->available_devices = inputs[3];
        systemConfig->total_devices = inputs[3];
        systemConfig->quantum = inputs[4];

        free(inputs);
        inputs = NULL;

    } else if (inputs[0] == 'A'){
        fprintf(stdout, "Job Arrival\n");
        
        //Create Job
        job *aJob = createJob(inputs);

        free(inputs);
        inputs = NULL;

        //Place job in the right queue
        if(aJob->memory > systemConfig->total_memory){
            //NOT ENOUGH TOTAL MEMORY
        } else if (aJob->devices > systemConfig->total_devices){
            //NOT ENOUGH TOTAL DEVICES
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

        if(runningProc)
            printf("pid %d request id %d time %d", runningProc->proc->pid, aRequest->id, currentTime);
        printAllQueues(systemConfig);
        
        //only handle requests if the request is for the currently running process
        if(runningProc && runningProc->proc->pid == aRequest->id){
            runningProc->proc->request = aRequest;
        }  else {
            //requests only arrive for the proc on the cpu
            //if the req is not for the currently running process, it is not needed - free it
            free(aRequest);
        }

        free(inputs); 
        inputs = NULL;

    } else if (inputs[0] == 'L'){
        fprintf(stdout, "Release of Devices\n");

        //Create release
        release *aRelease = createRelease(inputs);

        if(runningProc)
            printf("pid %d release id %d time %d", runningProc->proc->pid, aRelease->id, currentTime);
        
        //only handle a release if the release is for the currently running process
        if(runningProc && runningProc->proc->pid == aRelease->id){
            runningProc->proc->allocated_devices -= aRelease->devices; //release devices
            //interrupt quantum and move to ready queue
            appendQueue(&readyQueue, runningProc);
            runningProc->next = NULL;
            runningProc=NULL;
            //check if the release of devices allows any other procs to move to RQ
            checkWaitQueue(systemConfig);

            free(aRelease);
        } else {
            free(aRelease);
        }

        free(inputs);
        inputs = NULL;

    } else if (inputs[0] == 'D'){
        fprintf(stdout, "System Status\n");
        if(currentTime >= inputs[1]){
            printAllQueues(systemConfig);
        }

        free(inputs);
        inputs = NULL;
    }
}

//return array of integers for a given string input line
int *parseInput(char* input){
    char* token = strtok(input, " ");
    char* subtext = malloc(sizeof(char) * 10); 
    int* inputs = malloc(((int)INPUT_LENGTH) * sizeof(int));

    inputs[0] = input[0]; //input[0] = arrival time of event

    int counter = 1;
    //Split input string by space character
    while (token = strtok(0, " ")) {
        if (counter == 1){
            //first input is always just an integer
            inputs[counter] = atoi(token);
        } else {
            //extract integer
            memcpy(subtext,&token[2], strlen(token));
            inputs[counter] = atoi(subtext);
        }
        counter++;
    }

    inputs[counter] = -1; //value to note end of list defined as -1
    free(subtext);

    return inputs;
}


/*
 *  print the status of each queue
 *  
 *  @return void
 */
void printAllQueues(config *systemConfig){

    printf("At time %d: \n", currentTime);
    printf("Current Available Main Memory=%d\n", systemConfig->available_memory);
    printf("Current Devices=%d\n", systemConfig->available_devices);

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
    printProcQueue(waitingQueue);
    
}

// SAFETY ALGORITHM
int isSafe(config *systemConfig){
    int numProcs = 0;

    node *tempHead = readyQueue;
    node *temp = readyQueue;
    node *prev = temp;
    while(temp != NULL){
        //count procs in ready queue
        prev = temp;
        temp = temp->next;
        numProcs += 1;
    }
    //temporarily append the currently running process to the linked list of procs to iterate
    prev->next = runningProc;
    numProcs+=1;

    //set finish to false by default
    bool finish[numProcs];
    for(int i = 0; i < numProcs; i+=1){
        finish[i] = false;
    }

    int work = systemConfig->available_devices; //work = available
    bool found = true;

    //loop until all processes are finished or there are no processes with a need <= work 
    while(found){
        found = false;
        temp = tempHead;
        for(int i = 0; i < numProcs; i++){
            if(finish[i] == false && temp && temp->proc->devices - temp->proc->allocated_devices <= work){
                found = true;
                work = work + temp->proc->allocated_devices;
                finish[i] = true;
            }
            temp = temp->next;
        }
    }

    //If all processes are finished, safe state
    bool isSafe = true;
    for(int i = 0; i < numProcs; i+=1){
        if(finish[i] == false)
            isSafe = false;
    }

    //runningProc was temporarily added to the end of the list
    //now remove it so that runningProc is not part of the RQ
    prev->next = NULL;
    return isSafe;
}