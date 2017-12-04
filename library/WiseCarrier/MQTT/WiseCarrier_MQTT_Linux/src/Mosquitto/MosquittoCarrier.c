/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/03/21 by Scott68 Chang							    */
/* Modified Date: 2016/03/21 by Scott68 Chang								*/
/* Abstract     : Mosquitto Carrier API definition						    */
/* Reference    : None														*/
/****************************************************************************/
//#include "network.h"
#include <stdio.h>
#include <string.h>
#include "mosquitto.h"
#include "susiaccess_def.h"
#include "WiseCarrier_MQTT.h"
#include "topic.h"
#include <pthread.h>
#include <sys/time.h>

pthread_mutex_t g_connect_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *g_connect_mutex_p = &g_connect_mutex;

static char g_version[32] = {0};

typedef struct{
	struct mosquitto *mosq;

	/*Connection Info*/
	char strClientID[DEF_DEVID_LENGTH];
	char strServerIP[DEF_MAX_STRING_LENGTH];
	int iServerPort;
	char strAuthID[DEF_USER_PASS_LENGTH];
	char strAuthPW[DEF_USER_PASS_LENGTH];
	int iKeepalive;

	char strWillTopic[DEF_MAX_PATH];
	char* pWillPayload;
	int iWillLen;

	tls_type tlstype;
	/*TLS*/
	char strCaFile[DEF_MAX_PATH];
	char strCaPath[DEF_MAX_PATH];
	char strCertFile[DEF_MAX_PATH];
	char strKeyFile[DEF_MAX_PATH];
	char strCerPasswd[DEF_USER_PASS_LENGTH];
	/*pre-shared-key*/
	char strPsk[DEF_USER_PASS_LENGTH];
	char strIdentity[DEF_USER_PASS_LENGTH];
	char strCiphers[DEF_MAX_CIPHER];

	void *pUserData;

	WICAR_CONNECT_CB on_connect_cb;
	WICAR_DISCONNECT_CB on_disconnect_cb;
	WICAR_LOSTCONNECT_CB on_lostconnect_cb;
	topic_entry_st* pSubscribeTopics;
}mosq_car_t;


typedef enum{
	mc_err_success = 0,
	mc_conn_accept = 400,
	mc_err_invalid_param = 500,
	mc_err_no_init,
	mc_err_init_fail,
	mc_err_already_init,
	mc_err_no_connect,
	mc_err_malloc_fail,
	mc_err_not_support,
}mc_err_code;

mc_err_code g_iErrorCode = mc_err_success;

mosq_car_t* g_mosq = NULL;
#pragma region MQTT MESSAGE QUEUE

struct mqttmsg
{
	int mid;
	pthread_mutex_t waitlock;
	pthread_cond_t waitresponse;
	struct mqttmsg* next;
};

struct msg_queue {
	struct mqttmsg* root;
	int size;
};

struct msg_queue g_msg_queue;

struct mqttmsg* MQTT_LastMsgQueue()
{
	struct mqttmsg* msg = g_msg_queue.root;
	struct mqttmsg* target = NULL;
	if(msg == NULL)
		return target;
	else
		target = msg;

	while(target->next)
	{
		target = target->next;
	}
	return target;	
}

void MQTT_InitMsgQueue()
{
	memset(&g_msg_queue, 0, sizeof(struct msg_queue));
}

void MQTT_UninitMsgQueue()
{
	struct mqttmsg* msg = g_msg_queue.root;
	struct mqttmsg* target = NULL;
	while(msg)
	{
		target = msg;
		msg = msg->next;
		pthread_mutex_destroy(&target->waitlock);
		pthread_cond_destroy(&target->waitresponse);
		free(target);
		g_msg_queue.size--;
	}
}

struct mqttmsg* MQTT_AddMsgQueue(int mid)
{
	struct mqttmsg* msg = NULL;
	struct mqttmsg* newmsg = malloc(sizeof(struct mqttmsg));
	memset(newmsg, 0, sizeof(struct mqttmsg));
	newmsg->mid = mid;
	pthread_mutex_init(&newmsg->waitlock, NULL);
	pthread_cond_init(&newmsg->waitresponse, NULL);

