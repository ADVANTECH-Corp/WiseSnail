#include "pthread.h"
#include "time.h"

int ADVPLAT_CALL pthread_create(pthread_t *thread, void *attr,void *(*start_routine) (void *), void *arg) {
	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)start_routine,arg,0,thread);
	return 0;
}

int ADVPLAT_CALL pthread_join(pthread_t thread, void **retval) {
	DWORD exitCode;
	HANDLE h = OpenThread(THREAD_QUERY_INFORMATION, FALSE, thread);
	WaitForSingleObject(h, INFINITE);
	GetExitCodeThread(h, &exitCode);
	CloseHandle(h);
	*retval = (void *)exitCode;
	return 0;
}

int ADVPLAT_CALL pthread_mutex_init(pthread_mutex_t * mutex, const pthread_mutexattr_t * attr) {
	mutex->m=CreateMutex(NULL,FALSE,NULL);
	InitializeCriticalSection(&mutex->cs);
	return 0;
}

int ADVPLAT_CALL pthread_mutex_destroy(pthread_mutex_t *mutex) {
	CloseHandle(mutex->m);
	return 0;
}

int ADVPLAT_CALL pthread_mutex_lock(pthread_mutex_t *mutex) {
	WaitForSingleObject(mutex->m, INFINITE);
	return 0;
}

int ADVPLAT_CALL pthread_mutex_trylock(pthread_mutex_t *mutex) {
	if (WaitForSingleObject(mutex->m, 0) == WAIT_TIMEOUT) 
		return EBUSY; 
	else 
		return 0;
}

int ADVPLAT_CALL pthread_mutex_unlock(pthread_mutex_t *mutex) {
	ReleaseMutex(mutex->m); 
	LeaveCriticalSection(&mutex->cs);
	return 0;
}


int ADVPLAT_CALL pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *cattr) {
	InitializeConditionVariable(*cond);
	return 0;
}

int ADVPLAT_CALL pthread_cond_broadcast(pthread_cond_t *cond) {
	WakeAllConditionVariable(*cond);
	return 0;
}

int ADVPLAT_CALL pthread_cond_signal(pthread_cond_t *cond) {
	WakeConditionVariable(*cond);
	return 0;
}

int ADVPLAT_CALL pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime) {
	DWORD ms = (DWORD)(abstime->tv_sec - time(NULL));
	ms *= 1000;
	ms += abstime->tv_nsec / 1000000;
	if (SleepConditionVariableCS(*cond, &mutex->cs, ms) == ETIMEDOUT) {
		return WAIT_TIMEOUT;
	}
	else {
		return 0;
	}

}

int ADVPLAT_CALL pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
	DWORD ms = 0;
	SleepConditionVariableCS(*cond, &mutex->cs, ms);
	return 0;
}
