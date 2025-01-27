#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){
	
	/**
	   TO DO:
		* increments the given semaphore
		* if the semaphore was at 0, and some other thread was waiting
		* the thread is resumed
		* returns 0 on success, an error code on failure 
	**/
	
	// Same initial stuff 
	
	int fd = running->syscall_args[0];
	
	SemDescriptor* semDesc = SemDescriptorList_byFd(&running->sem_descriptors, fd);
	
	if(!semDesc){
		printf("[SEM_ERR] Trying to Wait a non-existing semaphore!\n");
		running->syscall_retvalue = DSOS_ESEMWAIT_NOT_EXIST;
		return;
	}
	
	Semaphore* sem = semDesc->semaphore;
		
	if(!sem){
		printf("[SEM_ERR] I wasn't able to get the semaphore to Wait!\n");
		running->syscall_retvalue = DSOS_ESEMWAIT_NOT_FOUND;
		return;
	}
	
	SemDescriptorPtr* semDescPtr = semDesc->ptr;
					
	if(!semDescPtr){
		printf("[SEM_ERR] I wasn't able to get the semDescPtr!\n");
		running->syscall_retvalue = DSOS_ESEMWAIT_NOT_FOUND;
		return;
	}
	
	// Decrement count and check if < 0.
	// If true, we operate a context switch by putting the current process
	// in the waiting queue and taking the next process to run from the
	// Ready queue. 
	
	if(--(sem->count) < 0) {
		List_detach(&sem->descriptors, (ListItem*) semDescPtr);
		List_insert(&sem->waiting_descriptors, sem->waiting_descriptors.last, (ListItem*) semDescPtr);
		running->status = Waiting;
		List_insert(&waiting_list, waiting_list.last,(ListItem*) running);
		PCB* pcb_aux = (PCB*) List_detach(&ready_list, (ListItem*) ready_list.first);
		running = pcb_aux;
	}
	
	printf("[SEM_INFO]Thread #%d has correctly launched a semWait on sem id #%d\n", disastrOS_getpid(), fd);
	
	running->syscall_retvalue = 0;
	return;
	
	
}
