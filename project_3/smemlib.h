#ifndef SMEMLIB_H
#define SMEMLIB_H

#define SEMNAME_MUTEX       "/mutex_name"

#define SHM_NAME  "/memory_name"

#define BUFSIZE  4194304		/*  bounded buffer size */



struct shared_data {
	char buf[BUFSIZE];    /* shared buffer */
	int h_start;
	int m_start;
	size_t size;

	//****
	int p_num;
	pid_t pid[10];
	//struct shared_data *p_sdp[10];
};


int smem_init(int segmentsize); 
int smem_remove(); 

int smem_open(); 
void *smem_alloc (int size);
void smem_free(void *p);
int smem_close();
//to test it prints memory segment
void print_mem();
void print_start();
void print_headers();
#endif

    
    