	msg = MQTT_LastMsgQueue();
	if(msg==NULL)
	{
		g_msg_queue.size = 1;
		g_msg_queue.root = newmsg;
	}
	else
	{
		g_msg_queue.size++;
		msg->next = newmsg;
	}
	return newmsg;
}

struct mqttmsg* MQTT_FindMsgQueue(int mid)
{
	struct mqttmsg* msg = g_msg_queue.root;
	struct mqttmsg* target = NULL;

	while(msg)
	{
		if(msg->mid == mid)
		{
			target = msg;
			break;
		}
		msg = msg->next;
	}
	return target;
}

void MQTT_FreeMsgQueue(int mid)
{
	struct mqttmsg* msg = g_msg_queue.root;
	struct mqttmsg* target = NULL;
	if(msg == NULL)
		return;
	if(msg->mid == mid)
	{
		g_msg_queue.root = msg->next;
		g_msg_queue.size--;
		target = msg;
		goto FREE_MSG;
	}

	while(msg->next)
	{
		if(msg->next->mid == mid)
		{
			target = msg->next;
			msg->next = target->next;
			g_msg_queue.size--;
			break;
		}
		msg = msg->next;
	}
FREE_MSG:
	if(target == NULL) return;
	pthread_mutex_destroy(&msg->waitlock);
	pthread_cond_destroy(&msg->waitresponse);
	free(msg);
}

void MQTT_publish_callback(struct mosquitto *mosq, void *obj, int mid)
{	
	struct mqttmsg* msg = MQTT_FindMsgQueue(mid);
	if(msg == NULL) return;
	pthread_cond_signal(&msg->waitresponse);
}

#pragma endregion MQTT MESSAGE QUEUE

void on_connect_callback(struct mosquitto *mosq, void *userdata, int rc)
{
	if(!g_mosq)
		return;
	g_iErrorCode = mc_conn_accept + rc;
	if(rc == 0)
	{
		if(g_mosq->on_connect_cb)
			g_mosq->on_connect_cb(userdata);
		
		if(g_connect_mutex_p != NULL) {
			pthread_mutex_unlock(g_connect_mutex_p);
			g_connect_mutex_p = NULL;
		}
	}
	else
	{
		mosquitto_reconnect_async(mosq);
		if(g_mosq->on_lostconnect_cb)
			g_mosq->on_lostconnect_cb(userdata);
	}
}

void on_disconnect_callback(struct mosquitto *mosq, void *userdata, int rc)
{
	if(!g_mosq)
		return;
	g_iErrorCode = mc_conn_accept + rc;
	if(rc == 0)
	{
		if(g_mosq->on_disconnect_cb)
			g_mosq->on_disconnect_cb(userdata);
	}
	else
	{
		mosquitto_reconnect_async(mosq);
		if(g_mosq->on_lostconnect_cb)
			g_mosq->on_lostconnect_cb(userdata);
	}
}

void on_message_recv_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{	
	struct topic_entry * target = NULL;
	if(!g_mosq)
		return;

	target = topic_find(g_mosq->pSubscribeTopics, msg->topic);
	
	if(target != NULL)
	{
		if(target->callback_func)
			((WICAR_MESSAGE_CB)target->callback_func)(msg->topic, msg->payload, msg->payloadlen, userdata);
	}
}

int on_password_check(char *buf, int size, int rwflag, void *userdata)
{
	int length = 0;

	if(!buf)
		return 0;

	if(!g_mosq)
		return 0;

	length = strlen(g_mosq->strCerPasswd);

	memset(buf, 0, size);
	if(length+1 >= size)
	{
		strncpy(buf,g_mosq->strCerPasswd,size);
		return size;
	}
	else
	{
		strncpy(buf, g_mosq->strCerPasswd, length+1);
		return length;
	}
}

struct mosquitto * _initialize(char const * devid, void* userdata)
{
	struct mosquitto *mosq = NULL;

	mosquitto_lib_init();
	mosq = mosquitto_new(devid, true, userdata);
	if (!mosq)
	{
		g_iErrorCode = mc_err_init_fail;
		return NULL;
	}

	mosquitto_connect_callback_set(mosq, on_connect_callback);
	mosquitto_disconnect_callback_set(mosq, on_disconnect_callback);
	mosquitto_message_callback_set(mosq, on_message_recv_callback);

