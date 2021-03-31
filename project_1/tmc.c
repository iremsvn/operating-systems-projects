#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>


// Link list node 
struct Node 
{ 
    char str[1024]; 
    int num;
    struct Node* next; 
}; 

  

//Arg Node referencing to the thread function
struct ArgNode 
{ 
    char* inputFileName; 
    struct Node **list;
 
};



void sortedInsert(struct Node**, struct Node*); 
 

void sortedInsert(struct Node** head_ref, struct Node* new_node) 
{ 
    struct Node* current;

    current = *head_ref;
 
    while(current != NULL){
	if (strcmp(current->str,new_node->str) == 0){
		current->num = current->num + 1;
		free(new_node);
		new_node = NULL;
		return;
	}
	current = current->next;
    }

    if (*head_ref == NULL || strcmp((*head_ref)->str,new_node->str) > 0)
    { 
        new_node->next = *head_ref; 
        *head_ref = new_node;
	new_node = NULL; 
    } 
    else
    { 
        current = *head_ref; 
        while (current->next!=NULL && 
               strcmp(current->next->str, new_node->str) < 0) 
        { 
            current = current->next; 
        } 
        new_node->next = current->next; 
        current->next = new_node;
	new_node = NULL; 
    } 
} 
  


void printList(struct Node *head) 
{ 
    struct Node *temp = head; 
    while(temp != NULL) 
    { 
        printf("%s  ", temp->str);
	printf("num: %d  ", temp->num);
	printf("\n");  
        temp = temp->next; 
    } 
} 



//****************************

void *processFile(void *args) {

    struct ArgNode *actual_args = args;

    FILE *fr;

    fr = fopen(actual_args->inputFileName, "r");

    if (fr == NULL) {
        exit(EXIT_FAILURE);
    }

    char name[1024];
    struct Node* nodeT = NULL;	
    while (fscanf(fr, "%s", name) != EOF )
	{
		nodeT = (struct Node*)malloc(sizeof(struct Node));
		nodeT->num = 1;
		strcpy(nodeT->str, name);
		sortedInsert((actual_args->list), nodeT);
	}
			
   fclose(fr);

	//printList(*(actual_args->list));		

    return 0;
}








//***********MAIN**************
int main(int argc, char *argv[])
{
	const int N = atoi(argv[1]);

	if (N < 1 || N > 5){
	    printf("Give an integer between 1 and 5.\n");
	    exit(EXIT_FAILURE);
	  }

	FILE* fileOut = fopen(argv[N + 2], "w+");

	if (fileOut == NULL) {
		exit(EXIT_FAILURE);
    	}

	struct timeval current_time; //*****
    	struct timeval done_time;

   	gettimeofday(&current_time, NULL);

	struct Node *root[N];
	struct ArgNode *arg[N];

	
	for (int i = 0; i < N; i++) {
		root[i] = NULL; 
		arg[i] = malloc(sizeof *arg[0]);
    	}

	int err;
	pthread_t tid[N];
	
	for (int i = 0; i < N; i++)
	    {

		arg[i]->inputFileName = argv[i + 2];
		arg[i]->list = &root[i];

		err = pthread_create(&(tid[i]), NULL, &processFile, arg[i]);

		if (err != 0)
		    printf("\nthread error:[%s]", strerror(err));


		

   	   }
	for(int i = 0; i < N; i++)
		pthread_join(tid[i], NULL);


			struct Node *itemptr[N];
			
			for (int i = 0; i < N; i++){
				itemptr[i] = (struct Node*)*(arg[i]->list);
				
			}

			char min[1024];
			//int finishedLists = 0;
			int finishedAll = 0; //***
			
			while(finishedAll == 0){ 
				int counter = 0;
				finishedAll = 1; //***
				for (int i = 0; i < N; i++){
					if(itemptr[i] != NULL){
						if(counter == 0){
							strcpy(min, itemptr[i]->str);
							counter++;
						}
						else if(strcmp(min, itemptr[i]->str) > 0){ strcpy(min, itemptr[i]->str); }
						finishedAll = 0; //***	
					}
				}

				int noOfOccurance = 0;
				for (int i = 0; i < N; i++){
					if(itemptr[i]!= NULL){
						if( strcmp(min, itemptr[i]->str) == 0){
							noOfOccurance += itemptr[i]->num;
							itemptr[i] = itemptr[i]->next;
						}	
					}
					//else{finishedLists++;}
				}
				if (noOfOccurance != 0){
					printf("word = %s  ", min);
					printf("occuranceNum = %d\n", noOfOccurance);
					fprintf(fileOut, "%s %d\n", min, noOfOccurance);
				}	
					
			}
		
		fclose(fileOut);

		for (int i = 0; i < N; i++) {
			free(root[i]);
			free(arg[i]);
		    }

	gettimeofday(&done_time, NULL); //****
	printf("\nseconds : %ld\nmicro seconds : %ld\n", done_time.tv_sec - current_time.tv_sec, done_time.tv_usec - current_time.tv_usec);
	
	return 0;
}
