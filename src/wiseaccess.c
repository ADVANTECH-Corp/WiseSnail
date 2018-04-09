/*
 * wiseaccess.c
 *
 *  Created on: 2016¦~1¤ë20¤é
 *      Author: Fred.Chang
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "wiseconfig.h"
#include "wisememory.h"
#include "wiseutility.h"
//#include "wisemqtt.h"
#include "wiseagentlite.h"
#include "wiseaccess.h"
#include "wisestorage.h"
#include "WISECore.h"



///cagent/admin/00170d00006063c2/agentactionreq
static const char *SEN_GET_RESPONSE = "{\"susiCommData\":{\"commCmd\":%d,\"handlerName\":\"%s\",\"sessionID\":\"%s\",\"sensorInfoList\":{\"e\":[%s]}}}";
//@@@ commandId[n], handlerName[s], sessionId[s], senData[ss]

static const char *SEN_GET_DATA_V_JSON = "{\"n\":\"%s%s\",\"v\":%d,\"StatusCode\":%d}";
static const char *SEN_GET_DATA_FV_JSON = "{\"n\":\"%s%s\",\"v\":%f,\"StatusCode\":%d}";
static const char *SEN_GET_DATA_SV_JSON = "{\"n\":\"%s%s\",\"sv\":\"%s\",\"StatusCode\":%d}";
static const char *SEN_GET_DATA_BV_JSON = "{\"n\":\"%s%s\",\"bv\":%s,\"StatusCode\":%d}";

///cagent/admin/00170d00006063c2/agentactionreq
static const char *SEN_SET_RESPONSE = "{\"susiCommData\":{\"commCmd\":%d,\"handlerName\":\"%s\",\"sessionID\":\"%s\",\"sensorInfoList\":{\"e\":[%s]}}}";
//@@@ commandId[n], handlerName[s], sessionId[s], senData[ss]

///cagent/admin/00000d00006063c2/agentactionreq
//static const char *SEN_RENAME_RESPONSE = "{\"susiCommData\":{\"agentID\":\"%s\",\"commCmd\":%d,\"handlerName\":\"%s\",\"sessionID\":\"%s\",\"requestID\":2001,\"result\":\"%s\"}}";
//@@@ agentId[s], commandId[n], handlerName[s], sessionId[s], result[s]

/************************************************************/
//Variable


static WiseAgentCfg agentCfg;

void SetGWName(WiseAgentData *data) {
    strcpy(agentCfg.gwName, data->string);
    WiseStorage_WriteAgent(&agentCfg);
}

const char *GetGWName() {
    WiseStorage_ReadAgent(&agentCfg);
    return agentCfg.gwName;
}

static int heartBeatRate = 5;

void SetHeartBeatRate(WiseAgentData *data) {
	heartBeatRate = data->value;
}


/************************************************************/


/* cmd array */
#define AGENT_CMD_LEN 256
#define AGENT_SESSIOIN_ID_LEN 33
#define AGENT_CLIENT_ID_LEN 17
#define AGENT_HANDLE_NAME_LEN 32
#define AGENT_ITEM_NAME_LEN 32
#define AGENT_ITEM_VALUE_LEN 32

typedef struct wiseagent_cmddata {
    union {
        double value;
        char string[AGENT_CMD_LEN];
    };
} WiseAgent_CmdData;

typedef struct wiseagent_cmd {
    int cmdId;
	int deviceId;
    int itemId;
	char handleName[AGENT_HANDLE_NAME_LEN];
    char name[AGENT_ITEM_NAME_LEN];
	WiseAgent_CmdData data;
    int statusCode;
    char sessionId[AGENT_SESSIOIN_ID_LEN];
    void *userdata;
} WiseAgent_CMD;


static int gCmdHead = 0;
static int gCmdTail = 0;
WiseAgent_CMD gCmds[MAX_CMDS];

WiseAgentInfoSpec gGwItem = {
	.type = WISE_STRING, .name = "/GW/Name", .string = agentCfg.gwName, .setValue = SetGWName
};

static char gGwId[32] = {0};

typedef struct wiseagent_device {
	int connection;
    char cliendId[AGENT_CLIENT_ID_LEN];
	WiseDeviceCfg cfg;
	WiseDeviceINfo info;
	char infospec[4096];
	WiseAgentInfoSpec *items[MAX_ITEMS];
	int itemCount;
} WiseAgent_DEVICE;

WiseAgent_DEVICE gDevices[MAX_DEVICES];
static int gDeviceCount = 0;

