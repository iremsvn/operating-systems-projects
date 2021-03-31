#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
/*
*	İrem Seven      21704269
*	Ataberk Gözkaya 21501928
*/
#define TRACE 1

#define MAXFILENAME 128

int minCpuBurst;
int minIoBurst;
int maxCpuBurst;
int maxIoBurst;
int doneThreads;
char algo[32];
int duration;
int N;
int quantum;
int total = 0;
int totalWait[6];
int response[6];
int* cont;


struct timeval first_time;
struct timeval start_time;
struct timeval current_time;
//*****

struct bb_qelem {
        struct bb_qelem *next;
	int tid;
        int data;  //cpu burst for now change
};


struct bb_queue {
        struct bb_qelem *head;
        struct bb_qelem *tail;
	int count; 
};


void bb_queue_init(struct bb_queue *q)
{
        q->count = 0;
        q->head = NULL;
        q->tail = NULL;
}



void bb_queue_insert(struct bb_queue *q, struct bb_qelem *qe)
{

        if (q->count == 0) {
                q->head = qe;
                q->tail = qe;
        } else {
                q->tail->next = qe;
                q->tail = qe;
        }

        q->count++;
}


struct bb_qelem *bb_queue_retrieve(struct bb_queue *q) 
{
        struct bb_qelem *qe;

        if (q->count == 0)
                return NULL;

	if(strcmp(algo, "FCFS") == 0){

		qe = q->head;
		q->head = q->head->next;
		q->count--;
	
	}
	
	
	//ROUND ROBIN
	if(strcmp(algo, "RR") == 0){
	  	qe = q->head;
		//printf(qe->data);
		//int quantum = 50;  
		cont[qe->tid - 1] = 0;
		
		if(qe->data > quantum){ //**************Q*****************
			usleep(quantum);

			qe->data = qe->data - quantum;
//printf(qe->data);

			q->head = q->head->next;

			if(q->count == 1) q->head = qe; else q->tail->next = qe; //**changed**

			q->tail = qe;
			cont[qe->tid - 1] = 0;
			
			
		}
		else{
			
			usleep(qe->data);

			q->head = q->head->next;

			cont[qe->tid - 1] = 1;
			q->count--;
			
		}
	}



	if(strcmp(algo, "SJF") == 0){
		
		struct bb_qelem *temp;
		temp = q->head;
		int min = temp->data;
		qe = temp; 
		    while (temp != NULL) { 
		  
			if (min > temp->data){ 
			    min = temp->data;
			    qe = temp; 
			}
		  
			temp = temp->next; 
		    }  
		if(qe == q->head){
			q->head = q->head->next;
		}
		else{
			temp = q->head;
			while(temp->next != qe){
				temp = temp->next;
			}
			if( qe == q->tail) q->tail = temp; //**changed**
			temp->next = qe->next;
		}
		q->count--;
	}
	
        return (qe);
}



struct bounded_buffer {
        struct bb_queue *q;               /* bounded buffer queue  */
        pthread_mutex_t th_mutex_queue;   /* mutex  to protect queue */
        pthread_cond_t  th_cond_hasspace;  /* will cause producer to wait */
        pthread_cond_t  th_cond_hasitem;   /* will cause consumer to wait */
};


void bb_add(struct  bounded_buffer* bbp, struct bb_qelem *qep, int flag, int numberWait, int contIndex)
{
	pthread_mutex_lock(&bbp->th_mutex_queue);

	if(flag == 1) cont[contIndex] = 1;

	/* critical section begin */
	while (flag == 0 || bbp->q->count == N - doneThreads || cont[contIndex] == 0){ 
			flag = 1; //**changed**
			pthread_cond_wait(&bbp->th_cond_hasspace, &bbp->th_mutex_queue);
			
			usleep(numberWait);
			total = total + numberWait;
			
	}
	flag = 0;
	bb_queue_insert(bbp->q, qep); //

	if (TRACE) {
		printf ("producer insert item = %d\n", qep->data);
		fflush (stdout);
	}
		
	pthread_cond_signal(&bbp->th_cond_hasitem);

	/* critical section end */
	pthread_mutex_unlock(&bbp->th_mutex_queue);
}


