#include "Bank.h"
#include "Bank.c"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

struct trans {  // holds one transcation to perform
	int accountID;
	double balance;
};

struct request { // holds one request to perform in the queue
	struct request * next;  // pointer to the next request in the list
	int request_id;      // request ID assigned by the main thread
	int check_acc_id;  // account ID for a CHECK request
	struct trans * transactions; // array of transaction data
	int num_trans;     // number of accounts in this transaction
	struct timeval starttime, endtime; // starttime and endtime for TIME
};

struct queue { // a queue of requests to process
	struct request * head, * tail; // head and tail of the list
	int num_jobs;               // number of jobs currently in queue
};

void *worker(void *); // method that processes what a worker thread should do 

pthread_mutex_t mut_queue; // mutex lock for request processing
pthread_mutex_t *mut_locks; // array of mutex locks for all accounts
pthread_cond_t worker_cv; // mutex conditional to trigger worker threads
struct queue *list; // the queue that holds all requests
int numThreads; // the number of worker threads the user requests
int numAccounts; // the number of accounts the user requests
FILE *outputFile; // FILE pointer to write to a file

int main(int argc, char **argv){                   
	char nextCmd[1024];  // the request user enters every iteration
	if(argc !=4){ // case when user does not enter valid information in the server
		fprintf(outputFile, "Error: %d\n", argc);
		exit(0);
	}
	// read user inputs
	numThreads = atoi(argv[1]);
	numAccounts = atoi(argv[2]);
	outputFile = fopen(argv[3], "w");

	pthread_t worker_threads[numThreads]; // worker threads

	// initialize mutexes
	pthread_mutex_init(&mut_queue, NULL);
	pthread_cond_init(&worker_cv, NULL);
	mut_locks = malloc(sizeof(pthread_mutex_t) * numAccounts);
	int j =0;
	for(j = 0; j<numAccounts ; j++){
		pthread_mutex_init(&mut_locks[j], NULL);
	}


	pthread_mutex_lock(&mut_queue);
	//initalize the queue and its contents
	list = malloc(sizeof(struct queue) * 100000);
	list->head = malloc(sizeof(struct request) * 1024);
	list->tail= malloc(sizeof(struct request) * 1024);
	int i = 0;

	initialize_accounts(numAccounts);// initialze bank accounts 
	int requestNum = 0; // the current request number
	pthread_mutex_unlock(&mut_queue);

	/* creating worker threads and assigning them to the worker thread*/
  	for (i = 0; i < numThreads; i++)
  	{
    	pthread_create(&worker_threads[i], (void *)NULL, worker, (void *)NULL);
  	}
	
	while(1){ // a constant loop to get user's commands

		//get users's command
		fgets(nextCmd, 1024, stdin);
		
		nextCmd[strlen(nextCmd)-1] = '\0';
		char *act = strtok(nextCmd, " ");
		if(strcmp(act,"CHECK") ==0){ // BALANCE CHECK REQUEST
			
			char *temp;
			temp = strtok(NULL, " ");	
			int ID = atoi(temp);
			if(ID <= numAccounts && ID > 0){ // checks if the user entered account ID is valid
				
				pthread_mutex_lock(&mut_queue);
				requestNum++;
				printf("ID %d\n",requestNum);
				if(list->num_jobs == 0){// first job is added to the queue
					// creating a tempeorary request with the neccesary data
					struct request *temp_request = malloc(sizeof(struct request) * 1024);
					temp_request->request_id = requestNum;
					temp_request->check_acc_id = ID;
					temp_request->num_trans = -1;

					//add the request to the queue
					list->head = temp_request;
					list->tail = temp_request;
					list->num_jobs = 1;	
	
					pthread_cond_broadcast(&worker_cv); // signal the worker threads

				}else{ // every other job added to the queue
					
					// creating a tempeorary request with the neccesary data
					struct request *temp_request = malloc(sizeof(struct request) * 1024);
					temp_request->request_id = requestNum;
					temp_request->check_acc_id = ID;
					temp_request->num_trans = -1;
					
					//add the request to the queue
					list->tail->next = temp_request;
					list->tail = temp_request;
					list->num_jobs = list->num_jobs + 1;
	
					pthread_cond_broadcast(&worker_cv);	// signal the worker threads			
				}
				pthread_mutex_unlock(&mut_queue);
							
			}
				
		}else if(strcmp(act,"TRANS") ==0){ // TRANSACTION REQUEST
			pthread_mutex_lock(&mut_queue);
			requestNum++;
			struct trans *actions = malloc(sizeof(*actions) * 100); // array of transactions to perform
			int i = 0;
			int id, amt;
			char *str = strtok(NULL, " ");
			while (str != NULL) { // read all the transcations user requested to perform and store it in the array
				id = atoi(str);
				str = strtok(NULL, " ");
				amt = atoi(str);
				str = strtok(NULL, " ");
				struct trans *t =  malloc(sizeof(*t) * 1024);
				t->accountID = id;
				t->balance = amt;
				actions[i] = *t;
				i++;
			}
			// creating a tempeorary request with the neccesary data
			struct request *temp_request = malloc(sizeof(*temp_request) * 1024);
			temp_request->request_id = requestNum;
			temp_request->check_acc_id = -1;
			temp_request->num_trans = i;
			temp_request->transactions = actions;
			printf("ID %d\n",requestNum);
				//add the request to the queue
				if(list->num_jobs == 0){// first job is added to the queue
					
					list->head = temp_request;
					list->tail = temp_request;
					list->num_jobs = 1;	

					pthread_cond_broadcast(&worker_cv); // signal the worker threads	
				}else{ // every other job added to the queue
					list->tail->next = temp_request;
					list->tail = temp_request;
					list->num_jobs = list->num_jobs + 1;

					pthread_cond_broadcast(&worker_cv);	// signal the worker threads				
				
							
				}
			pthread_mutex_unlock(&mut_queue);
		}else if(strcmp(act,"END") ==0){
			pthread_mutex_lock(&mut_queue);
			struct request *temp_request = malloc(sizeof(*temp_request) * 1024);
			temp_request->request_id = requestNum;
			temp_request->check_acc_id = -1;
			temp_request->num_trans = -1;
			printf("ID %d\n",requestNum);
			

			if(list->num_jobs == 0){// first job is added to the queue
					list->head = temp_request;
					list->tail = temp_request;
					list->num_jobs = 1;	
					pthread_cond_broadcast(&worker_cv); // signal the worker threads	
			}else{ // every other job added to the queue
					list->tail->next = temp_request;
					list->tail = temp_request;
					
					list->num_jobs = list->num_jobs + 1;
					pthread_cond_broadcast(&worker_cv);		// signal the worker threads					
			}
		
			pthread_mutex_unlock(&mut_queue);
		}
	}

}

