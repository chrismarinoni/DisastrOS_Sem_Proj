#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semPost(){
  
  int fd = running->syscall_args[0];
	
	SemDescriptor* semDesc = SemDescriptorList_byFd(&running->sem_descriptors, fd);
	
	if(!semDesc){
		printf("[SEM_ERR] Trying to close a non-existing semaphore!\n");
		running->syscall_retvalue = DSOS_ESEMWAIT_NOT_EXIST;
		return;
	}
	
	Semaphore* sem = semDesc->semaphore;
		
	if(!sem){
		printf("[SEM_ERR] I wasn't able to get the semaphore to destroy!\n");
		running->syscall_retvalue = DSOS_ESEMWAIT_NOT_FOUND;
		return;
	}
	
	SemDescriptorPtr* semDescPtr = semDesc->ptr;
					
	if(!semDescPtr){
		printf("[SEM_ERR] I wasn't able to get the semDescPtr!\n");
		running->syscall_retvalue = DSOS_ESEMWAIT_NOT_FOUND;
		return;
	}
	
	
	if(++sem->count <= 0) {
		List_insert(&ready_list, ready_list.last, (ListItem*) running);
		List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*)semDescPtr);
		List_detach(&waiting_list, (ListItem*)semDesc->pcb);
		running->status = Ready;
		running = semDesc->pcb;
		
	}
  
}
