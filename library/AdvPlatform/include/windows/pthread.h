/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/08/18 by Fred Chang									*/
/* Modified Date: 2015/08/18 by Fred Chang									*/
/* Abstract     :  					*/
/* Reference    : None														*/
/****************************************************************************/
#ifndef __pthread_h__
#define __pthread_h__
#ifdef __cplusplus
extern "C" {
#endif

#include "export.h"
#include <windows.h>

#define pthread_t DWORD
ADVPLAT_EXPORT int ADVPLAT_CALL pthread_create(pthread_t *thread, void *attr,void *(*start_routine) (void *), void *arg);
ADVPLAT_EXPORT int ADVPLAT_CALL pthread_join(pthread_t thread, void **retval);

//Mutex
typedef struct
{
	HANDLE m;
	CRITICAL_SECTION cs;
} pthread_mutex_t;

#define __SIZEOF_PTHREAD_MUTEXATTR_T 4
typedef union
{
  char __size[__SIZEOF_PTHREAD_MUTEXATTR_T];
  int __align;
} pthread_mutexattr_t;

ADVPLAT_EXPORT int ADVPLAT_CALL pthread_mutex_init(pthread_mutex_t * mutex, const pthread_mutexattr_t * attr);
ADVPLAT_EXPORT int ADVPLAT_CALL pthread_mutex_destroy(pthread_mutex_t *mutex);
ADVPLAT_EXPORT int ADVPLAT_CALL pthread_mutex_lock(pthread_mutex_t *mutex);
ADVPLAT_EXPORT int ADVPLAT_CALL pthread_mutex_trylock(pthread_mutex_t *mutex);
ADVPLAT_EXPORT int ADVPLAT_CALL pthread_mutex_unlock(pthread_mutex_t *mutex);

//Condition
#define pthread_cond_t PCONDITION_VARIABLE
#define pthread_condattr_t void
ADVPLAT_EXPORT int ADVPLAT_CALL pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *cattr);
#define pthread_cond_destroy(x) do { 0; } while(0)

ADVPLAT_EXPORT int ADVPLAT_CALL pthread_cond_broadcast(pthread_cond_t *cond);
ADVPLAT_EXPORT int ADVPLAT_CALL pthread_cond_signal(pthread_cond_t *cond);
ADVPLAT_EXPORT int ADVPLAT_CALL pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);
ADVPLAT_EXPORT int ADVPLAT_CALL pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);


#ifdef __cplusplus
}
#endif

#endif //__pthread_h__