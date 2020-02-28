

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <algorithm>

#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>

#include <stdio.h> 
#include <stdlib.h> 
#include <map>
#include <set>

#include "config.h"
#include "instConfig.h"
#include "instUnmap.h"
#include "types.h"


#include <experimental/filesystem>

using namespace std;
namespace fs = std::experimental::filesystem;


static u8* trace_bits;
static s32 shm_id;                    /* ID of the SHM region             */
static unsigned short prev_id;



/* 
    fork
*/
void initAflForkServer(){
    /* start fork */
    int temp_data;
    pid_t fork_pid;

    /* Set up the SHM bitmap. */
    char *shm_env_var = getenv(SHM_ENV_VAR);
    if(!shm_env_var) {
        printf("Error getting shm\n");
        return;
    }
    shm_id = atoi(shm_env_var);
    trace_bits = (u8*)shmat(shm_id, NULL, 0);
    if(trace_bits == (u8*)-1) {
        perror("shmat");
        return;
    }

    // enter fork() server thyme!
    //int n;
    if( write(FORKSRV_FD+1, &temp_data, 4) !=4 ) {
        perror("Error writting fork server\n");
        return;
    }

    /* All right, let's await orders... */
    while(1) {
        
        int stMsgLen = read(FORKSRV_FD, &temp_data, 4);
        if(stMsgLen != 4) {
            /* we use a status message length 2 to indicate a new reading from file. */
            if(stMsgLen == 2){
                exit(EXIT_SUCCESS);
            }
            
            printf("Error reading fork server %x\n",temp_data);
            return;
            
        }

        /* Parent - Fork off worker process that actually runs the benchmark. */
        fork_pid = fork();
        if(fork_pid < 0) {
            printf("Error on fork()\n");
            return;
        }
        /* Child worker - Close descriptors and return (runs the benchmark). */
        if(fork_pid == 0) {
            close(FORKSRV_FD);
            close(FORKSRV_FD+1);
            return;
        } 
        
        /* Parent - Inform controller that we started a new run. */
		if (write(FORKSRV_FD + 1, &fork_pid, 4) != 4) {
    		perror("Fork server write(pid) failed");
			exit(EXIT_FAILURE);
  		}

        /* Parent - Sleep until child/worker finishes. */
		if (waitpid(fork_pid, &temp_data, 2) < 0) {//2: WUNTRACED
    		perror("Fork server waitpid() failed"); 
			exit(EXIT_FAILURE);
  		}

        /* Parent - Inform controller that run finished. 
            * write status (temp_data) of waitpid() to the pipe
        */
		if (write(FORKSRV_FD + 1, &temp_data, 4) != 4) {
    		perror("Fork server write(temp_data) failed");
			exit(EXIT_FAILURE);
  		}
  		/* Jump back to beginning of this loop and repeat. */


    }

}

/* Oracle: exit if not examined 
    random_id: calculate edge id
    flag_id: record examined blocks
*/
void OracleBB(u16 random_id, u32 flag_id){
    if (trace_bits){
        if (trace_bits[flag_id + MAP_SIZE] == 255){ // have not been examined;
            trace_bits[flag_id + MAP_SIZE] = 0; //examined
            exit(BLOCK_EXIT);
        }
        trace_bits[prev_id ^ random_id] ++;
        prev_id = random_id >> 1;
        
    }

}

/* Tracer
    random_id: calculate edge id
    flag_id: record examined blocks
 */
void TracerBB(u16 random_id, u32 flag_id){
    if (trace_bits){
        trace_bits[prev_id ^ random_id] ++;
        prev_id = random_id >> 1; // like AFL

        trace_bits[flag_id + MAP_SIZE] = 0; //edge flag: set to 0: examined

    }
}

/* Trimmer 
    random_id: calculate edge id
 */
void TrimmerBB(u16 random_id){
    if (trace_bits){
        trace_bits[prev_id ^ random_id] ++;
        prev_id = random_id >> 1; // like AFL
    }
}










