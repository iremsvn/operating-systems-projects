#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h> 
#include <time.h>
#include <sys/time.h>

#define MQNAME "/q_name%d"



struct item {

	int occuranceNum;

	char word[1024];

};




struct Node 

{ 

    char str[1024]; 

    int num;

    struct Node* next; 

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

  


  

/**

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

*/



//***********MAIN**************



int main(int argc, char *argv[])

{

	int N = atoi(argv[1]);

	if (N < 1 || N > 5){
	    printf("Give an integer between 1 and 5.\n");
	    exit(EXIT_FAILURE);
	  }



	mqd_t mq[N];

	struct mq_attr mq_attr;

	//mq_attr.mq_maxmsg = 10000; 

	int n,m, buflen;




	//Open and Assign Files


	//OUTPUT FILE

	FILE* fileOut = fopen(argv[N + 2], "w+");
	
	if (fileOut == NULL)
		exit(EXIT_FAILURE);


	FILE* file[N];

	struct Node *head[N]; 
/**
	for(int i = 0; i < N; i++){

		file[i] = fopen(argv[i + 2], "r");

		if (file[i] == NULL)

			exit(EXIT_FAILURE);



		if (NULL != file[i]) {

    			fseek (file[i], 0, SEEK_END);

    			int size = ftell(file[i]);



    			if (0 == size) {

        			printf("There is an empty file, files cannot be empty!\n");

				exit(EXIT_FAILURE);

    			}

		}

		

		fclose(file[i]);


		//file[i] = fopen(argv[i + 2], "r");

		head[i] = NULL;	

	} 
*/
	
	struct timeval current_time; //*****
    	struct timeval done_time;

	gettimeofday(&current_time, NULL);
	

	//Craeting Message Queues

	for (int i = 0; i < N; i++){

		char buf[32];

	    	char *nameQ = MQNAME;

	    	sprintf(buf, nameQ, i);

		mq[i] = mq_open(buf,O_RDWR | O_CREAT, 0666, 0);

		if(mq[i] == -1) {perror("cannot create msg queue\n"); exit(1);}

	}

	

	struct Node* nodeT = NULL;	



	char name[1024];

	int child = 0;

	

	//Create Children and Process 

	for(int i = 0; i < N ; i++){

    		child = fork();

    		if (child == 0){

			file[i] = fopen(argv[i + 2], "r"); //***
			if (file[i] == NULL)
				exit(EXIT_FAILURE);

			head[i] = NULL;	

			//parse file and create list

			while (fscanf(file[i], "%s", name) != EOF )

			{

				nodeT = (struct Node*)malloc(sizeof(struct Node));

				nodeT->num = 1;

				strcpy(nodeT->str, name);

    				sortedInsert(&head[i], nodeT);	

			}


			fclose(file[i]);		


			//send items on list as list to queue after linked list is created

			struct item item;
			//****
			if(head[i] == NULL){
				item.occuranceNum = -1;
				strcpy(item.word, "");
				n = mq_send(mq[i], (char*)&item,sizeof(struct item),0);
				if(n == -1){perror("send failed\n"); exit(1);}
			}

			struct Node* ptr = head[i];

			while(ptr != NULL){

				item.occuranceNum = ptr->num;

				strcpy(item.word, ptr->str);

				n = mq_send(mq[i], (char*)&item,sizeof(struct item),0);

				if(n == -1){perror("send failed\n"); exit(1);}

				ptr = ptr->next;

			}



				struct Node* temp = head[i];



			   	 while(temp != 0){

			     		temp = head[i]->next;

			     		free(head[i]);

			     		head[i] = temp;

			  	 }

				head[i] = NULL;

				temp = NULL;


				exit(0);

		}



		

	}




			char *bufptr[N];


			//Assign pointers to buffers

			for(int i = 0; i < N; i++){

				mq_getattr(mq[i],&mq_attr);

				buflen = mq_attr.mq_msgsize;

				bufptr[i] = (char*) malloc(buflen);	

			}


			

			int msgnum = 0;		

			struct item *itemptr[N];

			
			int totalmsg = N;
			//To make sure linked list are already calculated, so that we can continue
			int i = 0;
			while (i < N){

				msgnum = 0;

				mq_getattr(mq[i],&mq_attr);

				msgnum = mq_attr.mq_curmsgs;

					

				if(msgnum != 0){

					m = mq_receive(mq[i], (char*)bufptr[i],buflen,NULL);

					if(m == -1){perror("receive failed\n"); exit(1);}

					itemptr[i] = (struct item*) bufptr[i];
					
					if(itemptr[i]->occuranceNum == -1){itemptr[i] = NULL; totalmsg--;} //*****

					i++;

				}

			}


			//int totalmsg = N;

			char min[1024];


			while(1){ 
	
				int counter = 0;

				for (int i = 0; i < N; i++){

					if(itemptr[i]!= NULL){

						if(counter == 0){

							strcpy(min, itemptr[i]->word);

							counter++;

						}

						else if(strcmp(min, itemptr[i]->word) > 0){ strcpy(min, itemptr[i]->word); }

					} 

				}



				int noOfOccurance = 0;

				for (int i = 0; i < N; i++){

					if(itemptr[i]!= NULL){

						if( strcmp(min, itemptr[i]->word) == 0){

							noOfOccurance += itemptr[i]->occuranceNum;

							msgnum = 0;

							mq_getattr(mq[i],&mq_attr);

							msgnum = mq_attr.mq_curmsgs;

							if(msgnum == 0){ sleep(0.05);}

							mq_getattr(mq[i],&mq_attr);

							msgnum = mq_attr.mq_curmsgs;

							if(msgnum != 0){

								m = mq_receive(mq[i], (char*)bufptr[i],buflen,NULL);

								if(m == -1){perror("receive failed\n"); exit(1);}

								itemptr[i] = (struct item*) bufptr[i];

							}

							else{
								itemptr[i] = NULL;
								totalmsg--;
							}

						}

						

					}

				}

				if (noOfOccurance != 0){

					printf("word = %s  ", min);

					printf("occuranceNum = %d\n", noOfOccurance);

					fprintf(fileOut, "%s %d\n", min, noOfOccurance);	

				}



				if(totalmsg == 0){

		

					fclose(fileOut);


					for (int i = 0; i < N; i++){
						mq_close(mq[i]);
					}


					for (int i = 0; i < N; i++){

						free(bufptr[i]);
						bufptr[i] = NULL;
					}

					gettimeofday(&done_time, NULL); //****
					printf("\nseconds : %ld\nmicro seconds : %ld\n", done_time.tv_sec - current_time.tv_sec, done_time.tv_usec - current_time.tv_usec);
					return 0;

				}



			}



	return 0;	


}




