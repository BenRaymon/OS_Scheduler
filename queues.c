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