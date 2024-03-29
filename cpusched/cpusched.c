/*
 ============================================================================
Name	     : proj1.c
 Author      : Darren Jennings
 Version     :
 Copyright   : Copyright 2014
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "dbg.h"
#include "list.c"

typedef enum {
  FCFS,
  SRTF
}algorithm;

typedef struct process {
  unsigned int pid; // positive integer process id
  unsigned int arrival_time; //in milliseconds from 0
  unsigned int burst_time; //in milliseconds
  unsigned int start_time; //in milliseconds
  unsigned int finish_time; //in milliseconds
  unsigned int waiting_time; //in milliseconds
  unsigned int burst_remaining; // in milliseconds
} process;

process * create_process(int, int, int, int, int, int, int);
void free_process(process *);
process * getMaxBurstTimePS(List *);
process * create_process(int id, int arrival_time, int burst, int start_time, int finish_time, int waiting_time, int burst_remaining)
{
  process* ps_ptr = (process *) malloc(sizeof(process));
  ps_ptr->pid = id;
  ps_ptr->arrival_time = arrival_time;
  ps_ptr->burst_time = burst;
  ps_ptr->start_time = start_time;
  ps_ptr->finish_time = finish_time;
  ps_ptr->waiting_time = waiting_time;
  ps_ptr->burst_remaining = burst_remaining;
  return ps_ptr;
}

void schedulePS(List *pslist, algorithm alg){
	////////////////////////////////////////////////////////////
	//
	// First Come First Serve
	//
	////////////////////////////////////////////////////////////
	if(alg == FCFS){
		debug("****FCFS****");
		int time_elapsed = 0;
		LIST_FOREACH(pslist, first, next, cur){
				process *ps = cur->value;
				//debug("Process %d, elapsed time: %d", ps->pid,time_elapsed);
				ps->start_time = time_elapsed;
				ps->finish_time = ps->start_time + ps->burst_time;
				ps->waiting_time = ps->start_time - ps->arrival_time;
				time_elapsed += ps->burst_time;
			}
		}

	////////////////////////////////////////////////////////////
	//
	// Shortest Remaining Time First
	//
	////////////////////////////////////////////////////////////
	if(alg == SRTF){
		debug("****SRTF****");
		int time_elapsed = 0;
		int processesCompleted = 0;
		process *max = getMaxBurstTimePS(pslist);
		//List *psStack = List_create();
			for(time_elapsed=0;processesCompleted!=(pslist->count);time_elapsed++)
			{
				//keep filling the shortestps with the maxps for reference
				process *shortestps = max;
				LIST_FOREACH(pslist, first, next, cur)
				{
					process *ps = cur->value;
					if(ps->arrival_time<=time_elapsed && ps->burst_remaining < shortestps->burst_remaining && ps->burst_remaining>0)
					{
						shortestps = ps;
						//debug("Shortest is pid: %d",ps->burst_time);
					}
				}
				shortestps->burst_remaining--;
				if(shortestps->burst_remaining==0)
				{
					processesCompleted++;
					shortestps->finish_time = time_elapsed+1;
					shortestps->waiting_time = ((int)(time_elapsed+1)-(int)shortestps->burst_time)-(int)shortestps->arrival_time;
					//List_push(pslist, shortestps);
				}
			}
		//pslist = psStack;
	}
}

void sortPSListByFinishTime(List *list)
{
	int sorted;
	do {
	sorted = 1;
        LIST_FOREACH(list, first, next, cur) {
        	if(cur->next) {
        	process *ps = cur->value;
        	process *psn = cur->next->value;
                if(ps->finish_time > psn->finish_time){
                    ListNode_swap(cur, cur->next);
                    sorted = 0;
                }
            }
        }
    } while(!sorted);
}

process *getMaxBurstTimePS(List *list){
	process *psmaxt;// = create_process(0,0,0,0,0,0,0);
	LIST_FOREACH(list, first, next, cur){
				process *ps = cur->value;
				if(psmaxt==NULL){
					psmaxt = ps;
				}else{
					if(ps->burst_time > psmaxt->burst_time){
						psmaxt = ps;
					}
				}
			}
	//debug("Done sorting...");
	//free_process(psmaxt);
	return psmaxt;
}

void free_process(process* process)
{
  free(process);
}

int main(int argc, char *argv[]) {

	//////////////////
		//PRINT ARGS
		//////////////////
		/*int i=0;
		while(i < argc) {
			printf("\narg[%d]: %s\n", i, argv[i]);
			i++;
		}*/

	////////////////////////////////////
	//INITIALIZE FILES FROM ARGUMENTS
	////////////////////////////////////

		char *inputfilename = argv[1];
		FILE *inputFILE;
		inputFILE = fopen(inputfilename,"r");

		char *outputfilename = argv[2];
		FILE *outputFILE;
		outputFILE = fopen(outputfilename,"w");

		//Get the algorithm from the command line
		char *algorithm = argv[3];

		// Get the limits on the number of processes to handle
		// limit = -1 means no limit.
		int limit = -1;
		if(argv[4] == NULL){
			debug("No limit. To infinity, and beyond.");
			limit = -1;
		}else{
			limit = atoi(argv[4]);
			debug("Limiting to %d processes", limit);
		}

		//Check that they exist
		if (inputFILE == NULL || algorithm == NULL || outputFILE == NULL) {
		  fprintf(stderr, "Can't open the files, and algorithm not specified. please check your argument list...\n");
		  exit(1);
		}

		//initialize the list of processes
		List *pslist = List_create();

		//scan the file
		char *id = malloc(24 * sizeof(id));
		char *arrivalt = malloc(24 * sizeof(arrivalt));
		char *burst = malloc(24 * sizeof(burst));
		int lineCount = 0; // lineCount for limiting number of processes.
			while(fscanf(inputFILE, "%s %s %s", id,arrivalt,burst) != EOF && lineCount != limit) {
				//debug("%s, %s, %s",id,arrivalt,burst);
				lineCount++;
				//process *ps = create_process((int)strtol(id, NULL, 12),(int)strtol(arrivalt, NULL, 12),(int)strtol(burst, NULL, 12),0,0,0,(int)strtol(burst, NULL, 12));
				process *ps = create_process(atoi(id),atoi(arrivalt),atoi(burst),0,0,0,atoi(burst));
				List_push(pslist,ps);
			}

		if(strcmp(algorithm,"FCFS") == 0){
			schedulePS(pslist, FCFS);
		}
		else if(strcmp(algorithm, "SRTF") == 0){
			schedulePS(pslist,SRTF);
		}

		sortPSListByFinishTime(pslist);

		float avgWaitingTime = 0;
		float avgTurnAroundTime = 0;
		LIST_FOREACH(pslist, first, next, cur){
					if(cur->next){
						process *ps = cur->value;
						avgWaitingTime += ps->waiting_time;
						avgTurnAroundTime += (ps->finish_time - ps->start_time);
						//debug("pid: %d, arrival time:%d, cpu burst:%d, finish_time: %d, waiting_time: %d,",ps->pid,ps->arrival_time,ps->burst_time, ps->finish_time, ps->waiting_time);
						fprintf(outputFILE, "%d %d %d %d\n",ps->pid,ps->arrival_time,ps->finish_time, ps->waiting_time);
					}
					if(cur->next == NULL){
						process *ps = cur->value;
						avgWaitingTime += ps->waiting_time;
						avgTurnAroundTime += (ps->finish_time - ps->start_time);
					//	debug("pid: %d, arrival time:%d, cpu burst:%d, finish_time: %d, waiting_time: %d, \n",ps->pid,ps->arrival_time,ps->burst_time, ps->finish_time, ps->waiting_time);
						fprintf(outputFILE, "%d %d %d %d\n",ps->pid,ps->arrival_time,ps->finish_time, ps->waiting_time);
					}
				}
		avgWaitingTime = avgWaitingTime / pslist->count;
		avgTurnAroundTime = avgTurnAroundTime / pslist->count;

		debug("Average Waiting time: %f", avgWaitingTime);
		debug("Average Turn-around time: %f", avgTurnAroundTime);
		/////////////////////////////////
		//
		// Free memory stuff
		//
		/////////////////////////////////

		//debug("bout to free some shiz");
		free(id);
		free(arrivalt);
		free(burst);
		//debug("bout to close input file");
		fclose(inputFILE);
		//debug("bout to close output file");
		fclose(outputFILE);
		//debug("bout to destroy");
		List_clear_destroy(pslist);


		debug("Finished....");
	return 0;
}
