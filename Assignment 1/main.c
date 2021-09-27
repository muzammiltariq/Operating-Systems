#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "policies.h"

int main(int argc, char * argv[]){
    FILE * stream = fopen(argv[1], "r");
    if (stream == NULL) {
    	printf("%s%s\n","Error. Unable to open file ", argv[1]);
    	exit(1);
    }
    char s[512];
    int size = 0;
    int i = 0;
    struct process *processes = NULL;
    while(fscanf(stream,"%s",s)!=EOF) {
        if (s[0] != '#') {
            if (processes == NULL) {
                size = size + 1;
                processes = (struct process*) malloc(sizeof(struct process));
            }
            else if (processes != NULL) {
                size = size + 1;
                processes = realloc(processes,size * sizeof(struct process));
            }
            char *token = strtok(s,":");
            strcpy((processes+i)->name,token);
            token = strtok(NULL, ":");
            (processes+i)->ID = atoi(token);
            token = strtok(NULL, ":");
            (processes+i)->duration = atoi(token);
            token = strtok(NULL, ":");
            (processes+i)->arrival_time = atoi(token);
            i++;
        }
    }
    fclose(stream);
    for (int i = 0 ; i < size ; i++) {
        printf("%s(%d),",processes[i].name,processes[i].duration);
    }
    printf("\n");
    for(int i=0;i<size;i++){
        for(int j=i+1;j<size;j++){
            if(processes[i].arrival_time>processes[j].arrival_time){
                struct process temp=processes[i];
                processes[i] = processes[j];
                processes[j] = temp;
            }
        }
    }
    int end_time = processes[0].arrival_time;
    for (int i = 0 ; i < size ; i++) {
        printf("%s(%d),",processes[i].name,processes[i].duration);
        end_time = end_time + processes[i].duration;
    }
    printf("\n");
    if (strcmp(argv[2],"FIFO") == 0) {
        FIFO(processes,end_time,size);
    }
    else if (strcmp(argv[2],"SJF") == 0) {
        SJF(processes,end_time,size);
    }
    else if (strcmp(argv[2],"STCF") == 0) {
        STCF(processes,end_time,size);
    }
    else if (strcmp(argv[2],"RR") == 0) {
        RR(processes,end_time,size);
    }
    else {
        printf("%s","Error: unknown POLICY");
    }
    free(processes);
}
