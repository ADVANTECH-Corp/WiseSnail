/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/03/16 by Zach Chih								    */
/* Modified Date: 2016/03/22 by Zach Chih									*/
/* Abstract     : Paho Carrier												*/
/* Reference    : None														*/
/****************************************************************************/
#ifndef _PAHO_CARRIER_H_
#define _PAHO_CARRIER_H_
#include "pthread.h"

typedef enum {
	NO_CONNECT=0,
	CONNECT,
	FIRST_LOST,
	FURTHER_LOST
}connection_state;

typedef struct{
   pthread_t thread;
   bool isThreadRunning;
}thread_context_t;

#endif