#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

struct val{
	struct val * next;	
	int x;
};

struct queue {

	struct val * head, * tail; // head and tail of the list
	int num_items;               // number of jobs currently in queue
};

struct queue *list;
int main(int argc, char **argv){
	list = malloc(sizeof(struct queue) * 10);
	int i;
	list->head = malloc(sizeof(struct val) * 10);
	list->tail=malloc(sizeof(struct val) * 10);
	
	for(i=0; i<10; i++){
		list->head->x = i;
		list->head->next = malloc(sizeof(struct val) * 10);
		printf("%d\n", list->head->x);
		list->head=list->head->next;
		//printf("%s\n", list->head->hello);
	}

}