	MQTT_InitMsgQueue();
	mosquitto_publish_callback_set(mosq, MQTT_publish_callback);

	return mosq;
}

void _uninitialize(struct mosquitto *mosq)
{
	if(mosq == NULL)
		return;

	mosquitto_connect_callback_set(mosq, NULL);
	mosquitto_disconnect_callback_set(mosq, NULL);
	mosquitto_message_callback_set(mosq, NULL);

	mosquitto_publish_callback_set(mosq, NULL);
	MQTT_UninitMsgQueue();

	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}

bool _tls_set(mosq_car_t *pmosq)
{
	int result = MOSQ_ERR_SUCCESS;
	if(pmosq == NULL)
	{
		g_iErrorCode = mc_err_no_init;
		return false;
	}

	mosquitto_tls_insecure_set(pmosq->mosq, true);
	result = mosquitto_tls_set(pmosq->mosq, strlen(pmosq->strCaFile)>0?pmosq->strCaFile:NULL, strlen(pmosq->strCaPath)>0?pmosq->strCaPath:NULL, strlen(pmosq->strCertFile)>0?pmosq->strCertFile:NULL, strlen(pmosq->strKeyFile)>0?pmosq->strKeyFile:NULL, on_password_check);
	g_iErrorCode = result;
	return result==MOSQ_ERR_SUCCESS?true:false;
}

bool _psk_set(mosq_car_t *pmosq)
{
	int result = MOSQ_ERR_SUCCESS;
	if(pmosq == NULL)
	{
		g_iErrorCode = mc_err_no_init;
		return false;
	}
	result = mosquitto_tls_psk_set(pmosq->mosq, strlen(pmosq->strPsk)>0?pmosq->strPsk:NULL, strlen(pmosq->strIdentity)>0?pmosq->strIdentity:NULL, strlen(pmosq->strCiphers)>0?pmosq->strCiphers:NULL);
	g_iErrorCode = result;
	return result==MOSQ_ERR_SUCCESS?true:false;
}

bool _connect(mosq_car_t *pmosq)
{
	int result = MOSQ_ERR_SUCCESS;
	if(pmosq == NULL)
	{
		g_iErrorCode = mc_err_no_init;
		return false;
	}

	if( strlen(pmosq->strAuthID)>0 && strlen(pmosq->strAuthPW)>0)
		mosquitto_username_pw_set(pmosq->mosq,pmosq->strAuthID,pmosq->strAuthPW);

	mosquitto_will_clear(pmosq->mosq);

	if(pmosq->pWillPayload != NULL) {
		mosquitto_will_set(pmosq->mosq, pmosq->strWillTopic, pmosq->iWillLen , pmosq->pWillPayload, 0, false);
	}
	mosquitto_loop_start(pmosq->mosq); /*Must call before mosquitto_connect_async*/

	if(g_connect_mutex_p != NULL) {
		pthread_mutex_lock(g_connect_mutex_p);
	}

	result = mosquitto_connect(pmosq->mosq, pmosq->strServerIP, pmosq->iServerPort, pmosq->iKeepalive);
	g_iErrorCode = result;
	
	if(g_connect_mutex_p != NULL) {
		pthread_mutex_lock(g_connect_mutex_p);
	}

	return result==MOSQ_ERR_SUCCESS?true:false;
}

bool _reconnect(mosq_car_t *pmosq)
{
	int result = MOSQ_ERR_SUCCESS;
	if(pmosq == NULL)
	{
		g_iErrorCode = mc_err_no_init;
		return false;
	}

	result = mosquitto_reconnect(pmosq->mosq);
	g_iErrorCode = result;

	return result==MOSQ_ERR_SUCCESS?true:false;
}

bool _disconnect(mosq_car_t *pmosq, bool bForce)
{
	int result = MOSQ_ERR_SUCCESS;
	if(pmosq == NULL)
	{
		g_iErrorCode = mc_err_no_init;
		return false;
	}
	if(!bForce)
		mosquitto_loop(pmosq->mosq, 0, 1);	
	result = mosquitto_disconnect(pmosq->mosq);
	mosquitto_loop_stop(pmosq->mosq, bForce); //disable force for linux.

	g_iErrorCode = mc_err_success;
	return result==MOSQ_ERR_SUCCESS?true:false;
}


