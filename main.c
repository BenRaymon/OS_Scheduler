#include "test.h"

node *holdQueueOne;
node *holdQueueTwo;

node *readyQueue;
node *runningProc;
node *waitingQueue;
node *finishedQueue;

int currentTime = 0;

int main(int argc, char *argv[]) {

    holdQueueOne = NULL;
    holdQueueTwo = NULL;
    readyQueue = NULL;
    waitingQueue = NULL;
    finishedQueue= NULL;
    runningProc = NULL;

    int* inputs = NULL;
    
    config *systemConfig = malloc(sizeof(config));

    FILE *file = fopen(argv[2], "r" );

    //READ INPUT FROM FILE
    if (file != NULL) {
        char line[1000];
        while(currentTime < 9999){
            
            currentTime += 1;

            if(inputs == NULL && fgets(line,sizeof(line),file)!= NULL){
                /* Process the next line of input from file */
                inputs = parseInput(line);
                if(inputs[1] > currentTime && inputs[1] != 9999){
                    //SKIP BECAUSE IT ISN'T TIME TO READ THIS INPUT YET
                } else {
                    test(inputs, systemConfig);
                    inputs = NULL;
                }
            } else if (inputs && inputs[1] <= currentTime){
                //DO THE WHOLE INPUTS THING
                test(inputs, systemConfig);
                inputs = NULL;
            }
            else {
                if(readyQueue == NULL && runningProc == NULL && inputs == NULL){
                    currentTime = 100000;
                }
            }

            /* SCHEDULING PROCESSES */

            //BEFORE SCHEDULING
            printf("TIME: %d\n", currentTime);
            
            printf("BEFORE\n");
            printAll();

            // ROUND ROBIN SCHEDULING
            //take a process off the ready queue and move it to the CPU
            if(readyQueue && runningProc == NULL){
                runningProc = readyQueue;
                readyQueue = readyQueue->next;
            }
            if(runningProc){
                runningProc->proc->running_time += 1;
                
                if(runningProc->proc->running_time == runningProc->proc->burst){
                    //process is done, take off CPU
                    //finishedQueue = appendQueue(runningProc, finishedQueue);
                    appendQueue(&finishedQueue, runningProc);
                    runningProc -> next = NULL;
                    runningProc = NULL;
                } else if(runningProc->proc->running_time % systemConfig->quantum == 0){
                    //quantum is up time to go back to the ready queue
                    //readyQueue = appendQueue(runningProc, readyQueue);
                    appendQueue(&readyQueue,runningProc);
                    runningProc->next = NULL;
                    runningProc = NULL;
                }
            }

            //AFTER SCHEDULING
            printf("AFTER\n");
            printAll();
            
        }

        fclose(file);
    }
}
void test(int *inputs, config *systemConfig){
    if(inputs[0] == 'C'){
        fprintf(stdout, "System Configuration\n");

        systemConfig->start_time = inputs[1];
        systemConfig->total_memory = inputs[2];
        systemConfig->available_memory = inputs[2];
        systemConfig->devices = inputs[3];
        systemConfig->quantum = inputs[4];

        free(inputs);
        inputs = NULL;

    } else if (inputs[0] == 'A'){
        fprintf(stdout, "Job Arrival\n");
        
        job *aJob = malloc(sizeof(job));

        aJob->arrival_time = inputs[1];
        aJob->job_id = inputs[2];
        aJob->memory = inputs[3];
        aJob->devices = inputs[4];
        aJob->burst = inputs[5];
        aJob->priority = inputs[6];
        
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
            
            //readyQueue = appendQueue(aNode, readyQueue);
            appendQueue(&readyQueue, aNode);

            //free the job that is no longer needed
            free(aJob);

        } else {
            //THERE IS NOT ENOUGH AVAILABLE MEMORY, PUT IN HOLD QUEUE
            node *aNode = malloc(sizeof(node));
            aNode -> job = aJob;

            if(aJob->priority == 1){
                insertHQ1(aNode);
            } else if (aJob->priority == 2){
                insertHQ2(aNode);
            }
        }

        
    } else if (inputs[0] == 'Q'){
        fprintf(stdout, "Request for Devices\n");

        request *aRequest = malloc(sizeof(request));

        aRequest->time = inputs[1];
        aRequest->job_id = inputs[2];
        aRequest->devices = inputs[3];

        free(inputs); 
        inputs = NULL;

    } else if (inputs[0] == 'L'){
        fprintf(stdout, "Release of Devices\n");

        release *aRelease = malloc(sizeof(release));
        
        aRelease->time = inputs[1];
        aRelease->job_id = inputs[2];
        aRelease->devices = inputs[3];

        free(inputs);
        inputs = NULL;

    } else if (inputs[0] == 'D'){
        fprintf(stdout, "System Status\n");
    }

    //return systemConfig;
}


process *createProc(job* aJob){
    process *aProc = malloc(sizeof(process));
    aProc->pid = aJob->job_id;
    aProc->arrival_time = aJob->arrival_time;
    aProc->burst = aJob->burst;
    aProc->allocated_memory = aJob->memory;
    aProc->devices = aJob->devices;
    aProc->waiting_time = 0;
    aProc->running_time = 0;
    return aProc;
}

void appendQueue(node **head, node *aNode){
    if(*head == NULL){
        *head = aNode;
    }else{
        node *curr = *head;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = aNode;
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


//SJF
void insertHQ1(node *newNode){
    //If no nodes, make head the new node
    if(holdQueueOne == NULL){
        holdQueueOne = newNode;
    } else {
        node* curr = holdQueueOne;
        //if new node has shorter burst than current head, place in front
        if(newNode->job->burst < curr->job->burst){
            holdQueueOne = newNode;
            newNode->next = curr;
            return;
        }
        
        while(curr->next != NULL){
            //If new job is shorter than next job, put in front of next node
            if(newNode->job->burst < curr->next->job->burst){
                newNode->next = curr->next;
                curr->next = newNode;
                return;
            }
            curr = curr->next;
        }
        //New job is currently the longest job so put at the end
        curr->next = newNode;
        return;
    }
}

//FIFO
void insertHQ2(node* newNode){
    //If no nodes, make head the new node
    if(holdQueueTwo == NULL){
        holdQueueTwo = newNode;
    } else {
        node* curr = holdQueueTwo;
        //If new node arrived before current head, place before head
        if(newNode->job->arrival_time < holdQueueTwo->job->arrival_time){
            holdQueueTwo = newNode;
            newNode->next = curr;
            return;
        } 

        while(curr->next != NULL){
            //If new job arrived before the next job, put in front of next node
            if(newNode->job->arrival_time < curr->next->job->arrival_time){
                newNode->next = curr->next;
                curr->next = newNode;
                return;
            }
            curr = curr->next;
        }
        //New job is currently the last to arrive so put at the end
        curr->next = newNode;
        return;
    }
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

void printAll(){
    /* Print all the Queues */
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