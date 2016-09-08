/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/03/16 by Zach Chih								    */
/* Modified Date: 2016/03/22 by Zach Chih									*/
/* Abstract     : MQTT Connector											*/
/* Reference    : None														*/
/****************************************************************************/
#include "WISEConnector.h"
#include "WiseCarrier_MQTT.h"
#include "MQTTConnector.h"
#include <stdio.h>

static WC_CONNECT_CALLBACK g_connect_cb = NULL;
static WC_LOSTCONNECT_CALLBACK g_lostconnect_cb = NULL;
static WC_DISCONNECT_CALLBACK g_disconnect_cb = NULL;
static WC_MESSAGE_CALLBACK g_message_cb = NULL;

char *clientID;
void *g_userdata;

void wc_connect_callback(void *userdata)
{	
	//printf("wc_connect_callback...\n");
	if(g_connect_cb!=NULL)
		g_connect_cb(g_userdata);
}
void wc_disconnect_callback(void *userdata)
{	
	//printf("wc_disconnect_callback...\n");
	if(g_disconnect_cb!=NULL)
		g_disconnect_cb(g_userdata);
}

void wc_lostconnect_callback(void *userdata)
{
	//printf("wc_lostconnect_callback...\n");
	if(g_lostconnect_cb!=NULL)
		g_lostconnect_cb(userdata);
}

void wc_message_callback(const char* topic, const void* payload, const int payloadlen, void *userdata)
{	
	//printf("wc_message_callback...\n");
	if(g_message_cb!=NULL)
		g_message_cb(topic,payload,payloadlen,userdata);
}


WISE_CONNECTOR_API bool wc_initialize(char const * devid, void* userdata)
{	
	//printf("wc_initialize...\n");
	g_userdata=userdata;
	//printf("g_userdata : %u\n",g_userdata);
	clientID=(char *)devid;
	//printf("devid: %s\n",clientID);
	return WiCar_MQTT_Init(wc_connect_callback, wc_disconnect_callback, g_userdata);
}


WISE_CONNECTOR_API void wc_uninitialize()
{
	 WiCar_MQTT_Uninit();
}


WISE_CONNECTOR_API void wc_callback_set( WC_CONNECT_CALLBACK connect_cb, WC_LOSTCONNECT_CALLBACK lostconnect_cb, WC_DISCONNECT_CALLBACK disconnect_cb, WC_MESSAGE_CALLBACK message_cb)
{
	g_connect_cb=connect_cb;
	g_lostconnect_cb=lostconnect_cb;
	g_disconnect_cb=disconnect_cb;
	g_message_cb=message_cb;

}


WISE_CONNECTOR_API bool wc_tls_set(const char *cafile, const char *capath, const char *certfile, const char *keyfile, const char* password)
{

	if(WiCar_MQTT_SetTls(cafile, capath, certfile, keyfile, password))
		return true;
	else
		return false;

}


WISE_CONNECTOR_API bool wc_tls_psk_set(const char *psk, const char *identity, const char *ciphers)
{
	if(WiCar_MQTT_SetTlsPsk(psk, identity, ciphers))
		return true;
	else
		return false;

}


WISE_CONNECTOR_API bool wc_connect(char const * ip, int port, char const * username, char const * password, int keepalive, char* willtopic, const void *willmsg, int msglen )
{	
	if(WiCar_MQTT_SetKeepLive(keepalive))
		if(WiCar_MQTT_SetAuth(username, password))
			if(WiCar_MQTT_SetWillMsg(willtopic, willmsg, msglen))
				if(WiCar_MQTT_Connect(ip, port, clientID, wc_lostconnect_callback))
					return true;
				else
					return false;
			else
				return false;
		else
			return false;
	else
		return false;

	
}


WISE_CONNECTOR_API bool wc_disconnect(bool bForce)
{
	if(WiCar_MQTT_Disconnect(bForce))
		return true;
	else
		return false;
}


WISE_CONNECTOR_API bool wc_publish(const char* topic, const void *msg, int msglen, int retain, int qos)
{
	if(WiCar_MQTT_Publish(topic,msg,msglen,retain,qos))
		return true;
	else
		return false;
}

WISE_CONNECTOR_API bool wc_subscribe(const char* topic, int qos)
{

	if(WiCar_MQTT_Subscribe(topic, qos, wc_message_callback))
		return true;
	else
		return false;

}

WISE_CONNECTOR_API bool wc_unsubscribe(const char* topic)
{
	if(WiCar_MQTT_UnSubscribe(topic))
		return true;
	else
		return false;

}

WISE_CONNECTOR_API bool wc_address_get(const char *address)
{
	return WiCar_MQTT_GetLocalIP(address);
}

WISE_CONNECTOR_API const char* wc_current_error_string_get()
{
	return WiCar_MQTT_GetCurrentErrorString();
}
