/*
 * wiseagentlite.h
 *
 *  Created on: 2015¦~11¤ë16¤é
 *      Author: Fred.Chang
 */

#ifndef WISEAGENTLITE_H_
#define WISEAGENTLITE_H_

//#define SERVER_ADDRESS          "172.22.12.206"
//#define SERVER_ADDRESS 			"wise-demo.cloudapp.net"
//#define SERVER_ADDRESS			"adv-wisecloud.cloudapp.net"
//#define SERVER_ADDRESS				"wise1501-demo.cloudapp.net"
//#define SERVER_ADDRESS 				"172.22.12.124"
//#define SERVER_ADDRESS			"rmm.wise-paas.com"
//#define SERVER_ADDRESS                "172.22.12.111"
//#define SERVER_ADDRESS                "172.22.12.92"
//#define SERVER_ADDRESS                "172.22.12.208"
#define SERVER_ADDRESS            "dev-wisepaas.eastasia.cloudapp.azure.com"
//#define SERVER_ADDRESS              "192.168.100.200"

// TOPIC
#define WA_PUB_CONNECT_TOPIC "/cagent/admin/%s/agentinfoack" /**/
#define WA_PUB_ACTION_TOPIC  "/cagent/admin/%s/agentactionreq"
#define WA_PUB_DEVINFO_TOPIC "/cagent/admin/%s/deviceinfo"

// Subscribe
#define WA_SUB_CBK_TOPIC "/cagent/admin/%s/agentcallbackreq"


#include "WiseSnail.h"
#define WiseDataType WiseSnail_DataType

typedef struct WiseSnail_Data WiseAgentData;
typedef struct WiseSnail_InfoSpec WiseAgentInfoSpec;

typedef void (*WiseMQTT_Event)(void *eventData);

#define WiseMQTT_SetValue WiseSnail_SetValue
#define WiseMQTT_GetValue WiseSnail_GetValue

void WiseAgent_Init(char *productionName, char *wanIp, unsigned char *parentMac, WiseAgentInfoSpec *infospec, int count);

void WiseAgent_RegisterInterface(char *ifMac, char *ifName, int ifNumber, WiseAgentInfoSpec *infospec, int count);

int WiseAgent_Open(char *server_url, int port, char *username, char *password, WiseAgentInfoSpec *infospec, int count);

void WiseAgent_RegisterSensor(char *deviceMac, char *defaultName, WiseAgentInfoSpec *infospec, int count);

void WiseAgent_SenHubDisconnect(char *deviceMac);
void WiseAgent_SenHubReConnected(char *deviceMac);


void WiseAgent_Write(char *deviceMac, WiseAgentData* data, int count);
void WiseAgent_Get(char *deviceMac, char *name, WiseAgentData *data);
void WiseAgent_Cmd_Handler();
void WiseAgent_Close();

//Advance function
void WiseAgent_Publish(const char *topic, const char *msg, const int msglen, const int retain, const int qos);

#endif /* WISEAGENTLITE_H_ */