struct bb_qelem *bb_rem (struct bounded_buffer *bbp, int firstBurst)
{
	struct bb_qelem *qe;
	pthread_mutex_lock(&bbp->th_mutex_queue);
	
	while (bbp->q->count == 0) {
		pthread_cond_wait(&bbp->th_cond_hasitem, &bbp->th_mutex_queue);
	}
	//printf("failed rem 1\n");
	qe = bb_queue_retrieve(bbp->q);
	//printf("failed rem 2\n");
	if (qe == NULL) {
		printf("can not retrieve; should not happen\n");
		exit(1);
	}
	
	if (TRACE) {
		printf ("consumer retrieved item = %d\n", qe->data);
		fflush (stdout);
	}

	//TIME GET
	if(firstBurst == 1){
		gettimeofday(&first_time, NULL);
	}
	else{
		gettimeofday(&current_time, NULL);
		start_time.tv_usec =  ((current_time.tv_sec - first_time.tv_sec) * 1000000) + (current_time.tv_usec - first_time.tv_usec); //**changed**
	}
	printf("%d %d\n", qe->tid, total);
	for(int i = 1; i < N+1; i++)
	{
		if(qe->tid == i)
		{
			totalWait[i] = totalWait[i] + total;
		}
	}
	total = total + qe->data;
	for(int i = 1; i < N+1; i++)
	{
		if(qe->tid == i)
		{
			response[i] = response[i] + total;
		}
	}

	//ROUND ROBIN
	if(strcmp(algo, "RR") != 0){
	  	usleep(qe->data);
		cont[qe->tid - 1] = 1;
	}
	  
	
	pthread_cond_signal(&bbp->th_cond_hasspace); //when deleted send signal to producer
	
	pthread_mutex_unlock(&bbp->th_mutex_queue);
	return (qe); 
}

int generateRandom(int min, int max)
{
	int num = max + 1;
	while(num > max || num < min)
	{ 
		num = 1 + rand() % 9;
		num = num * 100;	
    }
    
    return num;
}


char outfilename[MAXFILENAME];
char filePrefix[MAXFILENAME];


struct bounded_buffer *bbuffer;  


void *producer (void * arg)
{
	int* tIndex = arg;
	//cont[*tIndex] = 0;
	
	int tempDur = duration;
	if(strcmp(filePrefix, "no-infile") == 0)
	{
		int numberWait;
		int flag = 1;
		struct bb_qelem *qe;
		while(tempDur > 0)
		{
			int number = generateRandom(minCpuBurst, maxCpuBurst);
			qe = (struct bb_qelem *) malloc (sizeof (struct bb_qelem));
			if (qe == NULL) 
			{
				perror ("malloc failed\n"); 
				exit (1); 
			}
			qe->next = NULL; 
			qe->data = number;
			qe->tid = *tIndex;
			numberWait = generateRandom(minIoBurst ,maxIoBurst);
			
			bb_add (bbuffer, qe, flag, numberWait, (*tIndex - 1));
			flag = 0;
			tempDur--;
		}
	}
	else
	{	
		FILE *fp; 
		int number;
		int numberWait;
		char type[32]; 
		struct bb_qelem *qe;
		char infilename[MAXFILENAME]; 
		sprintf(infilename, "%s%d.txt", filePrefix, *tIndex); 
		fp = fopen (infilename, "r"); 
		int flag = 1;
		while (fscanf (fp, "%s", type) != EOF) {
			fscanf (fp, "%d", &number);
			if(strcmp(type, "CPU") == 0){
				qe = (struct bb_qelem *) malloc (sizeof (struct bb_qelem)); 
				if (qe == NULL) {
					perror ("malloc failed\n"); 
					exit (1); 
				}
				qe->next = NULL; 
				qe->data = number;
				qe->tid = *tIndex;
				numberWait = 0; 
				if(fscanf (fp, "%s", type) != EOF){ //if not the last line is cpu burst
					fscanf (fp, "%d", &numberWait);
					//printf("%d BEKLE\n", numberWait);
				}
				bb_add (bbuffer, qe, flag, numberWait, (*tIndex - 1));
				flag = 0;
			}
		}
	

		fclose (fp);
	} 
	
	doneThreads++;
	//printf("doneThreadsPRO %d\n", doneThreads);
	printf ("producer terminating\n");  fflush (stdout); 

	pthread_exit (NULL); 

}