WISE_CARRIER_API const char * WiCar_MQTT_LibraryTag()
{
	int major=0, minor=0, revision=0;
	mosquitto_lib_version(&major, &minor, &revision);
	sprintf(g_version, "mosquitto_%d.%d.%d", major, minor, revision);
	return g_version;
}

WISE_CARRIER_API bool WiCar_MQTT_Init(WICAR_CONNECT_CB on_connect, WICAR_DISCONNECT_CB on_disconnect, void *userdata)
{
	if(g_mosq)
	{
		g_iErrorCode = mc_err_already_init;
		return false;
	}
	g_mosq = malloc(sizeof(mosq_car_t));
	if(g_mosq == NULL){
		g_iErrorCode = mc_err_malloc_fail;
		return false;
	}

	memset(g_mosq, 0, sizeof(mosq_car_t));

	g_mosq->on_connect_cb = on_connect;
	g_mosq->on_disconnect_cb = on_disconnect;
	g_mosq->pUserData = userdata;
	g_mosq->iKeepalive = 10; //default 10 sec.
	g_iErrorCode = mc_err_success;
	return true;

}

WISE_CARRIER_API void WiCar_MQTT_Uninit()
{
	if(g_mosq)
	{
		struct topic_entry *iter_topic = NULL;
		struct topic_entry *tmp_topic = NULL;

		iter_topic = topic_first(g_mosq->pSubscribeTopics);
		while(iter_topic != NULL)
		{
			tmp_topic = iter_topic->next;
			topic_remove(&g_mosq->pSubscribeTopics, iter_topic->name);
			iter_topic = tmp_topic;
		}
		//g_mosq->pSubscribeTopics = NULL;
		if(g_mosq->mosq)
			_uninitialize(g_mosq->mosq);

		if(g_mosq->pWillPayload!=NULL)
			free(g_mosq->pWillPayload);
		g_mosq->pWillPayload = NULL;
		free(g_mosq);
	}
	g_mosq = NULL;

}

WISE_CARRIER_API bool WiCar_MQTT_SetWillMsg(const char* topic, const void *msg, int msglen)
{
	if(!g_mosq){
		g_iErrorCode = mc_err_no_init;
		return false;
	}

	strncpy(g_mosq->strWillTopic, topic, sizeof(g_mosq->strWillTopic));
	if(g_mosq->pWillPayload)
		free(g_mosq->pWillPayload);
	g_mosq->pWillPayload = malloc(msglen+1);
	if(g_mosq->pWillPayload)
	{
		memset(g_mosq->pWillPayload, 0, msglen+1);
		strcpy(g_mosq->pWillPayload, msg);
		g_mosq->iWillLen = msglen;
	}
	else{
		g_iErrorCode = mc_err_malloc_fail;
		return false;
	}
	g_iErrorCode = mc_err_success;
	return true;

}

WISE_CARRIER_API bool WiCar_MQTT_SetAuth(char const * username, char const * password)
{
	if(!g_mosq){
		g_iErrorCode = mc_err_no_init;
		return false;
	}

	strncpy(g_mosq->strAuthID, username, sizeof(g_mosq->strAuthID));
	strncpy(g_mosq->strAuthPW, password, sizeof(g_mosq->strAuthPW));
	g_iErrorCode = mc_err_success;
	return true;
}

WISE_CARRIER_API bool WiCar_MQTT_SetKeepLive(int keepalive)
{
	if(!g_mosq){
		g_iErrorCode = mc_err_no_init;
		return false;
	}

	g_mosq->iKeepalive = keepalive;
	g_iErrorCode = mc_err_success;
	return true;
}

WISE_CARRIER_API bool WiCar_MQTT_SetTls(const char *cafile, const char *capath, const char *certfile, const char *keyfile, const char* password)
{
	if(!g_mosq){
		g_iErrorCode = mc_err_no_init;
		return false;
	}
	g_mosq->tlstype = tls_type_tls;
	if(cafile)
		strncpy(g_mosq->strCaFile, cafile, sizeof(g_mosq->strCaFile));
	if(capath)
		strncpy(g_mosq->strCaPath, capath, sizeof(g_mosq->strCaPath));
	if(certfile)
		strncpy(g_mosq->strCertFile, certfile, sizeof(g_mosq->strCertFile));
	if(keyfile)
		strncpy(g_mosq->strKeyFile, keyfile, sizeof(g_mosq->strKeyFile));
	if(password)
		strncpy(g_mosq->strCerPasswd, password, sizeof(g_mosq->strCerPasswd));
	g_iErrorCode = mc_err_success;
	return true;
}