WiseAgentInfoSpec gInterfaceItem[] = {
    { WISE_STRING, 	"/Info/SenHubList"},
    { WISE_STRING, 	"/Info/Neighbor"},
    { WISE_STRING, 	"/Info/Name"},
    { WISE_VALUE, 	"/Info/Health", .value = 100},
	{ WISE_STRING, 	"/Info/sw", .string = "1.2.1.12"},
    { WISE_BOOL, 	"/Info/reset", .value = 0}
};

WiseAgentInfoSpec gSensorDefaultItem[] = {
    { WISE_STRING, "/Info/Name", .string = "OnBoard"},
    { WISE_STRING, "/Info/sw", .string = INFO_SW},
    { WISE_STRING, "/Net/sw", .string = NET_SW},
	{ WISE_STRING, "/Net/Neighbor", .string = ""},
    { WISE_VALUE, "/Net/Health", .value = 100},
};

void dump_gDevices (void)
{
	int i = 0 , d = 0;
	for (d = 0 ; d < gDeviceCount ; d++)
	{
		fprintf (stderr, "gDevices [%d] connection=%d\n", d, gDevices[d].connection);
		fprintf (stderr, "gDevices [%d] cliendId=%s\n",   d, gDevices[d].cliendId);
		fprintf (stderr, "gDevices [%d] infospec=%s\n",   d, gDevices[d].infospec);
		fprintf (stderr, "gDevices [%d] itemCount=%d\n",  d, gDevices[d].itemCount);
		WiseAgentInfoSpec **items = gDevices[d].items;
		for (i=0;i<gDevices[d].itemCount;i++)
		{
			WiseAgentInfoSpec *item;
			item = items[i];
			fprintf (stderr, "gDevices [%d] item[%02d] name=%s\n",  d, i, item->name);
			switch(item->type)
			{
				case WISE_VALUE:
					fprintf (stderr, "gDevices [%d] item[%02d] type=%d ,  WISE_VALUE\n",  d, i, item->type);
					fprintf (stderr, "gDevices [%d] item[%02d] value=%f\n",     		  d, i, item->value);
					break;
				case WISE_FLOAT:
					fprintf (stderr, "gDevices [%d] item[%02d] type=%d , WISE_FLOAT\n",   d, i, item->type);
					fprintf (stderr, "gDevices [%d] item[%02d] value=%f\n",     		  d, i, item->value);
					break;
				case WISE_STRING:
					fprintf (stderr, "gDevices [%d] item[%02d] type=%d , WISE_STRING\n",  d, i, item->type);
					fprintf (stderr, "gDevices [%d] item[%02d] string=%s\n",     		  d, i, item->string);
					break;
				case WISE_BOOL:
					fprintf (stderr, "gDevices [%d] item[%02d] type=%d , WISE_BOOL\n",    d, i, item->type);
					fprintf (stderr, "gDevices [%d] item[%02d] value=%f\n",     		  d, i, item->value);
					break;
			}
		}
	}
}

/*void SetSHName(WiseAgentData *data) {
	int d = WiseAccess_FindDevice(data->clientId);
	if(d >= 0) {
		strcpy(gDevices[d].cfg.infoName, data->string);
		WiseStorage_WriteDevice(data->clientId, &gDevices[d].cfg);
	}
}

const char *GetSHName(char *deviceId) {
	int d = WiseAccess_FindDevice(deviceId);
	if(d >= 0) {
		WiseStorage_ReadDevice(deviceId, &gDevices[d].cfg);
		return gDevices[d].cfg.infoName;
	} else return NULL;
}*/

void WiseAccess_RepublishSensorConnectMessage();

