#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

#define DEBUG 1

void internal_semClose(){
	
	/**
		TO DO:
		* releases from an application the given semaphore
		* returns 0 on success
		* returns an error code if the semaphore is not owned by the 
		* application
	**/ 
	
	// Get fd and use it to obtain the related semDescriptor 
	// (with error handling)
	
	int fd = running->syscall_args[0];
	
	SemDescriptor* semDesc = SemDescriptorList_byFd(&running->sem_descriptors, fd);
	
	if(!semDesc){
		printf("[SEM_ERR] Trying to close a non-existing semaphore!\n");
		running->syscall_retvalue = DSOS_ESEMCLOSE_NOT_EXIST;
		return;
	}
	
	// We don't need this semaphore anymore in the running proccess, so
	// the semDescriptor is removed by the list
	
	List_detach(&running->sem_descriptors, (ListItem *)semDesc);
	
	// Get the semaphore from the Descriptor
	
	Semaphore* sem = semDesc->semaphore;
		
	if(!sem){
		printf("[SEM_ERR] I wasn't able to get the semaphore to destroy!\n");
		running->syscall_retvalue = DSOS_ESEMCLOSE_NOT_FOUND;
		return;
	}
	
	// Since the descriptor has been removed, the same must be done with 
	// the pointer to the descriptor that is contained in the descriptor
	// list of semaphore
	
	SemDescriptorPtr* semDescPtr = (SemDescriptorPtr*)List_detach(&sem->descriptors, (ListItem*)(semDesc->ptr));
					
	if(!semDescPtr){
		printf("[SEM_ERR] I wasn't able to get the semDescPtr!\n");
		running->syscall_retvalue = DSOS_ESEMCLOSE_NOT_FOUND;
		return;
	}
	
	// The last step is to deallocate the structs of the descriptor 
	// and its pointer
	
	SemDescriptorPtr_free(semDescPtr);
	SemDescriptor_free(semDesc);
	
	// We have to lists in the semaphore: one of all active Descriptors 
	// (Ptrs), one of those associated to process waiting to gain access
	// to the critical section.
	// So, we need to completely deallocate the semaphore only if
	// no other processes are using it.
		
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
	
	// Decrement last_sem_fd and return 0
	
	running->last_sem_fd--;

	running->syscall_retvalue = 0;
	
	printf("[SEM_INFO] Semaphore with fd %d has been correctly closed\n", fd);
	
	return;
	
}
