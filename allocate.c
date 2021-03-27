#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


typedef struct SubProcessTracker {
    int K;
    int count;
} SubTracker;


typedef struct Process process;
struct Process {
    int timestamp;
    int pid;
    int finishingTime;
    int og_runningTime;
    int par_able;
    process *next;
    int isRunning;
    int subProcess;
    SubTracker *tracker;
    int start;

};

typedef struct CPU {
    process *head;
    process *tail;
    int CPUID;
    int remaining_ex_time;
    int active;
    int og_length;
    int q_length;
    int flag;
} CPU;

typedef struct Computer {
    CPU **CPU_List;
    int finish_count;
} Computer;



void addToCPU(CPU *Processor, int time, int pid, int timestamp, int parAble, void (*sortedInsert)(CPU* Processor, process *p), int sub_pIndex);
void addTrackerToCPU(CPU *Processor, int time, int pid, int timestamp, int parAble, int og_running, void (*sortedInsert)(CPU* Processor, process *p), int sub_pIndex, SubTracker *tracker);
void removeHead(CPU *Processor);
void changePrevTime(CPU *Processor, int PID, int TS);
void initCPUs(CPU** Processors, int pCount);
int get_most_free_CPU_index(CPU **Processors, int pCount);
void run_CPU(CPU* CPU0, int *MakeSpan, double *turnaround, double *max_overhead, double *overhead);
void sortedInsertBuffer(CPU *Processor, process *proc);
int round_up (double x);
int running_count(Computer *computer, int pCount);

