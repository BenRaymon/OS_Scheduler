#include "queues.h"

float systemTAT;

/*
 *  appends a node to the end of a linked list
 *
 *  @param head: a reference to the head of the linked list
 *  @param aNode: the node to be inserted
 *  @return void
 */
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

/*
 *  inserts a node into a linked list sorted by burst time (SJF)
 *
 *  @param head: a reference to the head of the linked list
 *  @param newNode: the node to be inserted
 *  @return void
 */
void insertSJF(node **head, node *newNode){
    //If no nodes, make head the new node
    if(*head == NULL){
        *head = newNode;
    } else {
        node* curr = *head;
        //if new node has shorter burst than current head, place in front
        if(newNode->job->burst < curr->job->burst){
            *head = newNode;
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

/*
 *  inserts a node into a linked list sorted by arrival time (FIFO)
 *
 *  @param head: a reference to the head of the linked list
 *  @param newNode: the node to be inserted
 *  @return void
 */
void insertFIFO(node **head, node *newNode){
    //If no nodes, make head the new node
    if(*head == NULL){
        *head = newNode;
    } else {
        node* curr = *head;
        //If new node arrived before current head, place before head
        if(newNode->job->arrival_time < curr->job->arrival_time){
            *head = newNode;
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

/*
 *  finds the first node in linked list with a required memory 
 *  that is less than or equal to the available system memory
 *
 *  @param head: a reference to the head of the linked list
 *  @param systemConfig: the current configuration of the system
 *  @return node: the first node that meets the criteria
 */
node *findNode(node** head, config *systemConfig)
{
    // Store head node
    node *temp = *head;
 
    // Search for the first process that can be put on the CPU
    while (temp != NULL && temp->job->memory > systemConfig->available_memory) {
        temp = temp->next;
    }

    if(temp)
        return temp;
    else return NULL;
}

/*
 *  finds the node that has a process with a specified PID
 *
 *  @param head: a reference to the head of the linked list
 *  @param keyID: the PID to search for
 *  @return node: the node that contains the process with PID = keyID
 */
node *findProc(node** head, int keyID)
{
    node *temp = *head;
 
    while (temp != NULL && temp->proc->pid != keyID) {
        temp = temp->next;
    }

    if(temp)
        return temp;

    else return NULL;
}

/*
 *  removes a node that has a job with a specified
 *  job id and frees the allocated memory for the node
 *  
 *  @param head: a reference to the head of the linked list
 *  @param keyID: the job id to search for
 *  @return job: the the job of the node that was removed and freed
 */
job *deleteJobNode(node** head, int keyID)
{
    node *temp = *head;
    node *prev;

    if (temp != NULL && temp->job->job_id == keyID) {
        job *aJob = temp->job;
        *head = temp->next;
        free(temp);
        return aJob;
    }
 
    while (temp != NULL && temp->job->job_id != keyID) {
        prev = temp;
        temp = temp->next;
    }
 
    if (temp == NULL)
        return NULL;
 
    prev->next = temp->next;
    job *aJob = temp->job;
    free(temp); 
    return aJob;
}

/*
 *  removes a node from a linked list that has a 
 *  process with a specified pid. Does not free any memory 
 *  
 *  @param head: a reference to the head of the linked list
 *  @param keyID: the pid to search for
 *  @return node: the node that was removed from the list
 */
node *removeProcNode(node** head, int keyID)
{
    node *temp = *head;
    node *prev;

    if (temp != NULL && temp->proc->pid == keyID) {
        *head = temp->next;
        temp->next = NULL;
        return temp;
    }
 
    while (temp != NULL && temp->proc->pid != keyID) {
        prev = temp;
        temp = temp->next;
    }
 
    if (temp == NULL)
        return NULL;
 
    prev->next = temp->next;
    temp->next = NULL;
    return temp;
}

/*
 *  print all of the jobs of the nodes in the linked list
 *  
 *  @param head: the head of the list
 *  @return void
 */
void printJobQueue(node *head){
    node *temp = head;

    printf("-------------------------\n");
    printf("Job ID    Run Time\n");
    printf("=========================\n");
    while(temp != NULL){
        printf("  %d \t     %d\n", temp->job->job_id, temp->job->burst);
        temp = temp->next;
    }
    
}

/*
 *  print all of the processes of the nodes in the linked list
 *  prints job id, run time, and time accrued on CPU
 * 
 *  @param head: the head of the list
 *  @return void
 */
void printProcQueue(node *head){
    node *temp = head;

    printf("----------------------------------\n");
    printf("Job ID    Run Time    Time Accrued\n");
    printf("==================================\n");
    while(temp != NULL){
        printf("  %d \t      %d \t  %d\n", temp->proc->pid, temp->proc->burst, temp->proc->running_time);
        temp = temp->next;
    }
}

/*
 *  print all of the finished processes in the linked list
 *  prints job id, arrival time, finish time, and turnaround time
 * 
 *  @param head: the head of the list
 *  @return void
 */
void printFinishedJobs(node *head){
    node *temp = head;

    systemTAT = 0;
    float count = 0;

    printf("--------------------------------------------------------\n");
    printf("Job ID    Arrival Time    Finish Time    Turnaround Time\n");
    printf("========================================================\n");
    while(temp != NULL){
        systemTAT += temp->proc->turnaround_time;
        count += 1;
        printf("  %d \t       %d \t      %d \t       %d\n", temp->proc->pid, temp->proc->arrival_time, temp->proc->completion_time, temp->proc->turnaround_time);
        temp = temp->next;
    }

    systemTAT = systemTAT / count;
}

/*
 *  print total turnaround time for the system
 *  
 *  @return void
 */
void printTAT(){
    printf("\nSystem Turnaround Time: %.2f \n\n", systemTAT);
}

/*
 *  print all of the processes of the nodes in the linked list
 *  prints job id, time accrued on CPU, and time remaining
 *  
 *  @param head: the head of the list
 *  @return void
 */
void printRunningProc(node *head){
    node *temp = head;

    printf("-----------------------------------\n");
    printf("Job ID    Time Accrued    Time Left\n");
    printf("===================================\n");
    while(temp != NULL){
        printf("  %d \t      %d \t  %d\n", temp->proc->pid, temp->proc->running_time, temp->proc->burst - temp->proc->running_time);
        temp = temp->next;
    }
}