void WiseAgent_Response(int cmdId, char *handler, int deviceId, int itemId, char *name, char *sessionId, int statusCode, WiseAgent_CmdData* cmddata) {
	char *mac;
	char *response = NULL;
    char *topic = (char *)WiseMem_Alloc(128);
	char *message = (char *)WiseMem_Alloc(8192);
    char *jsonvalue = (char *)WiseMem_Alloc(1024);
    char *pos = jsonvalue;
	
	if(deviceId == -1) {
		mac = gDevices[0].cliendId;
	} else {
		mac = gDevices[deviceId].cliendId;
	}
	
	if(strlen(handler) == 5 && strncmp(handler,"IoTGW",5) == 0) {
		sprintf(topic, WA_PUB_ACTION_TOPIC, gGwId);
	} else if(strlen(handler) == 6 && strncmp(handler,"SenHub",6) == 0) {
		sprintf(topic, WA_PUB_ACTION_TOPIC, mac);
	} else if(strncmp("0007",mac,4) == 0) {
		sprintf(topic, WA_PUB_ACTION_TOPIC, gGwId);
	} else {
		sprintf(topic, WA_PUB_ACTION_TOPIC, mac);
	}
	
    if(cmdId < 0 && statusCode == 0) {
       sprintf(message, "{\"errorRep\":\"Unknown cmd!\",\"sessionID\":\"%s\"}", sessionId);
	   response = message;
    } else {
        if(itemId >= 0) {
            WiseAgentInfoSpec *item;
			if(deviceId == -1) {
				item = &gGwItem;
			} else {
				item = gDevices[deviceId].items[itemId];
			}
            switch(cmdId) {
                case 524:
                {
					switch(item->type) {
						case WISE_VALUE:
							pos += sprintf(pos, SEN_GET_DATA_V_JSON, *name == '/' ? "" : "/SenData/", name, (int)cmddata->value, statusCode);
							break;
                        case WISE_FLOAT:
							pos += sprintf(pos, SEN_GET_DATA_FV_JSON, *name == '/' ? "" : "/SenData/", name, cmddata->value, statusCode);
							break;
						case WISE_STRING:
							pos += sprintf(pos, SEN_GET_DATA_SV_JSON, *name == '/' ? "" : "/SenData/", name, cmddata->string, statusCode);
							break;
						case WISE_BOOL:
							pos += sprintf(pos, SEN_GET_DATA_BV_JSON, *name == '/' ? "" : "/SenData/", name, cmddata->value > 0 ? "true" : "false", statusCode);
							break;
						/*default:
							wiseprint("Datatype error!!\n");
							infiniteloop();*/
					}
                } break;
                case 114:
                case 526:
                {
                    if(cmdId == 114) {
                        pos += sprintf(pos, "%s", "SUCCESS");
                    } else {
                        switch(item->type) {
							case WISE_VALUE:
                            case WISE_FLOAT:
							case WISE_BOOL:
								pos += sprintf(pos, SEN_GET_DATA_SV_JSON, *name == '/' ? "" : "/SenData/", name, "Success", 200);
								break;
							case WISE_STRING:
								pos += sprintf(pos, SEN_GET_DATA_SV_JSON, *name == '/' ? "" : "/SenData/", name, "Success", 200);
								break;
							/*default:
								wiseprint("Datatype error!!\n");
								infiniteloop();*/
						}
                    }
                } break;
            }
        } else {
            if(cmdId == 114) {
                pos += sprintf(pos, "%s", "FALSE");
            } else {
				switch(statusCode) {
					default:
					case 404:
						pos += sprintf(pos, SEN_GET_DATA_SV_JSON, *name == '/' ? "" : "/SenData/", name, "Not Found", statusCode);
						break;
				}
			}
		}
        switch(cmdId) {
            /*case 114:
                sprintf(message,SEN_RENAME_RESPONSE, mac, cmdId, handler, sessionId, jsonvalue);
            break;*/
            case 524:
                sprintf(message,SEN_GET_RESPONSE, cmdId, handler, sessionId, jsonvalue);
				response = message;
            break;
            case 526:
                sprintf(message,SEN_SET_RESPONSE, cmdId, handler, sessionId, jsonvalue);
				response = message;
            break;
			case 2052:
				if(deviceId == 0) {
                    WiseMem_Release();
					core_platform_register();
					WiseAgent_PublishInterfaceInfoSpecMessage(gGwId);
					WiseAgent_PublishInterfaceDeviceInfoMessage(gGwId);
					WiseAccess_RepublishSensorConnectMessage();
					response = NULL;
				} else {
					response = gDevices[deviceId].infospec;
				}
            break;
        }
    }

    //WiseMQTT_WriteOnce(topic, message);
	
	if(response != NULL) {
		core_publish(topic, response, strlen(response)+1, 0, 0);
	}
    
    WiseMem_Release();
}

/***************************************/
int WiseAccess_AssignCmd(int cmdId, int deviceId, int itemId, int statusCode, char *handleName, char *target, char *sessionId, void *data, void *userdata) {
	if((gCmdHead+1)%MAX_CMDS != gCmdTail) {
		WiseAgent_CMD *cmd = &gCmds[gCmdHead];
		cmd->cmdId = cmdId;
		cmd->deviceId = deviceId;
		cmd->itemId = itemId;
		
		if(data != NULL) memcpy(&cmd->data,data,sizeof(WiseAgent_CmdData));
		else memset(&cmd->data,0,sizeof(WiseAgent_CmdData));
		
		if(target != NULL) strcpy(cmd->name,target);
		else memset(cmd->name, 0, AGENT_ITEM_NAME_LEN);
		
		if(handleName != NULL) strcpy(cmd->handleName,handleName);
		else memset(cmd->handleName, 0, AGENT_HANDLE_NAME_LEN);
		
		cmd->statusCode = statusCode;
		
		if(sessionId != NULL) strcpy(cmd->sessionId, sessionId);
		else memset(cmd->sessionId, 0, AGENT_SESSIOIN_ID_LEN);
		
		gCmdHead = (gCmdHead+1)%MAX_CMDS;
		return 1;
	} else return 0;
}
static void CmdNotFound(int cmdId, int statusCode, char *handleName, char *target, char *sessionId) {
	WiseAccess_AssignCmd(-1, -1, -1, statusCode, handleName, target, sessionId, NULL, NULL);
}

