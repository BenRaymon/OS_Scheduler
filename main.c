#include "test.h"

int currentTime = 0;
bool alreadyScheduled = false;
node *holdQueueOne;
node *holdQueueTwo;
node *readyQueue;
node *waitingQueue;
node *finishedQueue;
node *runningProc;

int main(int argc, char *argv[]) {

    int* inputs = NULL;

    holdQueueOne = NULL;
    holdQueueTwo = NULL;

    readyQueue = NULL;
    waitingQueue = NULL;
    finishedQueue= NULL;
    
    runningProc = NULL;

    
    config *systemConfig = malloc(sizeof(config));

    FILE *file = fopen(argv[2], "r" );


    if (file != NULL) {
        char line[1000];
        
        while(currentTime < 9999){
            
            currentTime += 1;

            //process internal events before external events
            //if there is a proc in the ready queue or if there is a proc on the cpu
            //run this process before reading the next input line
            if(readyQueue != NULL || runningProc){     
                //if there is a runningProcess with a request, check if the request can be satisfied
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



            //CHECK FOR INCOMING RELEASES
            //RELEASE DEVICES

            //CHECK IF ANY PROC IN THE WAIT QUEUE CAN BE MOVED TO READY QUEUE
            
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
            //if the scheduler has not yet run for this time tick, execute proc on CPU
            if(!alreadyScheduled){
                printf("NO ITEM IN READY QUEUE, TIME: %d\n",currentTime);
                deadlockHandling(systemConfig);
            }
            
        }
        //Print status at the end
        printAllQueues(systemConfig);
        fclose(file);
    }
}

void deadlockHandling(config *systemConfig){
    if(runningProc && runningProc->proc->request){
        //if there is an incoming request, satisfied or not, the process moves off the CPU
        if(checkRequest(runningProc->proc, systemConfig)){
            //roundRobin(systemConfig);
            //if the request is satisified, move proc to RQ
            appendQueue(&readyQueue, runningProc);
            runningProc->next = NULL;
            runningProc=NULL;
        } else {
            //if the request is not satisfied, proc moves to WQ
            // node *aNode = malloc(sizeof(node));
            // aNode->proc=runningProc;
            // aNode->next = NULL;
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
                printf("REQUEST GRANTED pid %d # %d", proc->pid, proc->request->devices);
                
                //if it is safe the resources are allocated
                //process no longer has a request because it was granted
                free(proc->request);
                proc->request = NULL; 
                return true;
            } else {
                //otherwise don't allocate resources
                systemConfig->available_devices += proc->request->devices;
                proc->allocated_devices -= proc->request->devices;
                printf("REQUEST DENIED (unsafe) pid %d # %d", proc->pid, proc->request->devices);
                return false;
            }
        }                 
        printf("REQUEST DENIED (not enough avail devices) pid %d # %d", proc->pid, proc->request->devices);
        return false;
    } 
    printf("REQUEST DENIED (request !< need) pid %d # %d", proc->pid, proc->request->devices);
    return false;
}

//create a process for a given job
//create node to hold process
//add new node to ready queue
void moveJobToReadyQueue(job *aJob, config *systemConfig){
    process *aProc = createProc(aJob); //create proc for this job
    node *aNode = malloc(sizeof(node)); //create new node
    aNode->proc = aProc; //set the proc of the node
    aNode->next = NULL;
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


//create the necessary objects based on the input event
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

        //only handle requests if the request is for the currently running process
        if(runningProc && runningProc->proc->pid == aRequest->id){
            runningProc->proc->request = aRequest;
        } 
        // else {
        //     //find the process that the request is for
        //     node *aNode = findProc(&readyQueue, aRequest->id);
        //     if(aNode == NULL)
        //         aNode = findProc(&waitingQueue, aRequest->id);
        //     if(aNode != NULL)
        //         aNode->proc->request = aRequest;
        // }
        

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
        if(currentTime >= inputs[1]){
            printAllQueues(systemConfig);
        }
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


// print contents of given queue
void printQueue(node *head){
    node *temp = head;

    if(head->job){
        printf("\tJob IDs: ");
        while(temp != NULL){
            printf("%d ", temp->job->job_id);
            temp = temp->next;
        }
        printf("\n");
    }

    if(head->proc){
        printf("\tPIDs: ");
        while(temp != NULL){
            printf("%d ", temp->proc->pid);
            temp = temp->next;
        }
        printf("\n");
    }
    
}



/* Print all the Queues */
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
// LISTS WITH PROCESSES --> ready queue and wait queue
// finishReady = array of integers the length of the ready queue
// finishWork = array of integers the length of the wait queue
// 
// let work = systems available devices
// while found keep looping (keep checking if there is a proc that can be allocated devices)
// found = false
// loop thru # of elements in RQ
    //finishR[i] = false and then if need (devices - allocated_devices) <= work
        // work = work + node->proc->allocated_devices
        // finishR[i] = true
        // found = true
    // RQNode = -> next
// loop thru # of el in WQ
    //finishW[i] = false and then if need (devices - allocated_devices) <= work
        // work = work + node->proc->allocated_devices
        // finishW[i] = true
        // found = true
    // WQNode = -> next
//Once there are no more processes that are found, check if every proc finished
//loop thru the larger number of elements
// if i < HQ count
    // if finishedR[i] = false then isSafe = false
// if i < WQ count
    // if finishedW[i] = false then isSafe = false
//return isSafe

//EDIT TO ABOVE PSEUDOCODE --> DO NOT CHECK WAIT QUEUE, CHECK ALL PROC IN READY QUEUE + RUNNING PROC

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