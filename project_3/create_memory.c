

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "smemlib.h"

int main()
{
    
    if(smem_init(32768) != -1){ //32kb
    	printf ("Memory segment is created. Library is ready to use.\n"); 
	}

    return (0); 
}