float boolTrans(char *string, int len) 
{
	int i = 0;
	for(i = 0 ; i < len; i++) {
		if(string[i] == 't') {
			if(strncmp(&string[i],"true",4) == 0) {
				return (float)1.0;
			}
		}

		if(string[i] == 'f') {
			if(strncmp(&string[i],"false",5) == 0) {
				return (float)0.0;
			}
		}
	}

	return (float)atoi(string);
}

void CmdReceive(const char *topic, const void *payload, const long pktlength) {
	char sessionId[64] = {0};
	char handlerName[AGENT_HANDLE_NAME_LEN] = {0};
	char clientId[64] = {0};
	static char buffer[128] = {0};
	static char value[1024] = {0};
    int cmdId;
    char *start;
	char *target;
    char *end;
    int len;
    int search;
    WiseAgent_CMD *cmd;
    WiseAgentInfoSpec *item;
	WiseAgent_CmdData cmddata;
    wiseprint("@@@@@@@@@@@@@@topic = %s\r\n", topic);
    wiseprint("@@@@@@@@@@@@@@payload = %s\r\n", (char *)payload);

    start = strstr(payload,"\"commCmd\":");
	if(start != NULL) {
	    start += strlen("\"commCmd\":");
		cmdId = atoi(start);
	} else cmdId = -1;
	wiseprint("@@@@@@@@@@@@@@cmdId = %d\r\n", cmdId);
    //session id
    start = strstr(payload,"\"sessionID\":\"");
    if(start != NULL) {
		start += 13;
        strncpy(sessionId, start, 32);
        sessionId[32] = 0;
    } else {
		sessionId[0] = 0;
	}
    wiseprint("@@@@@@@@@@@@@@\033[36msessionId = [%s]\033[0m\r\n", sessionId);
	start = strstr(payload,"\"handlerName\":\"");
    if(start != NULL) {
		start += 15;
		end = strchr(start,'\"');
		len = (long)(end-start);
        strncpy(handlerName, start, len);
        handlerName[len] = 0;
    } else return;
	wiseprint("@@@@@@@@@@@@@@handlerName = %s\r\n", handlerName);
    switch(cmdId) {
    //set gwName
    case 113:
    {
        strcpy(buffer,"GW/Name");
        //wiseprint("@@@@@@@@@@@@@@\033[36msessionId = [%s]\033[0m\r\n", sessionId);

        //Name
        start = strstr(payload,"\"devName\":\"") + 11;
        end = strstr(start,"\"}");
        len = (long)(end-start);
        strncpy(value, start, len);
        value[len] = 0;

		WiseAgentData data;
		item = &gGwItem;
		
		data.clientId = gDevices[0].cliendId;;
		data.type = item->type;
		data.name = item->name;
		data.string = value;
		
		if(item->setValue != NULL) {
			data.info = item;
			item->setValue(&data);
		}
					
		if(WiseAccess_AssignCmd(cmdId, -1, 0, 200, NULL, item->name, sessionId, NULL, NULL)) {
			strcpy(item->string, value);
		}

		return;


    } break;
	/*case 125:
	{
		wiseprint("reconnect\n");
		if(strstr((char*)payload, "\"statuscode\":4") != NULL) {
			wiseprint("@@reconnect\n");
			start = strstr(topic,"/cagent/admin/") + 14;
			end = strstr(start,"/");
			len = (long)(end-start);
			strncpy(clientId,start,len);
			clientId[len] = 0;
			wiseprint("@@@@reconnect clientId = %s\n", clientId);
			int d = WiseAccess_FindDevice(clientId);
			wiseprint("@@@@reconnect d = %d\n", d);
			if(d > 0) {
				WiseAccess_AssignCmd(9999, d, 0, 200, NULL, NULL, NULL, NULL, NULL);
			}
		}
	} break;*/
	//get capability
    case 2051:
    {
    	wiseprint("get capability\n");
		start = strstr(topic,"/cagent/admin/") + 14;
		end = strstr(start,"/");
		len = (long)(end-start);
		strncpy(clientId,start,len);
		clientId[len] = 0;
		int d = WiseAccess_FindDevice(clientId);
		if(d < 0) d = 0;
		WiseAccess_AssignCmd(cmdId, d, 0, 200, NULL, NULL, NULL, NULL, NULL);
    } break;
    case 523:
    case 525:
    {
        //wiseprint("@@@@@@@@@@@@@@\033[36mtopic = %s\033[0m\r\n", topic);
        //wiseprint("@@@@@@@@@@@@@@\033[36mpayload = %s\033[0m\r\n", payload);
		int d = -1;
		int *itemCount = NULL;
		WiseAgentInfoSpec **items = NULL;
		if(strlen(handlerName) == 5 && strncmp(handlerName,"IoTGW",5) == 0) {
			start = strstr(payload,"IoTGW/") + 6;
			start = strchr(start,'/')+1;
			end = strchr(start,'/');
			len = (long)(end-start);
			
			strncpy(clientId,start,len);
			clientId[len] = 0;
			start += len;
			
			d = WiseAccess_FindDevice(clientId);
			
			if(d < 0) return;
			items = gDevices[d].items;
			itemCount = &gDevices[d].itemCount;
			
			end = strstr(start,"\"}");
			len = (long)(end-start);
			strncpy(buffer,start,len);
			buffer[len] = 0;
			target = buffer;
			
		} else if(strlen(handlerName) == 6 && strncmp(handlerName,"SenHub",6) == 0) {
			start = strstr(topic,"/cagent/admin/") + 14;
			end = strstr(start,"/");
			len = (long)(end-start);
			strncpy(clientId,start,len);
			clientId[len] = 0;
			
			d = WiseAccess_FindDevice(clientId);

			if(d < 0) return;
			items = gDevices[d].items;
			itemCount = &gDevices[d].itemCount;
			
			start = strstr(payload,"SenHub/") + 6;/* + strlen("SenHub/");*/
			end = strstr(start,"\"}");
			len = (long)(end-start);
			strncpy(buffer,start,len);
			buffer[len] = 0;
			
			start = strstr(payload,"/SenData/");
			if(start != NULL) {
				target = buffer + 9;
			} else {
				target = buffer;
			}
			//wiseprint("@@@@@@@@@@@@@@\033[36mbuffer = [%s]\033[0m\r\n", buffer);

			//wiseprint("@@@@@@@@@@@@@@\033[36msessionId = [%s]\033[0m\r\n", sessionId);
		}
		
		if(d < 0) return;
		
		//find value
		for(search = 0 ; search < *itemCount ; search++) {
			item = items[search];
			if(strcmp(target, item->name) == 0) {
				//if(WiseAccess_AssignCmd(cmdId, search, 200, item->name, sessionId, NULL)) {

				if((gCmdHead+1)%MAX_CMDS != gCmdTail) {
					/*cmd = &gCmds[gCmdHead];
					cmd->cmdId = cmdId;
					cmd->id = search;
					strcpy(cmd->name,item->name);
					cmd->statusCode = 200;
					strcpy(cmd->sessionId, sessionId);*/
					
					if(cmdId == 525) {
						//value set in
						WiseAgentData data;
						data.clientId = gDevices[d].cliendId;
						data.type = item->type;
						data.name = item->name;
						
						
						start = strstr(payload,"v\":");
						switch(*(start-1)) {
							case '\"':
								start += 3;
								end = strstr(start,",");
								len = (long)(end-start);
								memcpy(value, start, (long)(end-start));
								value[len] = 0;
								//item->value = atof(value);
								cmddata.value = atof(value);
								data.value = cmddata.value;
								break;
							case 's':
								start += 4;
								end = start;
								do {
									end = strstr(end,"\"");
									if(*(end-1) != '\\') break;
								} while(1);
								len = (long)(end-start);
								memcpy(value, start, len);
								value[len] = 0;
								strcpy(cmddata.string, value);
								data.string = cmddata.string;
								break;
							case 'b':
								start += 3;
								end = strstr(start,",");
								len = (long)(end-start);
								memcpy(value, start, len);
								value[len] = 0;
								//item->value = (float)atoi(value);
								//cmddata.value = (float)atoi(value);
								cmddata.value = boolTrans(value,strlen(value));
								data.value = cmddata.value;
								break;
							default:
								break;
						}
						
						if(item->setValue != NULL) {
							data.info = item;
							item->setValue(&data);
						}
					} else if(cmdId == 523) {
						if(item->getValue != NULL) {
							WiseAgentData data;
							data.clientId = gDevices[d].cliendId;
							data.type = item->type;
							data.name = item->name;
							switch(item->type) {
								case WISE_VALUE:
                                case WISE_FLOAT:
                                case WISE_BOOL:
									item->getValue(&data);
									cmddata.value = data.value;
									break;
								case WISE_STRING:
									data.string = cmddata.string;
									item->getValue(&data);
									break;
							}
						} else {
							switch(item->type) {
								case WISE_VALUE:
                                case WISE_FLOAT:
                                case WISE_BOOL:
									cmddata.value = item->value;
									break;
								case WISE_STRING:
									strcpy(cmddata.string, item->string);
									break;
							}
						}
					}

					//gCmdHead = (gCmdHead+1)%MAX_CMDS;
					WiseAccess_AssignCmd(cmdId, d, search, 200, handlerName, item->name, sessionId, &cmddata, NULL);
				}

				return;
			}
		}
		
        CmdNotFound(cmdId, 404, handlerName, target, sessionId);

    } break;

    default:
        CmdNotFound(cmdId, 0, handlerName, NULL, sessionId);
        break;
    }

    wiseprint("CmdReceive completed!!\n");

}