void *worker(void *arg){
	while(1){
	pthread_mutex_lock(&mut_queue);
	
  	while (list->num_jobs == 0) // wait for jobs to be added to the queue
    	pthread_cond_wait(&worker_cv, &mut_queue);

	// variable to record start and end time of the queue
	struct timeval startTime;
	struct timeval endTime;
	if(list->head->check_acc_id > 0 && list->head->check_acc_id <= numAccounts){// its an balance check request
		gettimeofday(&startTime, NULL); // get start time
		list->head->starttime=startTime;
		char output[2000]; // output to write to the file
		
		//read the user request account and return the balance
		pthread_mutex_lock(&mut_locks[list->head->check_acc_id -1]);
		int bal = read_account(list->head->check_acc_id);
		sprintf(output, "%d BAL %d TIME %ld.%06.ld", list->head->request_id, bal, startTime.tv_sec, startTime.tv_usec );
		pthread_mutex_unlock(&mut_locks[list->head->check_acc_id -1]); 

		// remove the request from the queue
		list->num_jobs = list->num_jobs -1;
		list->head= list->head->next;
		//get end time of the request processing
		gettimeofday(&endTime, NULL);

		// add the end time to the output
		char temp[2000];
		sprintf(temp, " %ld.%06.ld\n", endTime.tv_sec, endTime.tv_usec);
		strcat(output, temp);

		fprintf(outputFile, "%s",output);// write to the file
		
			
	}else if(list->head->num_trans > 0){ // is a transaction request
		int k =0;
		for(k =0 ;k <list->head->num_trans;k++){ // lock all the accounts corresponding to the request 
			pthread_mutex_lock(&mut_locks[list->head->transactions[k].accountID]);		
		}
	
		int numberTrans = list->head->num_trans; // number of transactions in this request
		gettimeofday(&startTime, NULL); //get start time
		list->head->starttime=startTime;

		int isValid = 1; // boolean to see if all the transactions are valid
		char output[2000]; // output to write to the file
		int id; // keeps track of the account ID that might have invalid transaction request
		int i = 0;
		for(i = 0;i < list->head->num_trans ; i++){ // validate to see if all the transactions are valid
			id = list->head->transactions[i].accountID;
			int balance = read_account(id);	
			if(list->head->transactions[i].balance + balance < 0){
				isValid = 0;
				break;
			}
		}
		if(isValid){ // if all accounts are valid process the request and update the accounts
			int j=0;
			for(j=0;j<list->head->num_trans;j++){
				int accountID = list->head->transactions[j].accountID;
				int balance = read_account(accountID);	
				write_account(accountID, list->head->transactions[j].balance + balance);		
			} 
			sprintf(output, "%d OK TIME %ld.%06.ld ", list->head->request_id, startTime.tv_sec, startTime.tv_usec);
			
		}else{ // output that it was an invalid transaction request for given account ID
			sprintf(output, "%d ISF %d TIME %ld.%06.ld ", list->head->request_id, id, startTime.tv_sec, startTime.tv_usec);
		}
		// get the end time of the processing	
		gettimeofday(&endTime, NULL);
		char temp[100];
		sprintf(temp, "%ld.%06.ld\n", endTime.tv_sec, endTime.tv_usec);
		strcat(output, temp);
		fprintf(outputFile, "%s", output);// write it to the file

		int l =0;
		for(l =0 ;l < numberTrans;l++){ // unlock all the accounts mutexes
			pthread_mutex_unlock(&mut_locks[list->head->transactions[l].accountID]);		
		}
		//remove the request from the queue
		list->num_jobs = list->num_jobs -1;
		list->head= list->head->next;
	}else{// is a end request
		fclose(outputFile); // close the file
		exit(0);	
	}
	
	
	pthread_mutex_unlock(&mut_queue);
	
	}

}
