#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

#define DEBUG 1

void internal_semClose(){
	
	int fd = running->syscall_args[0];
	
	SemDescriptor* semDesc = SemDescriptorList_byFd(&running->sem_descriptors, fd);
	
	if(!semDesc){
		printf("[SEM_ERR] Trying to close a non-existing semaphore!\n");
		running->syscall_retvalue = DSOS_ESEMCLOSE_NOT_EXIST;
		return;
	}
	
	List_detach(&running->sem_descriptors, (ListItem *)semDesc);
	
	if(DEBUG) printf("Test 1\n");

	Semaphore* sem = semDesc->semaphore;
		
	if(sem==0){
		printf("[SEM_ERR] I wasn't able to get the semaphore to destroy!\n");
		running->syscall_retvalue = DSOS_ESEMCLOSE_NOT_FOUND;
		return;
	}
	
	SemDescriptorPtr* semDescPtr = (SemDescriptorPtr*)List_detach(&sem->descriptors, (ListItem*)(semDesc->ptr));
					
	if(!semDescPtr){
		printf("[SEM_ERR] I wasn't able to get the semDescPtr!\n");
		running->syscall_retvalue = DSOS_ESEMCLOSE_NOT_FOUND;
		return;
	}

	List_detach(&running->sem_descriptors, (ListItem*) semDesc); 
	List_detach(&semaphores_list, (ListItem*) sem);
	
	if(DEBUG) printf("Test 2\n");
	
	int ret;
	
	SemDescriptor_free(semDesc);
	SemDescriptorPtr_free(semDescPtr);

	
	if(sem->descriptors.size == 0 && sem->waiting_descriptors.size == 0){
		
		ret = Semaphore_free(sem);
		
		if(DEBUG) printf("Test 3\n");
		
		if(ret == 0){
			printf("[SEM_ERR] Semaphore_free failed!\n");
		}
	}
	
	//SemDescriptorPtr* semDescPtr = semDesc->ptr;
	
	
		
	List_detach(&running->descriptors, (ListItem*) semDescPtr);
	
	
	
	
	
	
	
	
	if(ret != 0){
		printf("[SEM_ERR] Semaphore_free failed! Error Code: %d - see PoolAllocatorResult for more details.\n", ret);
		running->syscall_retvalue = DSOS_ESEMCLOSE_FAILED;
		return;
	}
		
	
}
