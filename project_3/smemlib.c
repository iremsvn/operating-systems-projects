#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <math.h>
#include "smemlib.h"

/*
OS PROJECT 3
IREM SEVEN
21704269
*/

// Define shared memory
#define SHM_SIZE sizeof (struct shared_data) 

// Define semaphore(s)
static sem_t *sem_mutex;       /* protects the buffer */

// Define your stuctures and variables. 
int fd;
struct stat sbuf;           /* info about shm */
struct shared_data * sdp;   /* pointer to shared data structure */
void *shm_start;

int smem_init(int segmentsize)
{
	/* first clean up a shm with same name */
	shm_unlink (SHM_NAME);

	/* first clean up semaphores with same names */
	sem_unlink (SEMNAME_MUTEX);

/*
	double d_sqrt = sqrt((int)segmentsize);
	int int_sqrt = (int) d_sqrt;
	d_sqrt = (double) (d_sqrt - int_sqrt);
	if(d_sqrt != 0){
		printf("memory could not created, the size given in bytes must be power of 2\n");
		return -1;
	}
*/

	/* create a shared memory segment */
	fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0660);
	if (fd < 0) {
		perror("can not create shared memory\n");
		exit (1); 
	}
	//printf("shm created, fd = %d\n", fd);
		
	ftruncate(fd, SHM_SIZE);    /* set size of shared memmory */
	fstat(fd, &sbuf);
	//printf("shm_size=%d\n", (int) sbuf.st_size);

	shm_start = mmap(NULL, sbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (shm_start < 0) {
		perror("can not map shm\n");
		exit (1); 
	}
	//printf ("mapped shm; start_address=%lu\n", (unsigned long) shm_start);
	close(fd); /* no longer need the descriptor */

	sdp = (struct shared_data *) shm_start;

	//segment size arrangement
	if(segmentsize < 32768) //***CHANGE***
		segmentsize = 32768;
	else if(segmentsize > 4194304)
		segmentsize =  4194304; 

	//initialize segment variables
	sdp->h_start = 0;
	sdp->m_start = -1;
	sdp->size = segmentsize;
	sdp->p_num = 0;

	for(int i = 0; i < 10; i++)
		sdp->pid[i] = -1;


	//there will be only a hole segment
	char* ptr = sdp->buf;
	sprintf(ptr, "%d" , -1); //next
	ptr+=8;
	sprintf(ptr, "%d" , (int)((segmentsize - 16) / 8)); //size/8


/*	
	//ONLY FOR TESTING, THE PURPOSE HERE IS TO CREATE A HOLE LIST INITIALLY***
	//THIS HOLE LIST IS NOT IN A MERGED MANNER SINCE IT IS JUST A REPRESANTATION OF A CASE
	//THUS NOT BEING MERGED IS NOT A PROBLEM FOR TESTING
	segmentsize = 32768;
	sdp->h_start = 0;
	sdp->m_start = -1;
	sdp->size = segmentsize;
	sdp->p_num = 0;

	for(int i = 0; i < 10; i++)
		sdp->pid[i] = -1;

	char* test_ptr = sdp->buf;
	int test = sdp->h_start;
	int test_size = 1024*2 - 16;
	sprintf(test_ptr, "%d" , test + test_size + 16); 
	sprintf(test_ptr + 8, "%d" , test_size / 8); 
	test_ptr += test_size + 16;
	test = atoi(&sdp->buf[test]);

	test_size = 1024*4 - 16;
	sprintf(test_ptr, "%d" , test + test_size + 16); 
	sprintf(test_ptr + 8, "%d" , test_size / 8); 
	test_ptr += test_size + 16;
	test = atoi(&sdp->buf[test]);

	test_size = 1024*8 - 16;
	sprintf(test_ptr, "%d" , test + test_size + 16); 
	sprintf(test_ptr + 8, "%d" , test_size / 8); 
	test_ptr += test_size + 16;
	test = atoi(&sdp->buf[test]);

	test_size = 1024*2 - 16;
	sprintf(test_ptr, "%d" , test + test_size + 16); 
	sprintf(test_ptr + 8, "%d" , test_size / 8); 
	test_ptr += test_size + 16;
	test = atoi(&sdp->buf[test]);

	test_size = 1024*16 - 16;
	sprintf(test_ptr, "%d" , -1); 
	sprintf(test_ptr + 8, "%d" , test_size / 8);  
	//END OF TESTING CASE
*/

	/* create and initialize the semaphores */
	sem_mutex = sem_open(SEMNAME_MUTEX, O_RDWR | O_CREAT, 0660, 1);
	if (sem_mutex < 0) {
		perror("can not create semaphore\n");
		exit (1); 
	}
	printf("sem %s created\n", SEMNAME_MUTEX);

	printf ("shared segment of size %d created\n", segmentsize); // remove all printfs when you are submitting to us.
  	return (0); 
}


