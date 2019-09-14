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
	
	Semaphore* sem = semDesc->semaphore;
		
	if(!sem){
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
	
	SemDescriptorPtr_free(semDescPtr);
	SemDescriptor_free(semDesc);
		
	int ret;
	
	if(sem->descriptors.size == 0 && sem->waiting_descriptors.size == 0){
		
		List_detach(&semaphores_list, (ListItem*) sem);

		ret = Semaphore_free(sem);
				
		if(ret != 0x0){
			printf("[SEM_ERR] Semaphore_free failed!\n");
			running->syscall_retvalue = DSOS_ESEMCLOSE_FREE_ERR;
			return;
		}
		
		disastrOS_printStatus();

	}
	
	
	running->last_sem_fd--;

	running->syscall_retvalue = 0;
	
	printf("[SEM_INFO] Semaphore with fd %d has been correctly closed\n", fd);
	
	
	
}