int main(int argc, char *argv[]) {

    char str[100];
    int pCount;


    FILE *fp;

    for (int i = 1; i < argc; i++) {

        // Check for input and read file
        if (strcmp(argv[i], "-f")==0) {
            fp = fopen(argv[i+1], "r");
        }

        // Check for number of processes
        if (strcmp(argv[i], "-p")==0) {

            pCount = atoi(argv[i+1]);

        }


        // Task 4 command.
        if (strcmp(argv[i], "-c")==0) {

        } 
    }


    // Intialise All CPUs required.
    Computer *computer = (Computer*)malloc(sizeof(Computer*));
    computer->CPU_List = (CPU**)malloc(pCount*sizeof(CPU*));
    initCPUs(computer->CPU_List, pCount);
    int *all_processors = (int *)calloc(pCount, sizeof(int));


    // Check whether the file was able to be opened, throw error if not.
    if (fp == NULL) {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }


    // Read the text file line by line for processes.
    int j;
    int first_flag = 1;
    int buffer_done = 0;
    int most_free;


    // timestamp, pid, remaining time, parallesiable (1=yes , 0=no), cpuid.
    int inputFile[1024][5];
    int attrCounter = 0;
    int processCounter = 0;

    CPU **firstRunning = (CPU**)malloc(pCount*sizeof(CPU*));
    initCPUs(firstRunning, pCount);


    // Read the entire file into a 2D array.
    while ( fgets(str, 100, fp) != NULL ) {
        int time = 0;
        int pid = 0;
        int timestamp = 0;
        char *parallisable;
        char *token = strtok(str, " ");
        j = 0;

        while(token != NULL && j <= 3) {


            //Grabbing each variable from the line.
            if (j == 0) {
                timestamp = atoi(token);
                inputFile[processCounter][j] = timestamp;
            } else if (j == 1) {
                pid = atoi(token);
                inputFile[processCounter][j] = pid;
            } else if (j == 2) {
                time = atoi(token);
                inputFile[processCounter][j] = time;
            } else if (j == 3) {
                parallisable = token;

                if (strcmp(parallisable, "n\n")==0 || strcmp(parallisable, "n")==0) {

                    inputFile[processCounter][j] = 0;
                } else {

                    inputFile[processCounter][j] = 1;
                }
            }
            
            token = strtok(NULL, " ");
            j++;
        }
        processCounter++;



    }


    int second = 0;
    int pIndex = 0;
    int finishCount = 0;
    int *currentProc;
    int timestamp;
    int pid;
    int finishingTime;
    int paraAble;
    int totalProcAdded = 0;


    double turnaround = 0;
    double overhead = 0;
    double max_overhead = 0;
    int Makespan = 0;
    int proc_remaining = 0;
    // this is where all processes will run.
    while (finishCount < pCount) {
        // Check for most free cpu.
        most_free = get_most_free_CPU_index(computer->CPU_List, pCount);
        double K = 0;


        // Proc arrive
        if (pIndex < processCounter) {
            currentProc = inputFile[pIndex];
            timestamp = currentProc[0];
            pid = currentProc[1];
            finishingTime = currentProc[2];
            paraAble = currentProc[3];
            if (paraAble == 1) {

                // this will need to be split.
                if (pCount < finishingTime) {
                    K = pCount;
                } else {
                    K = finishingTime;
                }

            }
        }


        if (timestamp == second) {
            // Check for other proc from the same time.

            if (K != 0) {
                SubTracker *tracker = (SubTracker*)malloc(sizeof(SubTracker));
                tracker->K = K;
                tracker->count = 0;
                for (int subIndex = 0; subIndex < K; subIndex++) {
                    // Initialise K processes.
                    // Add each process to a CPU.
                    int K_running_time = round_up((finishingTime/K)) + 1;
                    int curr_free = get_most_free_CPU_index(computer->CPU_List, pCount);

                    addTrackerToCPU(computer->CPU_List[curr_free], K_running_time, pid, timestamp, paraAble, finishingTime ,sortedInsertBuffer, subIndex, tracker);
                    // Count the subprocess in process count as well, we can take the K sub count out later.
                    computer->CPU_List[curr_free]->q_length++;
                }
                proc_remaining ++;
                totalProcAdded++;

            } else {
                // sort and insert.
                CPU* buffer = (CPU *)malloc(sizeof(CPU));
                buffer->head = buffer->tail = NULL;
                buffer->remaining_ex_time = 0;
                buffer->CPUID = 0;
                buffer->active = 0;
                buffer->q_length = 0;
                addToCPU(buffer, finishingTime, pid, timestamp, paraAble, sortedInsertBuffer, -1);
                buffer->q_length++;

                for (int j = pIndex + 1; j < processCounter; j++) {
                    if (inputFile[j][0] == timestamp) {
                        // Found stuff from the same time. Have to sort them first by finishing time and then pid.
                        
                         addToCPU(buffer, inputFile[j][2], inputFile[j][1], inputFile[j][0], inputFile[j][3], sortedInsertBuffer, -1);
                         buffer->q_length++;
                    } else {
                        pIndex = j - 1;
                        break;
                    }
                }


                process *bufferPtr = buffer->head;
                while (bufferPtr) {
                    int curr_free = get_most_free_CPU_index(computer->CPU_List, pCount);
                    addToCPU(computer->CPU_List[curr_free], bufferPtr->finishingTime, bufferPtr->pid, bufferPtr->timestamp, bufferPtr->par_able, sortedInsertBuffer, -1);
                    computer->CPU_List[curr_free]->q_length++;
                    proc_remaining++;
                    totalProcAdded++;
                    bufferPtr = bufferPtr->next;
                }


            }




            // Move onto next Process.
            pIndex++;


        }


        // Take time away from each running proc
        for (int i = 0; i < pCount; i++) {
            if (computer->CPU_List[i]->head != NULL) {
                process *curr_Proc = computer->CPU_List[i]->head;
                if (curr_Proc->isRunning != 1) {
                    curr_Proc->isRunning = 1;
                    if (curr_Proc->subProcess != -1) {
                        printf("%d,RUNNING,pid=%d.%d,remaining_time=%d,cpu=%d\n", second, curr_Proc->pid, curr_Proc->subProcess,curr_Proc->finishingTime,i);
                    } else {
                        printf("%d,RUNNING,pid=%d,remaining_time=%d,cpu=%d\n", second, curr_Proc->pid, curr_Proc->finishingTime,i);

                    }
                }

                curr_Proc->finishingTime -= 1;
                computer->CPU_List[i]->remaining_ex_time -= 1;
            }
        }



        // Time continues.
        second++;



        // Check for any proc that has finishing time = 0.
        for (int i = 0; i < pCount; i++) {
            process *curr_running = computer->CPU_List[i]->head;
            if (curr_running != NULL && curr_running->finishingTime == 0) {
                // print finish and remove this from the list.
                if (curr_running->subProcess == -1) {
                    // proc_remaining--;
                    int test = proc_remaining - running_count(computer,pCount);
                    printf("%d,FINISHED,pid=%d,proc_remaining=%d\n", second, curr_running->pid, test);
                    proc_remaining--;
                    double currProc_turnaround = second - curr_running->timestamp;
                    overhead += currProc_turnaround/curr_running->og_runningTime;
                    if (currProc_turnaround/curr_running->og_runningTime > max_overhead) {
                        max_overhead = currProc_turnaround/curr_running->og_runningTime;
                    }

                    turnaround += second - curr_running->timestamp;
                } else {
                    
                    curr_running->tracker->count++;
                    if (curr_running->tracker->count == curr_running->tracker->K) {
                        proc_remaining--;
                        printf("%d,FINISHED,pid=%d,proc_remaining=%d\n", second, curr_running->pid, proc_remaining);
                        
                        double currProc_turnaround = second - curr_running->timestamp;
                        overhead += currProc_turnaround/curr_running->og_runningTime;
                        if ((currProc_turnaround/curr_running->og_runningTime) > max_overhead) {
                            max_overhead = currProc_turnaround/curr_running->og_runningTime;
                        }
                        turnaround += second - curr_running->timestamp;
                    }
                }
                curr_running->isRunning = 0;


                removeHead(computer->CPU_List[i]);
                computer->CPU_List[i]->q_length -= 1;
                if (computer->CPU_List[i]->q_length == 0) {
                    computer->CPU_List[i]->active = 0;
                    // finishCount++;
                    if (totalProcAdded == processCounter) {
                        finishCount++;
                    }
                }
            }
        }
        
        int endflag = 0;
        for (int i = 0; i < pCount; i++) {
            process *curr_running = computer->CPU_List[i]->head;
            if (curr_running != NULL) {
                endflag = 1;
            }
        }

        if (endflag == 0 && totalProcAdded == processCounter) {
            finishCount++;
            Makespan = second;
            break;
        }



    }
    printf("Turnaround time %d\n", round_up(turnaround/processCounter));
    float max_overhead_rounded = ((int)(max_overhead * 100 + .5) / 100.0);
    float overhead_rounded = ((int)(overhead/processCounter * 100 + .5) / 100.0);
    printf("Time overhead %.2f %.2f\n", max_overhead_rounded, overhead_rounded);

    printf("Makespan %d\n", second);



    fclose(fp);
    return(0);

}





