#include "queues.h"

//Append to any queue
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

//SJF
//insert node to a linked list sorted by shortest job first 
//insert function for HQ1
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

//FIFO
//insert node to a linked list sorted by arrival time
//insert for HQ2
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

//finds the first node in linked list with a required memory that is less than or equal to the available system memory
//returns the first node that can be moved from HQ to RQ
node *findNode(node** head_ref, config *systemConfig)
{
    // Store head node
    node *temp = *head_ref;
 
    // Search for the first process that can be put on the CPU
    while (temp != NULL && temp->job->memory > systemConfig->available_memory) {
        temp = temp->next;
    }

    return temp;
}

//removes a node that has a job with specific ID from linked list
// returns the job of the node that is removed
job *deleteJobNode(node** head_ref, int keyID)
{
    node *temp = *head_ref;
    node *prev;

    if (temp != NULL && temp->job->job_id == keyID) {
        job *aJob = temp->job;
        *head_ref = temp->next;
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

void printFinishedJobs(node *head){
    node *temp = head;

    printf("--------------------------------------------------------\n");
    printf("Job ID    Arrival Time    Finish Time    Turnaround Time\n");
    printf("========================================================\n");
    while(temp != NULL){
        printf("  %d \t       %d \t      %d \t       %d\n", temp->proc->pid, temp->proc->arrival_time, temp->proc->completion_time, temp->proc->turnaround_time);
        temp = temp->next;
    }
}

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