int main(int argc, char **argv)
{
	N = atoi(argv[1]);
	minCpuBurst = atoi(argv[2]);
	maxCpuBurst = atoi(argv[3]);
	minIoBurst = atoi(argv[4]);
	maxIoBurst = atoi(argv[5]);
	duration = atoi(argv[7]);
	quantum = atoi(argv[9]);
	

	pthread_t prodtid[N]; 
	int ret;
	doneThreads = 0;

	if (argc != 11) {
		printf ("usage: mutexcond <infile> <outfile> <algo>\n"); 
		exit (1); 
	}

	int threadIndex[N];
		
	strcpy(filePrefix, argv[10]);
	strcpy(outfilename, argv[6]); 
	strcpy(algo, argv[8]); 

	bbuffer =  (struct bounded_buffer *) malloc(sizeof (struct bounded_buffer));
	bbuffer->q = (struct bb_queue *) malloc(sizeof (struct bb_queue));
	bb_queue_init(bbuffer->q);
	pthread_mutex_init(&bbuffer->th_mutex_queue, NULL);
	pthread_cond_init(&bbuffer->th_cond_hasspace, NULL);
	pthread_cond_init(&bbuffer->th_cond_hasitem, NULL);

	int tempCont[N];
	cont = tempCont;

	for(int i = 0; i < N; i++){
		threadIndex[i] = i + 1;
		ret = pthread_create (&prodtid[i], NULL, producer, &threadIndex[i]); 
		if (ret != 0) {
			perror ("thread create failed\n"); 
			exit (1); 
		}
	}


	//****
	FILE *fp; 
	struct bb_qelem *qe; 
	int firstBurst = 1;
	fp = fopen (outfilename, "w"); 
	
	while (1) {
		 
		qe = bb_rem (bbuffer, firstBurst); // one thread at a time
		
		firstBurst = 0;
		//printf("doneThreadsCON %d\n", doneThreads);
		if (doneThreads != N || bbuffer->q->count != 0) {
			if(qe != NULL){
			fprintf (fp, "%0.10ld %d %d\n", start_time.tv_usec, qe->data, qe-> tid); 
			fflush (fp);
			 
			if(cont[qe->tid - 1] == 1) free (qe);
			
			} 
		}
		else {
		    if(qe != NULL){ 
			    fprintf (fp, "%0.10ld %d %d\n", start_time.tv_usec, qe->data, qe-> tid); 
			    fflush (fp);
			   
			    if(cont[qe->tid - 1] == 1) free (qe);
			    
		    } 
			break; 
		}
	}

	fclose (fp); 
	
	printf ("consumer terminating\n"); fflush (stdout); 

	//****


	for(int i = 0; i < N; i++){
		pthread_join (prodtid[i], NULL); 
	}


	free(bbuffer->q);
	free(bbuffer);
	
	pthread_mutex_destroy(&bbuffer->th_mutex_queue);
	pthread_cond_destroy(&bbuffer->th_cond_hasspace);
	pthread_cond_destroy(&bbuffer->th_cond_hasitem);

	for(int i = 1; i < N+1; i++)
	{
		printf("%s %d %s %d\n", "waiting time of", i, "is", totalWait[i]);
	}
	for(int i = 1; i < N+1; i++)
	{
		printf("%s %d %s %d\n", "average response time of", i, "is", response[i]/duration);
	}

	printf ("closing...\n"); 
	return 0; 
}