void addToCPU(CPU *Processor, int time, int pid, int timestamp, int parAble, void (*sortedInsert)(CPU* Processor, process *p), int sub_pIndex) {

    // Make room for a new process.
    process *newProcess = (process*)malloc(sizeof(process));
    newProcess->next = NULL;
    assert(newProcess);

    // Adding Process information to the CPU.
    newProcess->finishingTime = time;
    newProcess->pid = pid;
    newProcess->timestamp = timestamp;
    newProcess->og_runningTime = time;
    newProcess->isRunning = 0;



    newProcess->par_able = parAble;

    if (sub_pIndex != -1) {
        newProcess->subProcess = sub_pIndex;
    } else {
        newProcess->subProcess = -1;
    }





    // Insert in ascending order of finishing time into the CPU.
    sortedInsert(Processor, newProcess);
    Processor->active = 1;


}

void addTrackerToCPU(CPU *Processor, int time, int pid, int timestamp, int parAble, int og_running, void (*sortedInsert)(CPU* Processor, process *p), int sub_pIndex, SubTracker *tracker) {

    // Make room for a new process.
    process *newProcess = (process*)malloc(sizeof(process));
    newProcess->next = NULL;
    assert(newProcess);

    // Adding Process information to the CPU.
    newProcess->finishingTime = time;
    newProcess->pid = pid;
    newProcess->timestamp = timestamp;
    newProcess->og_runningTime = og_running;
    newProcess->isRunning = 0;



    newProcess->par_able = parAble;

    if (sub_pIndex != -1) {
        newProcess->subProcess = sub_pIndex;
        newProcess->tracker = tracker;
    } else {
        newProcess->subProcess = -1;
    }





    // Insert in ascending order of finishing time into the CPU.
    sortedInsert(Processor, newProcess);
    Processor->active = 1;

}