void WiseAccess_Init(char *default_gwName, char *gwMac) {
    int retval = WiseStorage_ReadAgent(&agentCfg);

    if(retval == -1) {
        memset(&agentCfg, 0, sizeof(WiseAgentCfg));
    }
    //GW Name
    if(strlen(agentCfg.gwName) == 0) {
        sprintf(agentCfg.gwName, "%s(%s)",default_gwName, gwMac + (strlen(gwMac) - 4));
    }
    strcpy(gGwId,gwMac);
    WiseStorage_WriteAgent(&agentCfg);

}


static char gIf_Name[32];
static char gIf_SenHublist[2048];
void WiseAccess_InterfaceInit(char *deviceMac, char *name) {
	int d = WiseAccess_CreateDevice(deviceMac);
	if(d != 0) return;
	if(gDevices[d].itemCount == 0) {
		/*char *pos = gIf_SenHublist;
		pos += sprintf(pos,"%s",deviceMac);
		*(pos+2) = 0;*/
		gInterfaceItem[0].string = gIf_SenHublist;
		WiseAccess_AddItem(deviceMac, "/Info/SenHubList", &gInterfaceItem[0]);
		gInterfaceItem[1].string = gIf_SenHublist;
		WiseAccess_AddItem(deviceMac, "/Info/Neighbor", &gInterfaceItem[1]);
		strncpy(gIf_Name, name, sizeof(gIf_Name));
		gInterfaceItem[2].string = gIf_Name;
		WiseAccess_AddItem(deviceMac, "/Info/Name", &gInterfaceItem[2]);
		
		WiseAccess_AddItem(deviceMac, "/Info/Health", &gInterfaceItem[3]);
		WiseAccess_AddItem(deviceMac, "/Info/sw", &gInterfaceItem[4]);
		WiseAccess_AddItem(deviceMac, "/Info/reset", &gInterfaceItem[5]);
	}
}

