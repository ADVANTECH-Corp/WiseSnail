#ifdef __linux__ 
#include <pthread.h>
#else
#define pthread_mutex_t int
#define pthread_mutex_init(x,y)
#define pthread_mutex_lock(x)
#define pthread_mutex_unlock(x)
#define pthread_mutex_destroy(x)
#endif
#include <stdio.h>
#include "WiseSnail.h"
#include "wiseagentlite.h"

static pthread_mutex_t *pmutex = NULL;
static pthread_mutex_t mutex;

void WiseSnail_Init(char *productionName, char *wanIp, unsigned char *parentMac, WiseSnail_InfoSpec *infospec, int count) {
	if(pmutex == NULL) {
		pthread_mutex_init(&mutex, NULL);
		pmutex = &mutex;
		WiseAgent_Init(productionName, wanIp, parentMac, (WiseAgentInfoSpec *)infospec, count);
	}
}

void WiseSnail_RegisterInterface(char *ifMac, char *ifName, int ifNumber, WiseSnail_InfoSpec *infospec, int count) {
	if(pmutex != NULL) {
		pthread_mutex_lock(&mutex);
		WiseAgent_RegisterInterface(ifMac, ifName, ifNumber, (WiseAgentInfoSpec *)infospec, count);
		pthread_mutex_unlock(&mutex);
	}
}

int WiseSnail_Connect(char *server_url, int port, char *username, char *password, WiseSnail_InfoSpec *infospec, int count) {
	int ret = 0;
	if(pmutex != NULL) {
		pthread_mutex_lock(&mutex);
		ret = WiseAgent_Open(server_url, port, username, password, (WiseAgentInfoSpec *)infospec, count);
		pthread_mutex_unlock(&mutex);
	}
	return ret;
}

void WiseSnail_RegisterSensor(char *deviceMac, char *defaultName, WiseSnail_InfoSpec *infospec, int count) {
	if(pmutex != NULL) {
		pthread_mutex_lock(&mutex);
		WiseAgent_RegisterSensor(deviceMac, defaultName, (WiseAgentInfoSpec *)infospec, count);
		pthread_mutex_unlock(&mutex);
	}
}

void WiseSnail_SenHubDisconnect(char *deviceMac) {
	if(pmutex != NULL) {
		pthread_mutex_lock(&mutex);
		WiseAgent_SenHubDisconnect(deviceMac);
		pthread_mutex_unlock(&mutex);
	}
}

void WiseSnail_SenHubReConnected(char *deviceMac) {
	if(pmutex != NULL) {
		pthread_mutex_lock(&mutex);
		WiseAgent_SenHubReConnected(deviceMac);
		pthread_mutex_unlock(&mutex);
	}
}


void WiseSnail_Update(char *deviceMac, WiseSnail_Data* data, int count) {
	if(pmutex != NULL) {
		pthread_mutex_lock(&mutex);
		WiseAgent_Write(deviceMac, (WiseAgentData*)data, count);
		pthread_mutex_unlock(&mutex);
	}
}

void WiseSnail_Get(char *deviceMac, char *name, WiseSnail_Data *data) {
	if(pmutex != NULL) {
		pthread_mutex_lock(&mutex);
		WiseAgent_Get(deviceMac, name, (WiseAgentData *)data);
		pthread_mutex_unlock(&mutex);
	}
}

void WiseSnail_Cmd_Handler(WiseSnail_SleepOneSecond sleepOneSec) {
	if(pmutex != NULL) {
		pthread_mutex_lock(&mutex);
		WiseAgent_Cmd_Handler();
		pthread_mutex_unlock(&mutex);
	}
	sleepOneSec();
}

void WiseSnail_Uninit() {
	if(pmutex != NULL) {
		pthread_mutex_lock(&mutex);
		WiseAgent_Close();
		pthread_mutex_unlock(&mutex);
		pthread_mutex_destroy(&mutex);
		pmutex = NULL;
	}
}
