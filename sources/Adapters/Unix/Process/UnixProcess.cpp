
#include "UnixProcess.h"
#include "System/Console/Trace.h"
#include <stdio.h>
#include <fcntl.h>

void *_UnixStartThread(void *p) {
	SysThread *play=(SysThread *)p ;
	play->startExecution() ;
	return NULL ;
}

bool UnixProcessFactory::BeginThread(SysThread& thread) {
pthread_t pthread ;

	pthread_create(&pthread,0,_UnixStartThread,&thread) ;
	return true ;
}

SysSemaphore *UnixProcessFactory::CreateNewSemaphore(int initialcount, int maxcount) {
	return new UnixSysSemaphore(initialcount,maxcount) ;
} ;

UnixSysSemaphore::UnixSysSemaphore(int initialcount,int maxcount) {
  sem_init(&sem_, 0, initialcount);
} ;

UnixSysSemaphore::~UnixSysSemaphore() {
  sem_destroy(&sem_);
} ;

SysSemaphoreResult UnixSysSemaphore::Wait() {
	sem_wait(&sem_) ;
	return SSR_NO_ERROR ;
} ;

SysSemaphoreResult UnixSysSemaphore::TryWait() {
	return SSR_INVALID ;
}

SysSemaphoreResult UnixSysSemaphore::WaitTimeout(unsigned long timeout) {
	return SSR_INVALID ;
} ;

SysSemaphoreResult UnixSysSemaphore::Post() {
	sem_post(&sem_) ;
	return SSR_NO_ERROR ;
} ;