void WiseAccess_SensorInit(char *deviceMac, char *shName) {
	int d = WiseAccess_CreateDevice(deviceMac);
	if(d < 1) return;
	if(gDevices[d].itemCount == 0) {
		gDevices[d].connection = 1;
		
		WiseAccess_AddItem(deviceMac, "/Info/Name", &gSensorDefaultItem[0]);
		
		WiseAccess_AddItem(deviceMac, "/Info/sw", &gSensorDefaultItem[1]);
		WiseAccess_AddItem(deviceMac, "/Net/sw", &gSensorDefaultItem[2]);
		
		WiseAccess_AddItem(deviceMac, "/Net/Neighbor", &gSensorDefaultItem[3]);
		WiseAccess_AddItem(deviceMac, "/Net/Health", &gSensorDefaultItem[4]);
	}
}

void WiseAccess_SetInfoSpec(char *deviceMac, const char *infospec, int len) {
	int d = WiseAccess_FindDevice(deviceMac);
	if(d < 0) return;
	if(len > 4096) {
		wiseprint("infospec overflow\n");
		return;
	}

	strcpy(gDevices[d].infospec, infospec);
}


void WiseAccess_ConnectionStatus(char *deviceMac, int status) {
	int d = WiseAccess_FindDevice(deviceMac);
	if(d < 0) return;
	gDevices[d].connection = status;
}

