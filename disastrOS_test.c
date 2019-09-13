#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"

#define sizeOfBuffer 50
#define DEBUG 1

int to_write = 0;

int sem_read, sem_write, sem_empty, sem_filled;
int buffer[sizeOfBuffer];
int read_index = 0, write_index = 0;


// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void producer() {
	
	
	if(DEBUG) printf("Sto nel producer\n");		
	disastrOS_semWait(sem_empty);
	disastrOS_semWait(sem_write);
	
	buffer[write_index] = to_write;
	to_write++;
	write_index = (write_index+1) % sizeOfBuffer;
	printf("[SEM_INFO] Il figlio #%d Un nuovo dato è stato prodotto!\n", disastrOS_getpid());
	
	disastrOS_semPost(sem_write);
	disastrOS_semPost(sem_filled); 
	
	
}

void consumer() {
			
	if(DEBUG) printf("Sto nel consumer\n");
	disastrOS_semWait(sem_filled);
	disastrOS_semWait(sem_read);
	
	int value = buffer[read_index];
	read_index = (read_index+1) % sizeOfBuffer;
	printf("[SEM_INFO] Il figlio #%d ha letto il valore %d come nuovo dato!\n", disastrOS_getpid(),  value );
	
	disastrOS_semPost(sem_read);
	disastrOS_semPost(sem_empty); 
	
	
}

void childFunction(void* args){
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int type=0;
  int mode=0;

  int sem_read = disastrOS_semOpen(1, 1);
  int sem_write = disastrOS_semOpen(2, 1);
  int sem_filled = disastrOS_semOpen(3, 0);
  int sem_empty = disastrOS_semOpen(4, sizeOfBuffer);
  
  for(int i = 0; i < 3*sizeOfBuffer; i++){

	if(i%2 == 0){
		if(DEBUG) printf("Sono diventato consumer #%d\n", disastrOS_getpid());
		consumer();
	}else{
		if(DEBUG) printf("Sono diventato producer #%d\n", disastrOS_getpid());
		producer();
	}
	
  }
  disastrOS_semClose(sem_read);
  disastrOS_semClose(sem_write);
  disastrOS_semClose(sem_filled);
  disastrOS_semClose(sem_empty);
  

  int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
  printf("fd=%d\n", fd);
  printf("PID: %d, terminating\n", disastrOS_getpid());
  
  for (int i=0; i<(disastrOS_getpid()+1); ++i){
    printf("PID: %d, iterate %d\n", disastrOS_getpid(), i);
    disastrOS_sleep((20-disastrOS_getpid())*5);
  }
  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);
  

  printf("I feel like to spawn 10 nice threads\n");
  int alive_children=0;
  for (int i=0; i<10; ++i) {
    int type=0;
    int mode=DSOS_CREATE;
    printf("mode: %d\n", mode);
    printf("opening resource (and creating if necessary)\n");
    int fd=disastrOS_openResource(i,type,mode);
    printf("fd=%d\n", fd);
    disastrOS_spawn(childFunction, 0);
    alive_children++;
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);
    --alive_children;
  }
  printf("shutdown!");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
