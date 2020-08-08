/*
 * cpusched.c
 * This extra implements preemptive shortest job first method (psjf_scheduling).
 * To specify preemptive shortest job first, use "-a PSJF" command-line option.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdbool.h>

#define MAX_LINE_LENGTH 100

#define FCFS 0
#define PS   1
#define RR 2
#define STRIDE 3

#define PSJF 4

#define PRIORITY_LEVELS 4


/*
 * Stores raw event data from the input,
 * and has spots for per-task statistics.
 * You may want to modify this if you wish
 * to store other per-task statistics in
 * the same spot.
 */

typedef struct Task_t {
    int   arrival_time;
    float length;
    int   priority;

    float finish_time;
    int   dispatches;
    float cpu_cycles;
} task_t; 

typedef struct node_t {
    int index;            //Index of the struct (like in an array)
    struct node_t *next;  //Next node in the list
    struct node_t *prev;  //Previous node in the list  
    int task;	
} node_t;

//Some simple tasks 
void insert(node_t* head, int at, int* error_code, int tasknum);
void delete(node_t* head, int at, int* error_code);
void cleanup(node_t* head, node_t* tail);


/*
 * Some function prototypes.
 */

void read_task_data(void);
void init_simulation_data(int);
void first_come_first_serve(void);
void stride_scheduling(int);
void priority_scheduling(void);
void rr_scheduling(int);
void run_simulation(int, int);
void compute_and_print_stats(void);

void psjf_scheduling(void);


/*
 * Some global vars.
 */
int     num_tasks = 0;
task_t *tasks = NULL;

//Add a node
void insert(node_t* head, int at, int* error_code, int tasknum)
{
    //printf("\nAdding node at index %d, task %d \n", at, tasknum);
    node_t* curr = head;
    bool update_index = false;


    while(curr != NULL) {
        if(curr->index == at) {
            node_t* node = (node_t*)(malloc(sizeof(node_t)));
            if(node == NULL) {
                printf("malloc failed");
                break;
            }
            
            node->index = at;
            node->prev = curr->prev;
            node->next = curr;
			
			node->task = tasknum;

            curr->prev->next = node;            
            curr->prev = node;
            update_index = true; 
        }
       
        if(update_index) 
            curr->index++;

        curr = curr->next;

    } 
    
    if (!update_index) (*error_code)++;
}

//Delete a node
void delete(node_t* head, int at, int* error_code)
{
    //printf("\nDelete node at index %d\n", at);
    node_t* curr = head;
    bool update_index = false;

    while(curr != NULL) {
        if(curr->index == at) {
            node_t* tmp = curr;
            curr->prev->next = curr->next;
            curr->next->prev = curr->prev; 
            curr = curr->next;
            free(tmp);
            update_index = true; 
        }
       
        if(update_index) 
            curr->index--;

        curr = curr->next;

    } 
    
    if (!update_index) (*error_code)++;
}
//I only need to cleanup the the structs added in insert
//because only they used malloc.  If you try to free
//head/tail it will cause an error
void cleanup(node_t* head, node_t* tail)
{
    node_t* tmp;
    node_t* del = head->next;
    while(del != NULL && del != tail)
    {
        tmp = del;
        del = del->next;
        free(tmp);
    }
}

void read_task_data()
{
    int max_tasks = 2;
    int  in_task_num, in_task_arrival, in_task_priority;
    float in_task_length;
    

    assert( tasks == NULL );

    tasks = (task_t *)malloc(sizeof(task_t) * max_tasks);
    if (tasks == NULL) {
        fprintf(stderr, "error: malloc failure in read_task_data()\n");
        exit(1);
    }
   
    num_tasks = 0;

    /* Given the format of the input is strictly formatted,
     * we can used fscanf .
     */
    while (!feof(stdin)) {
        fscanf(stdin, "%d %d %f %d\n", &in_task_num,
            &in_task_arrival, &in_task_length, &in_task_priority);
        assert(num_tasks == in_task_num);
        tasks[num_tasks].arrival_time = in_task_arrival;
        tasks[num_tasks].length       = in_task_length;
        tasks[num_tasks].priority     = in_task_priority;

        num_tasks++;
        if (num_tasks >= max_tasks) {
            max_tasks *= 2;
            tasks = (task_t *)realloc(tasks, sizeof(task_t) * max_tasks);
            if (tasks == NULL) {
                fprintf(stderr, "error: malloc failure in read_task_data()\n");
                exit(1);
            } 
        }
    }
}


