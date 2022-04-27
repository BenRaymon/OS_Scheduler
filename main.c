#include "test.h"

//Hold queue 1 - linked list
node *headHQ1;
node *tailHQ1;
//Hold queue 2 - linked list
node *headHQ2;
node *tailHQ2;

int main(int argc, char *argv[]) {

    //Hold queue 1 - SJF - linked list
    headHQ1 = NULL;
    tailHQ1 = headHQ1;
    //Hold queue 2 - FIFO - linked list
    headHQ2 = NULL;
    tailHQ2 = headHQ2;

    config *systemConfig = malloc(sizeof(config));

    FILE *file = fopen(argv[2], "r" );

    //READ INPUT FROM FILE
    if (file != NULL) {
        char line[1000];
        while(fgets(line,sizeof(line),file)!= NULL){
            //fprintf(stdout,"%s",line);
            
            if(line[0] == 'C'){
                fprintf(stdout, "System Configuration\n");
                
                int* inputs = parseInput(line);

                systemConfig->start_time = inputs[0];
                systemConfig->memory = inputs[1];
                systemConfig->devices = inputs[2];
                systemConfig->quantum = inputs[3];

                free(inputs);

                // printf("Start time %d\n", systemConfig->start_time);
                // printf("Memory %d\n", systemConfig->memory);
                // printf("Devices %d\n", systemConfig->devices);
                // printf("Quantum %d\n", systemConfig->quantum);

            } else if (line[0] == 'A'){
                fprintf(stdout, "Job Arrival\n");
                
                int* inputs = parseInput(line);
                job *aJob = malloc(sizeof(job));

                aJob->arrival_time = inputs[0];
                aJob->job_id = inputs[1];
                aJob->memory = inputs[2];
                aJob->devices = inputs[3];
                aJob->burst = inputs[4];
                aJob->priority = inputs[5];
                
                free(inputs);

                if(aJob->memory > systemConfig->memory){
                    //NOT ENOUGH MEMORY
                } else if (aJob->devices > systemConfig->devices){
                    //NOT ENOUGH DEVICES
                } else {
                    node *aNode = malloc(sizeof(node));
                    aNode -> job = aJob;

                    if(aJob->priority == 1){
                        insertHQ1(aNode);
                    } else if (aJob->priority == 2){
                        insertHQ2(aNode);
                    }
                }

                printList(headHQ1);
                printf("split\n");
                printList(headHQ2);

                //create a node and add job to a linked list

                // printf("Arrival time %d\n", aJob->arrival_time);
                // printf("Job ID %d\n", aJob->job_id);
                // printf("Memory %d\n", aJob->memory);
                // printf("Devices %d\n", aJob->devices);
                // printf("Burst %d\n", aJob->burst);
                // printf("Priority %d\n", aJob->priority);


            } else if (line[0] == 'Q'){
                fprintf(stdout, "Request for Devices\n");

                int* inputs = parseInput(line);
                request *aRequest = malloc(sizeof(request));

                aRequest->time = inputs[0];
                aRequest->job_id = inputs[1];
                aRequest->devices = inputs[2];

                free(inputs);
                                
                // printf("time %d\n", aRequest->time);
                // printf("job id %d\n", aRequest->job_id);
                // printf("devices %d\n", aRequest->devices);

            } else if (line[0] == 'L'){
                fprintf(stdout, "Release of Devices\n");

                int* inputs = parseInput(line);
                release *aRelease = malloc(sizeof(release));
                
                aRelease->time = inputs[0];
                aRelease->job_id = inputs[1];
                aRelease->devices = inputs[2];

                free(inputs);

                // printf("time %d\n", aRelease->time);
                // printf("job id %d\n", aRelease->job_id);
                // printf("devices %d\n", aRelease->devices);

            } else if (line[0] == 'D'){
                fprintf(stdout, "System Status\n");
            }
        }

        fclose(file);
    }
}

int *parseInput(char* input){
    char* token = strtok(input, " ");
    char* subtext = malloc(sizeof(char) * 10); 
    int* inputs = malloc(7 * sizeof(int));

    int counter = 0;
    //Split input string by space character
    while (token = strtok(0, " ")) {

        if (counter == 0){
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
    //If no nodes, make head and tail the new node
    if(headHQ1 == NULL){
        headHQ1 = newNode;
        tailHQ1 = headHQ1;
    } else {
        node* curr = headHQ1;
        
        while(curr != NULL){
            //If new job is shorter, put in front of current node
            if(newNode->job->burst < curr->job->burst){
                if(curr->prev == NULL){
                    headHQ1 = newNode;
                } else {
                    curr->prev->next = newNode;
                }
                newNode->prev = curr->prev;
                newNode->next = curr;
                curr->prev = newNode;
                return;
            }
            //If reach end of list, new job is the longest and it belongs at the end
            else if(curr->next == NULL){
                curr->next = newNode;
                newNode->prev = curr;
                tailHQ1 = newNode;
                return;
            }
            curr = curr->next;
        }
    }
}

//FIFO
void insertHQ2(node* newNode){
    //If no nodes, make head and tail the new node
    if(headHQ2 == NULL){
        headHQ2 = newNode;
        tailHQ2 = headHQ1;
    } else {
        node* curr = headHQ2;
        
        while(curr != NULL){
            //If new job is first, put in front of current node
            if(newNode->job->arrival_time < curr->job->arrival_time){
                if(curr->prev == NULL){
                    headHQ2 = newNode;
                } else {
                    curr->prev->next = newNode;
                }
                newNode->prev = curr->prev;
                newNode->next = curr;
                curr->prev = newNode;
                return;
            } 
            //If reach end of list, new job has arrived last and it belongs at the end
            else if(curr->next == NULL){
                curr->next = newNode;
                newNode->prev = curr;
                tailHQ2 = newNode;
                return;
            }
            curr = curr->next;
        }
    }
}

void printList(node *head){
    node *temp = head;

    while(temp != NULL){
        printf("PRINT LIST jobid = %d burst = %d at = %d\n", temp->job->job_id, temp->job->burst, temp->job->arrival_time);
        temp = temp->next;
    }
}