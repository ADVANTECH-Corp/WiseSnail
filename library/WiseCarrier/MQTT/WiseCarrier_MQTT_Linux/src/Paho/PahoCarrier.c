/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/03/16 by Zach Chih								    */
/* Modified Date: 2016/03/22 by Zach Chih									*/
/* Abstract     : Paho Carrier												*/
/* Reference    : None														*/
/****************************************************************************/
#include "WiseCarrier_MQTT.h"
#include "PahoCarrier.h"
#include "MQTTClient.h"
#include <stdlib.h>
#include "pthread.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>


static WICAR_CONNECT_CB		g_connect_cb=NULL;
static WICAR_DISCONNECT_CB		g_disconnect_cb=NULL;
static WICAR_LOSTCONNECT_CB	g_lostconnect_cb=NULL;	
static WICAR_MESSAGE_CB		g_message_cb=NULL;

MQTTClient client=NULL;
MQTTClient_nameValue* version;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_willOptions will_opts = MQTTClient_willOptions_initializer; 
MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;

MQTTClient_message pubmsg = MQTTClient_message_initializer;
MQTTClient_deliveryToken token;

bool ssl_flag=false;
int irc=0;
void *g_userdata;
char g_strPassword[128] = {0};
char IP[128];

static thread_context_t g_ReconnectContex;
connection_state conn_state;


void lostconnect_callback(void *context, char *cause)
{
	printf("lostconnect_callback...\n");
	conn_state=FURTHER_LOST;

	if(g_lostconnect_cb != NULL)
		g_lostconnect_cb(g_userdata);
}

int message_callback(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	/* if 0 (false) is returned by the callback then it failed, so we don't remove the message from
	 * the queue, and it will be retried later.  If 1 is returned then the message data may have been freed,
	 * so we must be careful how we use it.
	 */
	char *payload;
	printf("message_callback...\n");

	payload=(char *)calloc(1,message->payloadlen+1);
	printf("Message arrived\n");
    printf("     topic: %s\n", topicName);

	strncpy(payload, message->payload, message->payloadlen);
	printf("   message: %s\n",payload);

	if(g_message_cb != NULL)
		g_message_cb(topicName, payload, message->payloadlen, g_userdata);

	MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
	free(payload);

	return 1;

}

void delivered_callback(void *context, MQTTClient_deliveryToken dt)
{
	printf("delivered_callback...\n");
	printf("Message delivery confirmed\n");
}

void connect_callback()
{
	printf("connect_callback...\n");
	if(g_connect_cb!=NULL)
		g_connect_cb(g_userdata);
	
}

void disconnect_callback()
{
	printf("disconnect_callback...\n");
	if(g_disconnect_cb != NULL)
		g_disconnect_cb(g_userdata);
}