void WiseAccess_Register(char *topic) {
    //WiseMQTT_Read(topic, CmdReceive);
}

int WiseAccess_FindDevice(char *deviceId) {
	int d = 0;
	for(d = 0 ; d < gDeviceCount ; d++) {
		if(strcmp(deviceId, gDevices[d].cliendId) == 0) {
			return d;
		}
	}
	return -1;
}

int WiseAccess_CreateDevice(char *deviceId) {
	int d = 0;
	for(d = 0 ; d < gDeviceCount ; d++) {
		if(strcmp(deviceId, gDevices[d].cliendId) == 0) {
			return d;
		}
	}
	
	if(gDeviceCount >= MAX_DEVICES) return -1;
	d = gDeviceCount;
	strncpy(gDevices[d].cliendId, deviceId, AGENT_CLIENT_ID_LEN);
	gDeviceCount++;
	return d;
}

static WiseAgentInfoSpec *WiseAccess_FindItem(WiseAgentInfoSpec **items, int *itemCount, char *name, WiseAgentInfoSpec *item) {
	int i = 0;
	for(i = 0 ; i < *itemCount ; i++) {
		if(strcmp(name, items[i]->name) == 0) {
			items[i] = item;
			return item;
		}
	}
	i = *itemCount;
	if(*itemCount < MAX_ITEMS) {
		(*itemCount)++;
		items[i] = item;
		return item;
	} else return NULL;
}

bool WiseAccess_AddItem(char *deviceId, char *name, WiseAgentInfoSpec *item) {
	int d = WiseAccess_FindDevice(deviceId);
	if(d < 0) return false;
	WiseAgentInfoSpec **items = gDevices[d].items;
	int *itemCount = &gDevices[d].itemCount;
	
	if( WiseAccess_FindItem(items, itemCount, name, item) != NULL) return true;
	else return false;
}

void WiseAccess_AccessVariable(char *deviceId, int set, char *name, WiseAgentData *data) {
    int d = WiseAccess_FindDevice(deviceId);
	if(d < 0) return;
	WiseAgentInfoSpec **items = gDevices[d].items;
	int itemCount = gDevices[d].itemCount;
	int search;
    WiseAgentInfoSpec *item;
    for(search = 0 ; search < itemCount ; search++) {
        item = items[search];
        if(strcmp(name, item->name) == 0) {
            switch(item->type) {
                case WISE_VALUE:
                case WISE_BOOL:
                case WISE_FLOAT:
                    if(set == 1) {
                        item->value = data->value;
                    } else {
                        data->value = item->value;
                    }
                    break;
                case WISE_STRING:
                    if(set == 1) {
						item->string = data->string;
                    } else {
						data->string = item->string;
                    }
                    break;
                /*default:
                    wiseprint("Datatype error!!\n");
                    infiniteloop();*/
            }
            break;
        }
    }
}

void WiseAccess_Update(char *deviceId, char *name, WiseAgentData *data) {
    WiseAccess_AccessVariable(deviceId, 1, name, data);
}

void WiseAccess_Get(char *deviceId, char *name, WiseAgentData *data) {
    WiseAccess_AccessVariable(deviceId, 0, name, data);
}

void WiseAccess_GetTopology() {
	int i = 0;
	int count = 0;
    char *topology = (char *)WiseMem_Alloc(1024);
	char *pos = topology;
	for(i = 1 ; i < gDeviceCount ; i++) {
		if(gDevices[i].connection) {
			if(count > 0) {
				pos += sprintf(pos,",");
			}
			pos += sprintf(pos,"%s",gDevices[i].cliendId);
			count++;
		}
	}
	
	char *target = gIf_SenHublist;
	if(count == 0) {
		/*target += sprintf(gIf_SenHublist,"%s",gDevices[0].cliendId);
		*(target+2) = 0;*/
        memset(gIf_SenHublist, 0, sizeof(gIf_SenHublist));
	} else {
		//sprintf(gIf_SenHublist,"%s,%s",gDevices[0].cliendId, topology);
        sprintf(gIf_SenHublist,"%s",topology);
	}
	
	WiseMem_Release();
}

void WiseAccess_RepublishSensorConnectMessage() {
	int i = 0;
	for(i = 1 ; i < gDeviceCount ; i++) {
		if(gDevices[i].connection) {
			WiseAgent_PublishSensorConnectMessage(gDevices[i].cliendId);
		}
	}
}


