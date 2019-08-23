#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semOpen(){
	
	/**
		TO DO:
		* creates a semaphore in the system, having num semnum
		* the semaphore is accessible throughuot the entire system
		* by its id.
		* On success, the function call returns semnum (>=0);
		* in failure the function returns an error code <0
	**/ 
	
	// NOTE: Even if not required, a second parameter called 'count' 
	// was used to increare the flexibility of the semOpen.
	
	// Setting id and count given when calling semOpen function
	
	int id = running->syscall_args[0];
	int count = running->syscall_args[1];
	
	// Count represent the number of threads that can simultaneosly work
	// on the same resourse (entering in the critical section). Must be 
	// greater than zero.
	// If equal to (or lower than) zero, returning value is set to
	// DSOS_ESEMOPEN_WRONG_INIT.
	// Id has to be positive too.
	
	if(count <= 0 || id < 0){
		printf("[SEM_ERROR] - Error allocating semaphore (id #%d) due to "
		"configuration error: COUNT must be greater than zero and ID greater than (or equal to) zero \n", id);
		running->syscall_retvalue = DSOS_ESEMOPEN_WRONG_INIT;
		return;
	}
	
	// Each semaphore must have a unique id. To check that we use the 
	// following function
	// Semaphore* SemaphoreList_byId(SemaphoreList* l, int id)
	// which returns 0 if the semaphore with that specific id is not
	// presented in the SemaphoreList.
	// If not presented, we allocate it and check if everything worked.
	
	Semaphore* sem = SemaphoreList_byId(&semaphores_list, id);
	
	if(!sem){
		sem = Semaphore_alloc(id, count);
		if(!sem) {
			printf("[SEM_ERR] - Error while allocating semaphore id #%d\n", id);
			running->syscall_retvalue = DSOS_ESEMOPEN_ALLOC;
			return;
		}
	} else {
		printf("[SEM_ERR] - Semaphore allocation failed: the semaphore with the specified id already exists\n");
		running->syscall_retvalue = DSOS_ESEMOPEN_EXISTS;
		return;
		// HA SENSO FARE QUESTA OPERAZIONE? CONTROLLA BENE
	}
	
	// The semaphore has been created, so it can be added to the
	// SemaphoreList
	
	SemDescriptor* ins = (SemDescriptor*) List_insert(&semaphores_list, semaphores_list.last, (ListHead*) s);
	if(!ins) 
		printf("To complete!");
	} else 
		printf("[SEM_INFO] Semaphore id #%d has been correctly created. Remember: always drive with the safety belt! ;)\n", id);
	
	// We need to allocate the descriptor associated to the new semaphore
	// Inside the PCB of the running process we have an int variable called
	// last_sem_fd that provides the number of the last used semDescriptor.
	// We will use that number incremented by one.
	/// CONTROLLA FUNZIONAMENTO LAST_SEM_FD
	
	SemDescriptor* semDesc = SemDescriptor_alloc(++running->last_sem_fd, &sem, running)
	
	// Check if semDesc has been correctly allocated, and in case of
	// success add it to the sem_descriptor list.
	
	if(!semDesc) {
		printf("[SEM_ERR] Error while allocating SemDescriptor associated to Semaphore id #%d\n", id);
		syscall_retvalue = DSOS_ESEMOPEN_DESC_ALLOC;
		return;
	}
	
	List_insert(&running->sem_descriptors, &running->sem_descriptors.last, (ListItem*) semDesc);	
	
	// Finally a semDescriptorPtr must be allocated an included in the 
	// list of all descriptors contained in the PCB.
	
	SemDescriptorPtr* semDescPtr = SemDescriptorPtr_alloc(semDesc);
	
	if(!semDescPtr){
		printf("[SEM_ERR] Error while allocating SemDescriptorPtr of the SemaphoreDescriptor associated to the semaphore id #%d\n", id);
		syscall_retvalue = DSOS_ESEMOPEN_DESCPTR_ALLOC;
		return;
	}
	
	List_insert(&running->descriptors, &running->descriptors.last, (ListItem*) semDescPtr);
	
	// We have accomplished all requirements for the SemOpen. The retvalue
	// can now be set to the fd of the semaphore.
	/// DUBBI - CONTROLLA FOGLI
	
	running->syscall_retvalue = semDesc->fd;
	return;
	
}