void init_simulation_data(int algorithm)
{
    int i;

    for (i = 0; i < num_tasks; i++) {
        tasks[i].finish_time = 0.0;
        tasks[i].dispatches = 0;
        tasks[i].cpu_cycles = 0.0;
    }
}


void first_come_first_serve() 
{
    int current_task = 0;
    int current_tick = 0;

    for (;;) {
        current_tick++;

        if (current_task >= num_tasks) {
            break;
        }

        /*
         * Is there even a job here???
         */
        if (tasks[current_task].arrival_time > current_tick-1) {
            continue;
        }

        tasks[current_task].cpu_cycles += 1.0;
        
        if (tasks[current_task].cpu_cycles >= tasks[current_task].length) {
            float quantum_fragment = tasks[current_task].cpu_cycles -
                tasks[current_task].length;
            tasks[current_task].cpu_cycles = tasks[current_task].length;
            tasks[current_task].finish_time = current_tick - quantum_fragment;
            tasks[current_task].dispatches = 1;
            current_task++;
            if (current_task > num_tasks) {
                break;
            }
            tasks[current_task].cpu_cycles += quantum_fragment;
        }
    }
}


void stride_scheduling(int quantum)
{
    printf("STRIDE SCHEDULING appears here\n");
    exit(1);
}

void psjf_scheduling(){
    //printf("PRIORITY SCHEDULING appears here\n");
	int error = 0;
    node_t head = {.index = -1, .prev = NULL, .next = NULL, .task = -1};
    node_t tail = {.index = 0, .prev = &head, .next = NULL, .task = -1};
    head.next = &tail; //Dereference to get the tail pointer
	
	int arrived_tasks = 0; //next task to check for arrival
    int completed_tasks = 0;
    int current_tick = 0; //keeps track of time
	float quantum_fragment = 0;
	
	//printf("Time:%d\n",current_tick);
    for (;;) {
		
		// Any more tasks to process?
        if (completed_tasks >= num_tasks) {
            break;
        }

		
		//Any more tasks that can arrive?
		if(arrived_tasks < num_tasks){
			int i;
			for(i = 0; i<num_tasks; i++){
				// Did a task arrive at current time?
				if(tasks[i].arrival_time == current_tick){
					//yes
					int index = 0; //where to insert in linked list

					if(head.next == &tail){

						//linked list empty
						insert(&head, index, &error, i);
					}else{
						//find proper position according to shortest job
						int len = tasks[i].length;
						node_t* curr = head.next;  //preemptive
						while(curr != NULL) {
							if(curr == &tail){
								//reached the end
								index = curr->index;
								break;
							}else{
								int currtask = curr->task;
								int currlen = tasks[currtask].length;
								if(currlen > len) {
									//found position
									index = curr->index;
			
									//if preempt first task in list
									if(index == 0){
										tasks[currtask].dispatches += 1;
									}
									break;
								}
							}	
							curr = curr->next;
						}
						insert(&head, index, &error, i);
					}	
					arrived_tasks += 1;
				}
			}
		}
		current_tick++;
		//printf("Time:%d\n",current_tick);
		if(head.next != &tail){
			//process task
			int current_task = head.next->task;
			tasks[current_task].cpu_cycles += quantum_fragment; //add cpu cycles that may have been to quantum
			quantum_fragment = 0;
			tasks[current_task].cpu_cycles += 1.0;
        
			//completed task?
			if (tasks[current_task].cpu_cycles >= tasks[current_task].length) {
				quantum_fragment = tasks[current_task].cpu_cycles -
					tasks[current_task].length;
				tasks[current_task].cpu_cycles = tasks[current_task].length;
				tasks[current_task].finish_time = current_tick - quantum_fragment;
				tasks[current_task].dispatches += 1;
				//remove task from linked list
				delete(&head, 0, &error);
				completed_tasks++;
				
				if (completed_tasks >= num_tasks) {
					break;
				}
				
				//another task in linked list?
				if(head.next != &tail){
					//get next task
					current_task = head.next->task;
					tasks[current_task].cpu_cycles += quantum_fragment; //add cpu cycles that may have been to quantum
					quantum_fragment = 0;
				}
			}
		}
    }
	
	//Always clean up your memory allocations!
    cleanup(&head, &tail);
}

