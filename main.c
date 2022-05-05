#include "test.h"

int currentTime = 0;
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

            /* SCHEDULING PROCESSES */

            //BEFORE SCHEDULING
            printf("TIME: %d\n", currentTime);
            printf("BEFORE\n");
            printAll();

            // ROUND ROBIN SCHEDULING - SINGLE TICK ON CPU
            roundRobin(systemConfig);

            //AFTER SCHEDULING
            printf("AFTER\n");
            printAll();

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
        }

        fclose(file);
    }
}

//ROUND ROBIN SCHEDULING ALGORITHM
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
            runningProc -> next = NULL;
            runningProc = NULL;
        } else if(runningProc->proc->running_time % systemConfig->quantum == 0){
            //quantum is up, time to go back to the ready queue
            appendQueue(&readyQueue,runningProc);
            runningProc->next = NULL;
            runningProc = NULL;
        }
    }
}



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
            process *aProc = createProc(aJob);
            //after creating the process it has memory allocated to it, must subtract that memory from available
            systemConfig->available_memory = systemConfig->available_memory - aProc->allocated_memory;

            node *aNode = malloc(sizeof(node)); 
            aNode->proc = aProc; 
            
            appendQueue(&readyQueue, aNode);

            //free the job that is no longer needed
            free(aJob);

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



void printList(node *head){
    node *temp = head;

    if(head->job){
        while(temp != NULL){
            printf("\tjobid = %d burst = %d at = %d\n", temp->job->job_id, temp->job->burst, temp->job->arrival_time);
            temp = temp->next;
        }
    }

    if(head->proc){
        while(temp != NULL){
            printf("\tpid = %d \n", temp->proc->pid);
            temp = temp->next;
        }
    }
    
}



/* Print all the Queues */
void printAll(){
    if(holdQueueOne){
        printf("HQ1\n");
        printList(holdQueueOne);
    }
    if(holdQueueTwo){
        printf("HQ2\n");
        printList(holdQueueTwo);
    }
    if(readyQueue){
        printf("Ready Q\n");
        printList(readyQueue);
    }
    if(runningProc){
        printf("Running pid: %d Time on CPU: %d\n", runningProc->proc->pid, runningProc->proc->running_time);
    }
    if(finishedQueue){
        printf("Finished Q\n");
        printList(finishedQueue);
    }
}