int smem_remove()
{
	shm_unlink (SHM_NAME);
	sem_close(sem_mutex);
	sdp = NULL;
	shm_start = NULL;
	return (0); 
}

int smem_open()
{

	// open the semaphores they should already be created and initialized
	sem_mutex = sem_open(SEMNAME_MUTEX, O_RDWR);
	if (sem_mutex < 0) {
		perror("cannot open semaphore\n");
		exit (1); 
	} 

	sem_wait(sem_mutex);


	/* open the shared memory segment */
	fd = shm_open(SHM_NAME, O_RDWR, 0600);
	if (fd < 0) {
		perror("can not open shm\n");
		exit (1); 
	}
	
	fstat(fd, &sbuf);	
	
	shm_start = mmap(NULL, sbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (shm_start < 0) {
		perror("can not map the shm \n");
		exit (1); 
	}
	//printf ("mapped shm; start_address=%lu\n", (unsigned long)shm_start);
	close(fd);	
	//p_sdp[p_num] = (struct shared_data *) shm_start;

	sdp = (struct shared_data *) shm_start;

	if(sdp->p_num == 10){
		printf("\ntoo many processes using library try later\n");
		sem_post(sem_mutex);
		return (-1);
	}	

	
	sdp->pid[sdp->p_num] = getpid();
	sdp->p_num++;

	sem_post(sem_mutex);

	return (0); 
}


void *smem_alloc (int size)
{ 
	
	sem_wait(sem_mutex);

	int original_size = size;
	if(size < 8){
		size = 8;
	}
	else if(size%8 != 0){
		int temp = (int) (size/8);
		size = (temp + 1) * 8;
	}

	pid_t pidval;
	pidval = getpid();
	int index;
	for(int i = 0; i < sdp->p_num; i++){
		if(sdp->pid[i] == pidval)
			index = i;	
	}

	
	if(sdp->h_start == -1){ //if hole list is empty,so memory full
		printf("memory full, pid %d could not allocated memory\n", getpid());
		sem_post(sem_mutex);
		return NULL;
	}
	
	int external_fragmentation = 0;
//--------------------------------------------------------------
/*
	//------worst-fit------
	int hole_prev = -1;
	int hole_index = sdp->h_start;
	int hole_size = (atoi((&sdp->buf[sdp->h_start])+8)) * 8;
	int max_size = hole_size;
	int max_prev = hole_prev;
	int max_index = hole_index;
	//find the max avaible size
	while(hole_index != -1){ //if next field is not -1
			hole_size = (atoi((&sdp->buf[hole_index])+8)) * 8;
			if(hole_size > max_size){
				max_size = hole_size;
				max_index = hole_index;
				max_prev = hole_prev;
			}
			else{
				external_fragmentation += hole_size;
				hole_prev = hole_index;
				hole_index = atoi(&sdp->buf[hole_index]);
			 }
	}
	
	if(max_index == -1 || max_size < size){ //if not enough size avaible
		printf("not enough memory avaible\n"); 
		printf("pid %d could not allocated memory ", getpid());	
		printf("EXTERNAL FRAGMENTATION BYTES: %d\n", external_fragmentation); //FOR SPACE TESTING ONLY, DELETE LATER***
		sem_post(sem_mutex);
		return NULL;
	}

	hole_index = max_index;
	hole_prev = max_prev;
	hole_size = max_size;
	//end-of-worst-fit

*/	
//----------------------------------------------------------------------

	//------first-fit: allocate first space that is big enough------
	int hole_size;
	int hole_prev = -1;
	int hole_index = sdp->h_start;
	hole_size = (atoi((&sdp->buf[sdp->h_start])+8)) * 8;
	if(hole_size < size){
		hole_prev = sdp->h_start;
		hole_index = atoi(&sdp->buf[hole_prev]);
		while(hole_index != -1){ //if next field is not -1
			hole_size = (atoi((&sdp->buf[hole_index])+8)) * 8;
			if(hole_size >= size){
				break; //now I have the hole_size and the hole_prev to be used
			}
			else{
				external_fragmentation += hole_size;
				hole_prev = hole_index;
				hole_index = atoi(&sdp->buf[hole_index]);
			 }
		}
	}
	
	if(hole_index == -1){ //if not enough size avaible 
		printf("not enough memory avaible "); 
		printf("pid %d could not allocated memory ", getpid());	
		//printf("EXTERNAL FRAGMENTATION BYTES: %d\n", external_fragmentation); //FOR SPACE TESTING ONLY, DELETE LATER***
		sem_post(sem_mutex);
		return NULL;
	}
	//end-of-first-fit

//----------------------------------------------------------------------

	//now find the corresponding memory index
	int mem_prev = -1;
	int mem_index;
	if(sdp->m_start == -1) //if memory list is empty
		mem_index = -1;
	else{
		mem_index = sdp->m_start;
		if(hole_index > mem_index){ //if hole_index is smaller than mem_start, then mem_index will be the head and mem_prev will be -1
			mem_prev = sdp->m_start;
			mem_index = atoi(&sdp->buf[mem_prev]);
			while(mem_index != -1){ //if next field is not -1
				if(mem_index > hole_index){ //if mem is further from hole_index to be allocated
					break; //now I have the mem_prev to be used
				}
				else{
					mem_prev = mem_index;
					mem_index = atoi(&sdp->buf[mem_index]);
				 }
			}
		}
	}
	

	//providing the space for the process
	char* ptr = sdp->buf;
	int hole_next = atoi(&ptr[hole_index]);
	
	if(sdp->m_start == -1){ //if mem list is empty,conceptually
		sdp->m_start = hole_index;
		sprintf(&ptr[sdp->m_start], "%d" , -1);
	}	
	else if(mem_prev == -1){ //if the addition will be made to head
		sprintf(&ptr[hole_index], "%d" , sdp->m_start); 
		sdp->m_start = hole_index;
	}
	else{
		sprintf(&ptr[hole_index], "%d" , mem_index);
		sprintf(&ptr[mem_prev], "%d" , hole_index);
	}


	//changes with hole list
	int new_hole_size = hole_size - size;
	if(new_hole_size < 24){ 	//16(for headers) + 8(min allocation). If its less than we need to DELETE the hole
		if(hole_prev == -1){ 	//if the deletion will be made on head
			sdp->h_start = hole_next;
		}
		else{ 	//delete not on head
			sprintf(&ptr[hole_prev], "%d" , hole_next);
		}
		//size header will not be change, but to protect area acesses null pointer will be assigned according to alloce request
		//sprintf(&ptr[hole_index + 15 + (size * 8)], "%c" , '\0');
	}
	else{ 	//the case when hole will be ARRANGE, not completely deleted
		if(hole_prev == -1){ //arrange head
			sdp->h_start = hole_index + 16 + size; 	//new index for the hole
			sprintf(&ptr[sdp->h_start], "%d" , hole_next);  //set the next index for the new hole
			sprintf(&ptr[sdp->h_start + 8], "%d" , ((new_hole_size - 16) / 8));    //set new size for hole
		}
		else{
			sprintf(&ptr[hole_prev], "%d" , hole_index + 16 + size); //set prev holes next address
			sprintf(&ptr[atoi(&ptr[hole_prev])], "%d" , hole_next);  //set the next index for the changed hole
			sprintf(&ptr[atoi(&ptr[hole_prev]) + 8], "%d" , ((new_hole_size - 16)/ 8));  //set new size for the hole
		}
		//size header will change for mem
		sprintf((&ptr[hole_index] + 8), "%d" , ((hole_size - new_hole_size) / 8));	//set the size for mem
		//sprintf(&ptr[hole_index + 15 + (size * 8)], "%c" , '\0'); 	//end of allocated memory put null to protect data access
	}
	
	//int internal_frag = 0;
	//internal_frag = (atoi(&sdp->buf[hole_index + 8])*8) - original_size;

	ptr = ptr + hole_index + 15; 	//the new ptr for the allocated space

	//printf("pid %d allocated memory of size %d ", getpid(), size);	
	//printf("INTERNAL FRAGMENTATION BYTES: %d\n", internal_frag); //FOR SPACE TESTING ONLY, DELETE LATER***

	sem_post(sem_mutex);

	return ptr;
}


void smem_free (void *p)
{
	sem_wait(sem_mutex);
	
	int p_next = atoi(p - 15);

	//find the corresponding memory index
	int mem_prev = -1;
	int mem_index = sdp->m_start;
	if(atoi(&sdp->buf[mem_index]) != p_next){ //if the mem_index is not the head mem
		mem_prev = sdp->m_start;
		mem_index = atoi(&sdp->buf[mem_prev]);
		while(mem_index != -1){ //if next field is not -1
			if(atoi(&sdp->buf[mem_index]) == p_next){
				break; //the mem_prev and mem_index to be used, mem_index is for the deallocation
			}
			else{
				mem_prev = mem_index;
				mem_index = atoi(&sdp->buf[mem_index]);
			 }
		}
	}


	//find the corresponding hole indexes
	int hole_prev = -1;
	int hole_index;
	if(sdp->h_start == -1) //if hole list is empty
		hole_index = -1;
	else{
		hole_index = sdp->h_start;
		if(mem_index > hole_index){ //if mem_index is smaller than hole_start, then hole_index will be the head and hole_prev will be -1
			hole_prev = sdp->h_start;
			hole_index = atoi(&sdp->buf[hole_prev]);
			while(hole_index != -1){ //if next field is not -1
				if(hole_index > mem_index){ //if hole is further from mem_index to be allocated
					break; //now I have the mem_prev to be used
				}
				else{
					hole_prev = hole_index;
					hole_index = atoi(&sdp->buf[hole_index]);
				 }
			}
		}
	}	
	

	//make the data empty
	int segment_limit = atoi(&sdp->buf[mem_index + 8]) * 8;
	for(int i = 0; i < segment_limit; i++){
		sprintf(p,"%s" , "");
		p++;
	}
	p = NULL;


	//DELETE MEM
	int mem_next = atoi(&sdp->buf[mem_index]); //the next address of the corresponding mem segment
	if(mem_prev == -1){ 	//if the deletion will be made on head
		sdp->m_start = mem_next;
	}
	else{ 	//delete not on head
		sprintf(&sdp->buf[mem_prev], "%d" , mem_next);
	}
	//size header will not be change, but to protect area acesses null pointer will be assigned according to alloce request
	//sprintf(&ptr[hole_index + 15 + size], "%c" , '\0');


	//ADD TO HOLE
	if(sdp->h_start == -1){ //if hole list is empty
		sdp->h_start = mem_index;
		sprintf(&sdp->buf[sdp->h_start], "%d" , -1);
	}	
	else if(hole_prev == -1){ //if the addition will be made to head
		sprintf(&sdp->buf[mem_index], "%d" , sdp->h_start); 
		sdp->h_start = mem_index;
	}
	else{
		sprintf(&sdp->buf[mem_index], "%d" , hole_index);
		sprintf(&sdp->buf[hole_prev], "%d" , mem_index);
	}

	//NOW MERGE THE HOLES IF NECESSARY
	int hole_cur = mem_index; //just renaming to make things easier

	if(hole_index == (hole_cur + (atoi(&sdp->buf[hole_cur + 8]) * 8) + 16)){
		sprintf(&sdp->buf[hole_cur], "%d" , atoi(&sdp->buf[hole_index]));
		sprintf(&sdp->buf[hole_cur + 8], "%d" , (2 + atoi(&sdp->buf[hole_index + 8]) + atoi(&sdp->buf[hole_cur + 8]))); //2 is for the 15 byte header and 1 byte terminator = 16/8 = 2
		//MAKE THE ADDRESS AND SIZE FIELDS EMPTY TO ACHIEVE SAFER ALLOCATION
		char* ptr = &sdp->buf[hole_index];
		for(int i = 0; i < 16; i++){
			sprintf(ptr,"%s" , "");
			ptr++;
		}
	}
	if(hole_prev != -1){ //if not the first hole
		if(hole_cur == (hole_prev + (atoi(&sdp->buf[hole_prev + 8]) * 8) + 16)){
			sprintf(&sdp->buf[hole_prev], "%d" , atoi(&sdp->buf[hole_cur]));
			sprintf(&sdp->buf[hole_prev + 8], "%d" , (2 + atoi(&sdp->buf[hole_cur + 8]) + atoi(&sdp->buf[hole_prev + 8]))); //2 is for the 15 byte header and 1 byte terminator = 16/8 = 2
			//MAKE THE ADDRESS AND SIZE FIELDS EMPTY TO ACHIEVE SAFER ALLOCATION
			char* ptr = &sdp->buf[hole_cur];
			for(int i = 0; i < 16; i++){
				sprintf(ptr,"%s" , "");
				ptr++;
			}
		}
	}

	sem_post(sem_mutex);
 
}

int smem_close()
{
	sem_wait(sem_mutex);

/*
	printf("******pids********");
	for(int i = 0; i < 10; i++){
		printf("\npid: %ld", (unsigned long) sdp->pid[i]);
		//printf("sdp: %ld\n", (unsigned long) sdp->p_sdp[i]);
	}

	printf("\nnum: %d\n", sdp->p_num);
*/
	

	pid_t pidval;
	pidval = getpid();
	int index;
	for(int i = 0; i < sdp->p_num; i++){
		if(sdp->pid[i] == pidval)
			index = i;
   	}

	sdp->p_num--; //decrease pids
	for(int i = index; i < sdp->p_num; i++){
		sdp->pid[i] = sdp->pid[i + 1];
		//sdp->p_sdp[i] = sdp->p_sdp[i + 1];
	}

	sdp->pid[sdp->p_num] = -1;
   	//sdp->p_sdp[sdp->p_num] = NULL;

	sdp = NULL; //EACH PROCESS HAS ITS OWN COPY OF GLOBAL VARIABLE
	sem_post(sem_mutex);

   	return (0); 
}


//FOR TESTING PURPOSES
void print_mem(){
	sem_wait(sem_mutex);
	printf("\n^^^^^^^MEMORY^^^^^^\n");
	char* ptr = sdp->buf;
	for(int i = 1; i < sdp->size + 1; i++){
		printf("|  %c  ", ptr[i - 1]);
		if(i%8 == 0) printf("\n\n");
	}
	printf("\n");

	printf("h_start: %d\n", sdp->h_start);
	printf("m_start: %d\n", sdp->m_start);

	sem_post(sem_mutex);
}

void print_start(){
	sem_wait(sem_mutex);
	printf("h_start: %d\n", sdp->h_start);
	printf("m_start: %d\n", sdp->m_start);
	sem_post(sem_mutex);
}


void print_headers(){
	sem_wait(sem_mutex);
	printf("HOLE HEADERS\nh_start: %d\n", sdp->h_start);
	if(sdp->h_start != -1){
		int index = sdp->h_start;
		while(index != -1){
			printf("index %d\nnext %d\n",index , atoi(&sdp->buf[index]));
			printf("size %d\n\n", atoi(&sdp->buf[index + 8]));
			index = atoi(&sdp->buf[index]);
		}
	}
/*
	printf("MEMORY HEADERS\nm_start: %d\n", sdp->m_start);
	i = sdp->m_start;
	if(i != -1){
		while(atoi(&sdp->buf[i]) != -1){
			printf("index %d\nnext %d\n",i, atoi(&sdp->buf[i]));
			printf("size %d\n\n", atoi(&sdp->buf[i]));
		}
	}
*/
	sem_post(sem_mutex);
}