void priority_scheduling()
{
    //printf("PRIORITY SCHEDULING appears here\n");
	int error = 0;
    node_t head = {.index = -1, .prev = NULL, .next = NULL, .task = -1};
    node_t tail = {.index = 0, .prev = &head, .next = NULL, .task = -1};
    head.next = &tail; //Dereference to get the tail pointer
	
	int arrived_tasks = 0; //next task to check for arrival
    int completed_tasks = 0;
    int current_tick = 0; //keeps track of time
	float quantum_fragment = 0;
	
	//printf("Time:%d\n",current_tick);
    for (;;) {
		
		// Any more tasks to process?
        if (completed_tasks >= num_tasks) {
            break;
        }

		
		//Any more tasks that can arrive?
		if(arrived_tasks < num_tasks){
			int i;
			for(i = 0; i<num_tasks; i++){
				// Did a task arrive at current time?
				if(tasks[i].arrival_time == current_tick){
					//yes
					int index = 0; //where to insert in linked list
					
					if(head.next == &tail){
						//linked list empty
						insert(&head, index, &error, i);
					}else{
						//find proper position according to priority
						int priority = tasks[i].priority;
						node_t* curr = head.next->next;

						while(curr != NULL) {
							int currtask = curr->task;
							int currpriority = tasks[currtask].priority;
						
							if(currpriority > priority) {
								//found position
								index = curr->index;
								break;
							}else if(curr == &tail){
								//reached the end
								index = curr->index;
								break;
							}
							curr = curr->next;
						}			
						insert(&head, index, &error, i);
					}	
					arrived_tasks += 1;
				}
			}
		}
		current_tick++;
		//printf("Time:%d\n",current_tick);
		if(head.next != &tail){
			//process task
			int current_task = head.next->task;
			tasks[current_task].cpu_cycles += quantum_fragment; //add cpu cycles that may have been to quantum
			quantum_fragment = 0;
			tasks[current_task].cpu_cycles += 1.0;
        
		
			//completed task?
			if (tasks[current_task].cpu_cycles >= tasks[current_task].length) {
				quantum_fragment = tasks[current_task].cpu_cycles -
					tasks[current_task].length;
				tasks[current_task].cpu_cycles = tasks[current_task].length;
				tasks[current_task].finish_time = current_tick - quantum_fragment;
				tasks[current_task].dispatches = 1;
				
				//remove task from linked list
				delete(&head, 0, &error);
				completed_tasks++;
				
				if (completed_tasks >= num_tasks) {
					break;
				}
				
				//another task in linked list?
				if(head.next != &tail){
					//get next task
					current_task = head.next->task;
					tasks[current_task].cpu_cycles += quantum_fragment; //add cpu cycles that may have been to quantum
					quantum_fragment = 0;
				}
			}
		}
    }
	
	//Always clean up your memory allocations!
    cleanup(&head, &tail);
}



void rr_scheduling(int quantum)
{
    //printf("RR SCHEDULING appears here\n");
    int error = 0;
    node_t head = {.index = -1, .prev = NULL, .next = NULL, .task = -1};
    node_t tail = {.index = 0, .prev = &head, .next = NULL, .task = -1};
    head.next = &tail; //Dereference to get the tail pointer
	
	int arrived_tasks = 0; //next task to check for arrival
    int completed_tasks = 0;
    int current_tick = 0; //keeps track of time
	//printf("Time:%d\n",current_tick);
	
	int after_insert = 0;
	int after_insert_task = 0;
	
	int quantum_left = quantum;
	
    for (;;) {
		// Any more tasks to process?
        if (completed_tasks >= num_tasks) {
            break;
        }

		//Any more tasks that can arrive?
		if(arrived_tasks < num_tasks){
			int i;
			for(i=0;i<num_tasks;i++){
				// Did a task arrive at current time?
				if(tasks[i].arrival_time == current_tick){
					//yes
					int index = 0; //where to insert in linked list
				
					if(head.next == &tail){
						//linked list empty
						insert(&head, index, &error, i);
						quantum_left = quantum; //reset, incase there wasn't a task in a while, and quantum_left is negative
					}else{
						//put at end of list
						index = tail.index;			
						insert(&head, index, &error, i);
					}	
					arrived_tasks += 1;
				}
			}
		}	
		if(after_insert == 1){
			if(tail.index > 1){
				//printf("task: %d, cpu: %f /n",after_insert_task,tasks[after_insert_task].cpu_cycles);
				delete(&head, 0, &error);
				insert(&head, tail.index, &error, after_insert_task);
			}
			after_insert = 0;
		}
		current_tick++;
		if(head.next != &tail){
			quantum_left--;
		}
		//printf("Time:%d , ql:%d\n",current_tick,quantum_left);
		
		//if there are tasks in list and current_tick is a multiple of quantum
		if(head.next != &tail && quantum_left <= 0 ){
			//process task
			int current_task = head.next->task;
			tasks[current_task].cpu_cycles += quantum; //add quantum cpu cycles. note: keeps track of quantum_fragments
														//from quantum_left
			tasks[current_task].dispatches++;

			quantum_left = quantum; //reset
			
			//completed task?
			if (tasks[current_task].cpu_cycles >= tasks[current_task].length) {
				float quantum_fragment = tasks[current_task].cpu_cycles -
					tasks[current_task].length;
				tasks[current_task].cpu_cycles = tasks[current_task].length;
				tasks[current_task].finish_time = current_tick - quantum_fragment;
				
				quantum_left-= quantum_fragment;
				
				//remove task from linked list
				//printf("finished task %d\n", current_task);
				//printf("task: %d, cpu: %f, final: %f /n",current_task,tasks[current_task].cpu_cycles,tasks[current_task].finish_time);
				delete(&head, 0, &error);
				completed_tasks++;
				
				if (completed_tasks >= num_tasks) {
					break;
				}
			}else{
				//did not complete task. put task to end of list	
				int i;
				for(i=0;i<num_tasks;i++){
					//checks if a task arrives at the same time a task ends/switches
					if(tasks[i].arrival_time == current_tick){
						//must insert new task first and then move current task to the end of list
						after_insert_task = current_task;
						after_insert = 1;
						break;
					}
				}
				
				if(after_insert != 1 && tail.index > 1){
					//put at end if isn't the only task in the list
					//printf("task: %d, cpu: %f /n",current_task,tasks[current_task].cpu_cycles);
					delete(&head, 0, &error);
					insert(&head, tail.index, &error, current_task);
				}
			}
		}
    }
	
	//Always clean up your memory allocations!
    cleanup(&head, &tail);
}