static void* ReconnectThreadStart(void *args)
{	

	thread_context_t *pthreadContex = (thread_context_t *)args;
	printf("ReconnectThreadStart...\n");
	while(true)
	{
		if(pthreadContex->isThreadRunning)
		{
			if(conn_state==FIRST_LOST||conn_state==FURTHER_LOST)
			{
				if(!MQTTClient_isConnected(client))
				{	printf("!MQTTClient_isConnected(client)...\n");
					if ((irc=MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
					{
						conn_state=FURTHER_LOST;
                        MQTTClient_disconnect(client, 0);
						printf("Failed to reconnect, return code %d\n", irc);    
					}
					else
					{	
						printf("reconnect successful!!\n");
						conn_state=CONNECT;
						connect_callback();					
					}
				}
			}
		}
		sleep(3);
	}
}
WISE_CARRIER_API const char* WiCar_MQTT_LibraryTag()
{
	version=MQTTClient_getVersionInfo();
	return version->value;

}
WISE_CARRIER_API bool WiCar_MQTT_Init(WICAR_CONNECT_CB on_connect, WICAR_DISCONNECT_CB on_disconnect, void *userdata)
{
	printf("WiCar_MQTT_Init...\n");

	conn_opts.cleansession = 1;
	conn_opts.MQTTVersion=4;	//fix timeout reconnection
	g_connect_cb=on_connect;
	g_disconnect_cb=on_disconnect;
	g_userdata=userdata;

	conn_state = NO_CONNECT;
	g_ReconnectContex.isThreadRunning = true;
	if(pthread_create(&g_ReconnectContex.thread,NULL, ReconnectThreadStart, &g_ReconnectContex) != 0)
	{
		g_ReconnectContex.isThreadRunning = false;
		printf("> start Reconnect thread failed!\r\n");	
    }

	return true;
}

WISE_CARRIER_API void WiCar_MQTT_Uninit()
{
	printf("WiCar_MQTT_Uninit...\n");
	if(client!=NULL)
	{	printf("MQTTClient_destroy\n");
		MQTTClient_destroy(&client);
	}
}

WISE_CARRIER_API bool WiCar_MQTT_SetWillMsg(const char* topic, const void *msg, int msglen)
{
	conn_opts.will=&will_opts;
	conn_opts.will->struct_version=0;
	conn_opts.will->retained=0;
	conn_opts.will->qos=2;
	conn_opts.will->topicName=topic;
	conn_opts.will->message=msg;
	return true;
}

WISE_CARRIER_API bool WiCar_MQTT_SetAuth(char const * username, char const * password)
{

	conn_opts.username=username;
	conn_opts.password=password;
	return true;

}

WISE_CARRIER_API bool WiCar_MQTT_SetKeepLive(int keepalive)
{

	conn_opts.keepAliveInterval=keepalive;
	return true;

}

WISE_CARRIER_API bool WiCar_MQTT_SetTls(const char *cafile, const char *capath, const char *certfile, const char *keyfile, const char* password)
{
	ssl_flag=true;
	conn_opts.ssl=&ssl_opts;
	conn_opts.ssl->struct_version=0;
	/*conn_opts.ssl->trustStore="server.crt";
	conn_opts.ssl->keyStore="ca.crt";
	conn_opts.ssl->privateKey="ca.key";
	conn_opts.ssl->privateKeyPassword="05155853";*/
	conn_opts.ssl->trustStore="ca.crt";
	conn_opts.ssl->keyStore="client.crt";
	conn_opts.ssl->privateKey="client.key";
	conn_opts.ssl->privateKeyPassword="05155853";
	//conn_opts.ssl->privateKeyPassword="123456";
	return true;
}

WISE_CARRIER_API bool WiCar_MQTT_SetTlsPsk(const char *psk, const char *identity, const char *ciphers)
{
	//No Support
	return false;
}

WISE_CARRIER_API bool WiCar_MQTT_Connect(const char* address, int port, const char* clientId, WICAR_LOSTCONNECT_CB on_lostconnect)
{

	if(ssl_flag)
		sprintf(IP,"ssl://%s:%d",address,port);
	else
		sprintf(IP,"tcp://%s:%d",address,port);		

	printf("IP : %s\n",IP);
    g_lostconnect_cb = on_lostconnect;

	MQTTClient_create(&client, IP, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);

	MQTTClient_setCallbacks(client, g_userdata, lostconnect_callback, message_callback, delivered_callback);	//must have messageArrived_callback

	if ((irc=MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
		conn_state=FIRST_LOST;
        printf("Failed to connect, return code %d\n", irc);      
		return false;
	}
	else
	{	
		conn_state=CONNECT;
		connect_callback();
		printf("connect successful!!\n");
		return true;
	}
	

}

WISE_CARRIER_API bool WiCar_MQTT_Reconnect()
{
	//No Use
	return false;
}

WISE_CARRIER_API bool WiCar_MQTT_Disconnect(int force)
{

	if(client==NULL)
	{
		printf("client==NULL\n");
		return false;
	}

	if ((irc=MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to disconnect, return code %d\n", irc);    
		return false;
    }
	else
	{
		printf("diconnect successful!!\n");
		MQTTClient_destroy(&client);
		disconnect_callback();
		return true;
	}
	

}

WISE_CARRIER_API bool WiCar_MQTT_Publish(const char* topic, const void *msg, int msglen, int retain, int qos)
{

	pubmsg.payload = (void *)msg;
    pubmsg.payloadlen = msglen;
    pubmsg.qos = qos;
    pubmsg.retained = retain;

	printf("publish : %s : %s\n",topic,msg);

	if(client==NULL)
	{
		printf("client==NULL\n");
		return false;
	}
	else
	{
		if ((irc=MQTTClient_publishMessage(client, topic, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)	//QOS=0 wiil disable token counting
		{	
			printf("Failed to pubilsh, return code %d\n", irc);      
			return false;
		}
		else
		{
			printf("publish successful!!\n");
			return true;
		}
	}

}

WISE_CARRIER_API bool WiCar_MQTT_Subscribe(const char* topic, int qos, WICAR_MESSAGE_CB on_recieve)
{

	if(client==NULL)
	{
		printf("client==NULL\n");
		return false;
	}
	else
	{
		if ((irc=MQTTClient_subscribe(client, topic, qos)) != MQTTCLIENT_SUCCESS)
		{
			printf("Failed to subscribe, return code %d\n", irc);  
			return false;
		}
		else
		{
			g_message_cb=on_recieve;
			printf("subscribe successful!!\n");
			return true;
		}
	}
}


WISE_CARRIER_API bool WiCar_MQTT_UnSubscribe(const char* topic)
{

	if(client==NULL)
	{
		printf("client==NULL\n");
		return false;
	}
	else
	{
		if ((irc=MQTTClient_unsubscribe(client,topic)) != MQTTCLIENT_SUCCESS)
		{
			printf("Failed to unsubscribe, return code %d\n", irc);   
			return false;
		}
		else
		{
			printf("unsubscribe successful!!\n");
			return true;
		}
	}

}

WISE_CARRIER_API bool WiCar_MQTT_GetLocalIP(const char * address)
{
	// No Support
	return false;
}

WISE_CARRIER_API const char *WiCar_MQTT_GetCurrentErrorString()
{
	switch(irc)
	{
		case MQTTCLIENT_SUCCESS:				//0
			return "No Error";
			break;
		case MQTTCLIENT_FAILURE:				//-1
			return "MQTTCLIENT_FAILURE";
			break;
		case MQTTCLIENT_MAX_MESSAGES_INFLIGHT:	//-4
			return "MQTTCLIENT_MAX_MESSAGES_INFLIGHT";
			break;
		case MQTT_BAD_SUBSCRIBE:				//0x80
			return "MQTT_BAD_SUBSCRIBE";
			break;
		case MQTTCLIENT_DISCONNECTED:			//-3
			return "MQTTCLIENT_DISCONNECTED";
			break;
		case MQTTCLIENT_BAD_UTF8_STRING:		//-5
			return "MQTTCLIENT_BAD_UTF8_STRING";
			break;
		case MQTTCLIENT_NULL_PARAMETER:			//-6
			return "MQTTCLIENT_NULL_PARAMETER";
			break;
		case MQTTCLIENT_TOPICNAME_TRUNCATED:	//-7
			return "MQTTCLIENT_TOPICNAME_TRUNCATED";
			break;
		case MQTTCLIENT_BAD_STRUCTURE:			//-8
			return "MQTTCLIENT_BAD_STRUCTURE";
			break;
		case MQTTCLIENT_BAD_QOS:				//-9
			return "MQTTCLIENT_BAD_QOS";
			break;
		default:
			return "Out Of Error Codes";
			break;
	}
}
