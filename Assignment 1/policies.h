#ifndef POLICIES_H
#define POLICIES_H
struct process {
    char name[10];
    int ID;
    int duration;
    int arrival_time;
};

struct node {
   struct process *val;
   int key;
   struct node *next;
};
void FIFO(struct process processes[], int end_time,int size);
void SJF(struct process processes[], int end_time,int size);
void STCF(struct process processes[], int end_time,int size);
void RR(struct process processes[], int end_time,int size);
void enqueue (struct node ** headaddr,struct process* proc, int key, char s);
struct process* dequeue (struct node ** headaddr);
int print (struct node *head);
#endif