void sortedInsertBuffer(CPU *Processor, process *proc) {
    process *currentNode;
    if (Processor->head == NULL || Processor->head->finishingTime > proc->finishingTime) {

        // Taking out the time that has been spent on a process when another process arrives and takes priority.
        if (Processor->head != NULL) {

            // Set the process back to inactive
            Processor->head->isRunning = 0;

            
        }
        proc->next = Processor->head;
        Processor->head = proc;
    } else if (Processor->head->timestamp == proc->timestamp && proc->pid < Processor->head->pid && Processor->head->finishingTime >= proc->finishingTime) {
        proc->next = Processor->head;
        Processor->head = proc;
    }
    else {
        currentNode = Processor->head;
        while (currentNode->next != NULL && currentNode->next->finishingTime <= proc->finishingTime) {
            currentNode = currentNode->next;
        }
        if (currentNode == Processor->tail) {
            Processor->tail = proc;
        }
        proc->next = currentNode->next;
        currentNode->next = proc;
    }
    Processor->remaining_ex_time += proc->finishingTime;
}


void removeHead(CPU *Processor) {
     if(Processor->head == NULL) {
         return;
     }

    process *tmp = Processor->head;
    Processor->head = Processor->head->next;
    Processor->remaining_ex_time -= tmp->finishingTime;
    free(tmp);
}


void changePrevTime(CPU *Processor, int PID, int TS) {
    process *ptr = Processor->head;
    while(ptr !=NULL) {
        if (ptr->pid == PID) {
            ptr->timestamp = TS;
        }
        ptr = ptr->next;
    }
}


void initCPUs(CPU** Processors, int pCount) {
    for (int i = 0; i < pCount; i++) {
        Processors[i] = (CPU *)malloc(sizeof(CPU));
        Processors[i]->head = Processors[i]->tail = NULL;
        Processors[i]->remaining_ex_time = 0;
        Processors[i]->CPUID = i;
        Processors[i]->active = 0;
        Processors[i]->q_length = 0;
        Processors[i]->flag = 0;
        assert(Processors[i]);
    }
}

int get_most_free_CPU_index(CPU **Processors, int pCount) {
    int smallest = -1;
    int id = -1;
    for (int i = 0; i < pCount; i++) {
        if (smallest == -1) {
            smallest = Processors[i]->remaining_ex_time;
            id = i;
        } else {
            if (smallest > Processors[i]->remaining_ex_time) {
                smallest = Processors[i]->remaining_ex_time;
                id = i;
            }
        }
    }
    assert(id != -1);
    return id;
}


int round_up (double x) {
    int x2 = (int)x;

    double offset = x - x2;
    if (offset == 0) {
        return (x2);
    }
    return ((int)(x+(1-offset)));
}

int running_count(Computer *computer, int pCount) {
    int count = 0;
    for (int i = 0; i < pCount; i++) {
        process *newP = computer->CPU_List[i]->head;
        if (newP && newP->finishingTime == 0) {
            count++;
        }
    }
    return count;
}