WISE_CARRIER_API bool WiCar_MQTT_SetTlsPsk(const char *psk, const char *identity, const char *ciphers)
{
	if(!g_mosq){
		g_iErrorCode = mc_err_no_init;
		return false;
	}
	g_mosq->tlstype = tls_type_psk;
	if(psk)
		strncpy(g_mosq->strPsk, psk, sizeof(g_mosq->strPsk));
	if(identity)
		strncpy(g_mosq->strIdentity, identity, sizeof(g_mosq->strIdentity));
	if(ciphers)
		strncpy(g_mosq->strCiphers, ciphers, sizeof(g_mosq->strCiphers));
	g_iErrorCode = mc_err_success;
	return true;
}

WISE_CARRIER_API bool WiCar_MQTT_Connect(const char* address, int port, const char* clientId, WICAR_LOSTCONNECT_CB on_lostconnect)
{
	struct mosquitto *mosq = NULL;
	if(!g_mosq){
		g_iErrorCode = mc_err_no_init;
		return false;
	}
	if(g_mosq->mosq)
	{
		_uninitialize(g_mosq->mosq);
		g_mosq->mosq = NULL;
	}
	
	mosq = _initialize(clientId, g_mosq);
	if (!mosq)
	{
		g_iErrorCode = mc_err_init_fail;
		return false;
	}
	g_mosq->mosq = mosq;

	switch (g_mosq->tlstype)
	{
	case tls_type_none:
		break;
	case tls_type_tls:
		if(!_tls_set(g_mosq)) return false;
		break;
	case tls_type_psk:
		if(!_psk_set(g_mosq)) return false;
		break;
	}

	strncpy(g_mosq->strServerIP, address, sizeof(g_mosq->strServerIP));
	g_mosq->iServerPort = port;
	strncpy(g_mosq->strClientID, clientId, sizeof(g_mosq->strClientID));
	g_mosq->on_lostconnect_cb = on_lostconnect;
	return _connect(g_mosq);

}

WISE_CARRIER_API bool WiCar_MQTT_Reconnect()
{
	bool bRet = false;
	if(!g_mosq){
		g_iErrorCode = mc_err_no_init;
		return false;
	}
	bRet = _reconnect(g_mosq);
	if(bRet)
	{
		struct topic_entry *iter_topic = NULL;

		iter_topic = topic_first(g_mosq->pSubscribeTopics);
		while(iter_topic != NULL)
		{
			mosquitto_subscribe(g_mosq->mosq, NULL, iter_topic->name, 0);
			iter_topic = iter_topic->next;
		}
	}
	return bRet;
}

WISE_CARRIER_API bool WiCar_MQTT_Disconnect(int force)
{
	if(!g_mosq){
		g_iErrorCode = mc_err_no_init;
		return false;
	}

	if(_disconnect(g_mosq, force>0?true:false))
	{
		struct topic_entry *iter_topic = NULL;
		struct topic_entry *tmp_topic = NULL;

		iter_topic = topic_first(g_mosq->pSubscribeTopics);
		while(iter_topic != NULL)
		{
			tmp_topic = iter_topic->next;
			topic_remove(&g_mosq->pSubscribeTopics, iter_topic->name);
			iter_topic = tmp_topic;
		}
		//g_mosq->pSubscribeTopics = NULL;
		_uninitialize(g_mosq->mosq);
		g_mosq->mosq = NULL;
		g_iErrorCode = mc_err_success;
		return true;
	}
	else
		return false;
}