void run_simulation(int algorithm, int quantum)
{
    switch(algorithm) {
        case STRIDE:
            stride_scheduling(quantum);
            break;
        case PS:
            priority_scheduling();
            break;
        case RR:
            rr_scheduling(quantum);
            break;
		case PSJF:
			psjf_scheduling();
			break;
        case FCFS:
        default:
            first_come_first_serve();
            break;
    }
}


void compute_and_print_stats()
{
    int tasks_at_level[PRIORITY_LEVELS] = {0,};
    float response_at_level[PRIORITY_LEVELS] = {0.0, };
    int scheduling_events = 0;
    int i;

    for (i = 0; i < num_tasks; i++) {
        tasks_at_level[tasks[i].priority]++;
        response_at_level[tasks[i].priority] += 
            tasks[i].finish_time - (tasks[i].arrival_time * 1.0);
        scheduling_events += tasks[i].dispatches;

        printf("Task %2d: cpu (%5.1f), response (%6.1f), wait (%6.1f), # dispatch (%5d)\n",
            i, tasks[i].length,
            tasks[i].finish_time - tasks[i].arrival_time,
            tasks[i].finish_time - tasks[i].arrival_time - tasks[i].cpu_cycles,
            tasks[i].dispatches);
            
    }

    printf("\n");

    if (num_tasks > 0) {
        for (i = 0; i < PRIORITY_LEVELS; i++) {
            if (tasks_at_level[i] == 0) {
                response_at_level[i] = 0.0;
            } else {
                response_at_level[i] /= tasks_at_level[i];
            }
            printf("Priority level %d: average response time (%4.1f)\n",
                i, response_at_level[i]);
        }
    }

    printf ("Total number of scheduling events: %d\n", scheduling_events);
}


int main(int argc, char *argv[])
{
    int i = 0;
    int algorithm = FCFS;
    int quantum = 1;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-q") == 0) {
            i++;
            quantum = atoi(argv[i]);
        } else if (strcmp(argv[i], "-a") == 0) {
            i++;
            if (strcmp(argv[i], "FCFS") == 0) {
                algorithm = FCFS;
            } else if (strcmp(argv[i], "PS") == 0) {
                algorithm = PS;
            } else if (strcmp(argv[i], "RR") == 0) {
                algorithm = RR;
            } else if (strcmp(argv[i], "STRIDE") == 0) {
                algorithm = STRIDE;
            } else if (strcmp(argv[i], "PSJF") == 0){
				algorithm = PSJF;
			}
        }
    }
         
    read_task_data();

    if (num_tasks == 0) {
        fprintf(stderr,"%s: no tasks for the simulation\n", argv[0]);
        exit(1);
    }

    init_simulation_data(algorithm);
    run_simulation(algorithm, quantum);
    compute_and_print_stats();

    exit(0);
}
