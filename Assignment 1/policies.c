#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "policies.h"

void FIFO(struct process processes[], int end_time,int size) {
    int time = 0;
    struct node *head = NULL;
    struct process *current = NULL;
    for (int i = 1 ; i < end_time + 1 ; i++) {
        printf("%d:",i);
        if (current != NULL) {
            printf("%s:",current->name);
            print(head);
            if (time == i) {
                current = dequeue(&head);
                time = i + current->duration;
            }
            for (int j = 0; j<size;j++) {
                if((processes+j)->arrival_time == i) {
                    enqueue(&head,(processes+j),(processes+j)->duration,'f');
                }
            }
        }
        else if (current == NULL) {
            printf("idle:");
            print(head);
            for (int j = 0; j<size;j++) {
                if((processes+j)->arrival_time == i) {
                    if (current != NULL) {
                    enqueue(&head,(processes + j),(processes+j)->duration,'f');
                    }
                    else {
                        current = processes+j;
                    }
                    time = i + current->duration;
                }
            }
        }
    }
}

void SJF(struct process processes[], int end_time,int size) {
    int time = 0;
    struct node *head = NULL;
    struct process *current = NULL;
    for (int i = 1 ; i < end_time + 1 ; i++) {
        printf("%d:",i);
        if (current != NULL) {
            printf("%s:",current->name);
            print(head);
            if (time == i) {
                current = dequeue(&head);
                time = i + current->duration; 
            }
            for (int j = 0; j<size;j++) {
                if((processes+j)->arrival_time == i) {
                    enqueue(&head,(processes+j),(processes+j)->duration,'s');
                    if (time-i >(processes+j)->duration) {
                        enqueue(&head,current,current->duration,'s');
                        current = dequeue(&head);
                        time = i + current->duration;
                    }
                }
            }
        }
        else if (current == NULL) {
            printf("idle:");
            print(head);
            for (int j = 0; j<size;j++) {
                if((processes+j)->arrival_time == i) {
                    if (current != NULL) {
                    enqueue(&head,(processes + j),(processes+j)->duration,'s');
                    }
                    else {
                        current = processes+j;
                    }
                    time = i + current->duration;
                }
            }
        }
    }
}

void STCF(struct process processes[], int end_time,int size) {
    int time = 0;
    struct node *head = NULL;
    struct process *current = NULL;
    for (int i = 1 ; i < end_time + 1 ; i++) {
        printf("%d:",i);
        if (current != NULL) {
            printf("%s:",current->name);
            print(head);
            if (time == i) {
                current = dequeue(&head);
                time = i + current->duration; 
            }
            for (int j = 0; j<size;j++) {
                if( (processes+j)->arrival_time == i) {
                    enqueue(&head,(processes+j),(processes+j)->duration,'s');
                    if (time-i >(processes+j)->duration) {
                        current->duration = time - i;
                        enqueue(&head,current,current->duration,'s');
                        current = dequeue(&head);
                        time = i + current->duration;
                    }
                }
            }
        }
        else if (current == NULL) {
            printf("idle:");
            print(head);
            for (int j = 0; j<size;j++) {
                if((processes+j)->arrival_time == i) {
                    if (current != NULL) {
                    enqueue(&head,(processes + j),(processes+j)->duration,'s');
                    }
                    else {
                        current = processes+j;
                    }
                    time = i + current->duration;
                }
            }
        }
    }
}

void RR(struct process processes[], int end_time,int size) {
    int time = 0;
    struct node *head = NULL;
    struct process *current = NULL;
    for (int i = 1 ; i < end_time + 1 ; i++) {
        printf("%d:",i);
        if (current != NULL) {
            printf("%s:",current->name);
            print(head);
            current->duration = current->duration - 1;
            if (head != NULL) {
                if (current->duration == 0) {
                    current = dequeue(&head);
                }
                else {
                    struct process *tmp = current;
                    current = dequeue(&head);
                    enqueue(&head,(tmp),(tmp)->duration,'f');
                }
            }
            for (int j = 0; j<size;j++) {
                if((processes+j)->arrival_time == i) {
                    enqueue(&head,(processes+j),(processes+j)->duration,'f');
                }
            }
        }
        else if (current == NULL) {
            printf("idle:");
            print(head);
            for (int j = 0; j<size;j++) {
                if((processes+j)->arrival_time == i) {
                    if (current != NULL) {
                    enqueue(&head,(processes + j),(processes+j)->duration,'f');
                    }
                    else {
                        current = processes+j;
                    }
                    time = i + current->duration;
                }
            }
        }
    }
}

void enqueue (struct node ** headaddr,struct process* proc, int key, char s){

    if (headaddr==NULL){
        fprintf(stderr, "NULL ptr passed\n"); exit(1);
    }
    struct node *n = (struct node*)malloc(sizeof(struct node));

    if (n==NULL){
       fprintf(stderr,"memory allocation failed\n");exit(1);
    }
    n->val = proc;
    n->next = NULL;
    n->key = key;

    if (*headaddr == NULL){ // empty list
        *headaddr = n;
    }
    else {
        if (s =='f') {
                struct node* tmp = * headaddr;
                while (tmp->next != NULL && tmp->next->key<= key) {
                    tmp = tmp -> next;
                }

                n->next = tmp->next;
                tmp-> next = n;
        }
        else {
            if (key<(*headaddr)->key)
            {
                n->next= *headaddr;
                *headaddr = n;
            }
            else
            {
                struct node* tmp = * headaddr;
                while (tmp->next != NULL && tmp->next->key<= key) {
                    tmp = tmp -> next;
                }

                n->next = tmp->next;
                tmp-> next = n;
            }
        }
    }
}

struct process* dequeue (struct node ** headaddr){


    if (headaddr==NULL){
        fprintf(stderr, "NULL ptr passed\n"); exit(1);
    }

    if (*headaddr == NULL){ // empty list
        exit(1);
    }

    else {
        struct process *proc;
        struct node *n = *headaddr;
        *headaddr = (*headaddr)->next;
        proc = n->val;
        free(n);
        return proc;
    }
    
}

int print (struct node *head)
{
    if (head == NULL)
    {
        printf("empty:\n");
        return 0;
    }
    else
    {
        while(head!=NULL)
        {
            fprintf(stdout,"%s(%d),",head->val->name,head->val->duration);
           
            head = head->next;
        }
        printf(":\n");
        
    }
    return 1;
    
}