WISE_CARRIER_API bool WiCar_MQTT_Publish(const char* topic, const void *msg, int msglen, int retain, int qos)
{
	int result = MOSQ_ERR_SUCCESS;
	struct mqttmsg* mqttmsg = NULL;
	int mid = 0;
	if(!g_mosq){
		g_iErrorCode = mc_err_no_init;
		return false;
	}

	result = mosquitto_publish(g_mosq->mosq, &mid, topic, msglen, msg, qos, retain>0?true:false);
	g_iErrorCode = result;
	if(result == MOSQ_ERR_SUCCESS)
	{
		if(qos>0)
			mqttmsg = MQTT_AddMsgQueue(mid);
	}

	if(mqttmsg)
	{
		struct timespec time;
		struct timeval tv;
		int ret = 0;
		gettimeofday(&tv, NULL);
		time.tv_sec = tv.tv_sec + 3;
		time.tv_nsec = tv.tv_usec;
		ret = pthread_cond_timedwait(&mqttmsg->waitresponse, &mqttmsg->waitlock, &time);
		if(ret != 0)
		{
			printf("Publish wait mid:%d timeout", mid);
			result = MOSQ_ERR_INVAL;
		}
		MQTT_FreeMsgQueue(mid);
	}

	return result==MOSQ_ERR_SUCCESS?true:false;
}

WISE_CARRIER_API bool WiCar_MQTT_Subscribe(const char* topic, int qos, WICAR_MESSAGE_CB on_recieve)
{
	int result = MOSQ_ERR_SUCCESS;
	topic_entry_st *ptopic = NULL;

	if(!g_mosq){
		g_iErrorCode = mc_err_no_init;
		return false;
	}
	ptopic = topic_find(g_mosq->pSubscribeTopics, topic);

	if(ptopic == NULL)
	{
		result = mosquitto_subscribe(g_mosq->mosq, NULL, topic, 0);
		if(result==MOSQ_ERR_SUCCESS)
		{
			ptopic = topic_add(&g_mosq->pSubscribeTopics, topic, (void*)on_recieve);
			if(ptopic == NULL)
			{
				mosquitto_unsubscribe(g_mosq->mosq, NULL, topic);
				g_iErrorCode = mc_err_malloc_fail;
				return false;
			}
		}
	}
	else
		ptopic->callback_func = on_recieve;

	g_iErrorCode = result;
	return result==MOSQ_ERR_SUCCESS?true:false;
}

WISE_CARRIER_API bool WiCar_MQTT_UnSubscribe(const char* topic)
{
	int result = MOSQ_ERR_SUCCESS;
	topic_entry_st *ptopic = NULL;

	if(!g_mosq){
		g_iErrorCode = mc_err_no_init;
		return false;
	}

	result = mosquitto_unsubscribe(g_mosq->mosq, NULL, topic);

	topic_remove(&g_mosq->pSubscribeTopics, (char*)topic);

	g_iErrorCode = result;
	return result==MOSQ_ERR_SUCCESS?true:false;
}

WISE_CARRIER_API bool WiCar_MQTT_GetLocalIP(const char *address)
{
	/*int sockfd = -1;
	char strAddr[16] = {0};
	if(g_mosq == NULL)
	{
		g_iErrorCode = mc_err_no_init;
		return false;
	}
	sockfd = mosquitto_socket(g_mosq->mosq);

	if(network_local_ip_get(sockfd, strAddr, sizeof(strAddr))==0)
	{
		g_iErrorCode = mc_err_success;
		strncpy(address, strAddr, sizeof(strAddr));
		return true;
	}
	else
	{
		g_iErrorCode = mc_err_no_connect;
		return false; 
	}*/
	// No Support
	return false;
}

WISE_CARRIER_API const char *WiCar_MQTT_GetCurrentErrorString()
{
	switch(g_iErrorCode){
		case mc_err_success:
			return "No error.";
		case mc_err_invalid_param:
			return "Invalided parameters.";
		case mc_err_no_init:
			return "No initialized.";
		case mc_err_init_fail:
			return "Initialize failed";
		case mc_err_already_init:
			return "Already initialized.";
		case mc_err_no_connect:
			return "No connected.";
		case mc_err_malloc_fail:
			return "Memory allocate failed";
		case mc_err_not_support:
			return "Not support.";
		default:
			{
				if( g_iErrorCode > mc_conn_accept &&  g_iErrorCode < mc_err_invalid_param)
				{
					int rc = g_iErrorCode - mc_conn_accept;
					return mosquitto_connack_string(rc);
				}
				else
					return mosquitto_strerror(g_iErrorCode);
			}
	}
}