void WiseAccess_GenerateTokenCapability(char *deviceId, char *token, char *buffer, int buflen) {
	if(strlen(token) == 0) return;
	int d = WiseAccess_FindDevice(deviceId);
	if(d < 0) return;
	WiseAgentInfoSpec **items = gDevices[d].items;
	int itemCount = gDevices[d].itemCount;
	int search;
    WiseAgentInfoSpec *item;
	char *pos = buffer;
	int count = 0;

    for(search = 0 ; search < itemCount ; search++) {
        item = items[search];
		if(item->name[0] == '/') {
			if(strncmp(item->name+1, token, strlen(token)) == 0) {
				if(count != 0) pos += sprintf(pos, ",");
				pos += sprintf(pos, "{\"n\":\"%s\"", item->name+strlen(token) + 2);
				switch(item->type) {
					case WISE_VALUE:
						pos += sprintf(pos, ",\"v\":%d", (int)item->value);
						break;
                    case WISE_FLOAT:
						pos += sprintf(pos, ",\"v\":%f", item->value);
						break;
					case WISE_BOOL:
						pos += sprintf(pos, ",\"bv\":%s", item->value > 0 ? "true" : "false");
						break;
					case WISE_STRING:
						pos += sprintf(pos, ",\"sv\":\"%s\"", item->string);
						break;
					/*default:
						wiseprint("Datatype error!!\n");
						infiniteloop();*/
				}
				
				if(item->setValue == NULL) {
					pos += sprintf(pos, ",\"asm\":\"r\"");
				} else {
					pos += sprintf(pos, ",\"asm\":\"rw\"");
				}
				pos += sprintf(pos, "}");
				count++;
			}
		}
    }
}

void WiseAccess_GenerateTokenDataInfo(char *deviceId, char *token, char *buffer, int buflen) {
	if(strlen(token) == 0) return;
	int d = WiseAccess_FindDevice(deviceId);
	if(d < 0) return;
	WiseAgentInfoSpec **items = gDevices[d].items;
	int itemCount = gDevices[d].itemCount;
	int search;
    WiseAgentInfoSpec *item;
	char *pos = buffer;
	int count = 0;

    for(search = 0 ; search < itemCount ; search++) {
        item = items[search];
		if(item->name[0] == '/') {
			if(strncmp(item->name+1, token, strlen(token)) == 0) {
				if(count != 0) pos += sprintf(pos, ",");
				pos += sprintf(pos, "{\"n\":\"%s\"", item->name+strlen(token) + 2);
				switch(item->type) {
					case WISE_VALUE:
						pos += sprintf(pos, ",\"v\":%d", (int)item->value);
						break;
                    case WISE_FLOAT:
						pos += sprintf(pos, ",\"v\":%f", item->value);
						break;
					case WISE_BOOL:
						pos += sprintf(pos, ",\"bv\":%s", item->value > 0 ? "true" : "false");
						break;
					case WISE_STRING:
						pos += sprintf(pos, ",\"sv\":\"%s\"", item->string);
						break;
					/*default:
						wiseprint("Datatype error!!\n");
						infiniteloop();*/
				}
				
				/*if(item->setValue == NULL) {
					pos += sprintf(pos, ",\"asm\":\"r\"");
				} else {
					pos += sprintf(pos, ",\"asm\":\"rw\"");
				}*/
				pos += sprintf(pos, "}");
				count++;
			}
		}
    }
}

int count = 0;
void WiseAccess_Handler() {
    WiseAgent_CMD *cmd;
    while(gCmdHead != gCmdTail) {
        cmd = &gCmds[gCmdTail];
        int cmdId = cmd->cmdId;
        switch(cmdId) {
		case 9999:
			if(cmd->deviceId == -1) {
				core_device_register();
			} else {
				WiseAgent_PublishSensorConnectMessage(cmd->name);
			}
			break;
		case 127:
			core_heartbeatratequery_response(heartBeatRate, cmd->sessionId, cmd->name);
			break;
		case 129:
			core_action_response(cmdId+1, cmd->sessionId, true, cmd->name);
			break;
        case 113:
            //WiseAgent_Response(114, "general", GW_MACADDRESS, cmd->id, cmd->name, cmd->sessionId, cmd->statusCode);
        	core_action_response(cmdId+1, cmd->sessionId, true, gDevices[cmd->deviceId].cliendId);
            break;
		case 2051:
        case 523:
        case 525:
            WiseAgent_Response(cmdId+1, cmd->handleName, cmd->deviceId, cmd->itemId, cmd->name, cmd->sessionId, cmd->statusCode, &cmd->data);
            break;
        }
        gCmdTail = (gCmdTail+1)%MAX_CMDS;
    }
	
	
	if(count%heartBeatRate == 0) {
		core_heartbeat_send();
	}
	count = (count+1%heartBeatRate);
}












