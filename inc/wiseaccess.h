/*
 * wiseaccess.h
 *
 *  Created on: 2016¦~1¤ë20¤é
 *      Author: Fred.Chang
 */

#ifndef WISEACCESS_H_
#define WISEACCESS_H_
#include <stdbool.h>
#include "version.h"
#include "wiseagentlite.h"

#define INFO_SW "1.0.00"
#define NET_VERSION(m,x,y,z) #m"."#x"."#y"."#z
#define NET_SW NET_VERSION(VER_MAJOR,VER_MINOR,VER_BUILD,VER_FIX)

void WiseAccess_Init(char *default_gwName, char *gwMac);
void WiseAccess_InterfaceInit(char *deviceMac, char *name);
void WiseAccess_SensorInit(char *deviceMac, char *shName);
void WiseAccess_SetInfoSpec(char *deviceMac, const char *infospec, int len);
void WiseAccess_ConnectionStatus(char *deviceMac, int status);
void WiseAccess_Register(char *topic);
bool WiseAccess_AddItem(char *deviceId, char *name, WiseAgentInfoSpec *item);
void WiseAccess_Update(char *deviceId, char *name, WiseAgentData *data);
void WiseAccess_Get(char *deviceId, char *name, WiseAgentData *data);

int WiseAccess_FindDevice(char *deviceId);
int WiseAccess_CreateDevice(char *deviceId);

//variable access
const char *GetGWName();
const char *GetSHName(char *deviceId);

//Response
void WiseAgent_RenameResponse(int cmdId, char *handler, char *mac, int index, char *name, char *sessionId, int statusCode);
void WiseAgent_GetResponse(int cmdId, char *handler, char *mac, int index, char *name, char *sessionId, int statusCode);
void WiseAgent_SetResponse(int cmdId, char *handler, char *mac, int index, char *name, char *sessionId, int statusCode);

int WiseAccess_AssignCmd(int cmdId, int deviceId, int itemId, int statusCode, char *handleName, char *target, char *sessionId, void *data, void *userdata);
void WiseAccess_GetTopology();
void WiseAccess_GenerateTokenCapability(char *deviceId, char *token, char *buffer, int buflen);
void WiseAccess_GenerateTokenDataInfo(char *deviceId, char *token, char *buffer, int buflen);
//Handler
void WiseAccess_Handler();

//Callback
void CmdReceive(const char *topic, const void *payload, const long pktlength);

#endif /* WISEACCESS_H_ */
