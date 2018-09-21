/*
 * wiseagentlite.c
 *
 *  Created on: 2015¦~11¤ë13¤é
 *      Author: Fred.Chang
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>

#include "snail_version.h"
#include "wisememory.h"
#include "wiseutility.h"
//#include "wisemqtt.h"
#include "wiseagentlite.h"
#include "wiseaccess.h"
#include "WISECore.h"

#if defined(WIN32)
#pragma comment(lib, "WiseCore_MQTT.lib")
#endif
/*===============================================================*/

#define INTERFACE "Ethernet"
#define VERSION SNAIL_FACTORY_VERSION

#define JSON_FMT_VER 2

int gJsonFmtVer = JSON_FMT_VER;

//[WillMessage]
///cagent/admin/000000049F0130E0/willmessage
//static const char *WILLMESSAGE_JSON = "{\"susiCommData\":{\"devID\":\"%s\",\"hostname\":\"%s\",\"sn\":\"%s\",\"mac\":\"%s\",\"version\":\""VERSOIN"\",\"type\":\"IoTGW\",\"product\":\"\",\"manufacture\":\"\",\"account\":\"%s\",\"passwd\":\"%s\",\"status\":%d,\"commCmd\":1,\"requestID\":21,\"agentID\":\"%s\",\"handlerName\":\"general\",\"sendTS\":%d}}";
//@@@ gwMac[s], hostname[s], Mac[s], Mac[s], username[s]{anonymous}, password[s], 0|1(connect or disconnect), gwMac[s], timestamp[d]

//[Connect]
///cagent/admin/000000049F0130E0/agentinfoack
//static const char *CONNECT_JSON = "{\"susiCommData\":{\"devID\":\"%s\",\"hostname\":\"%s\",\"sn\":\"%s\",\"mac\":\"%s\",\"version\":\""VERSOIN"\",\"type\":\"IoTGW\",\"product\":\"\",\"manufacture\":\"\",\"account\":\"%s\",\"passwd\":\"%s\",\"status\":%d,\"commCmd\":1,\"requestID\":21,\"agentID\":\"%s\",\"handlerName\":\"general\",\"sendTS\":%d}}";
//@@@ gwMac[s], hostname[s], Mac[s], Mac[s], username[s]{anonymous}, password[s], 0|1(connect or disconnect), gwMac[s], timestamp[d]

//[OSINFO]
///cagent/admin/000000049F0130E0/agentactionreq
//static const char *OSINFO_JSON = "{\"susiCommData\":{\"osInfo\":{\"cagentVersion\":\""VERSOIN"\",\"cagentType\":\"IoTGW\",\"osVersion\":\"\",\"biosVersion\":\"\",\"platformName\":\"\",\"processorName\":\"\",\"osArch\":\"RTOS\",\"totalPhysMemKB\":1026060,\"macs\":\"%s\",\"IP\":\"%s\"},\"commCmd\":116,\"requestID\":109,\"agentID\":\"%s\",\"handlerName\":\"general\",\"sendTS\":%d}}";
//@@@ Mac[n], ip[s], gwMac[s], timestamp[d]

static const char *SENDATA_JSON = "\"SenData\":{\"e\":[%s],\"bn\":\"SenData\"}";
static const char *INFO_JSON = "\"Info\":{\"e\":[%s],\"bn\":\"Info\"}";
static const char *NET_JSON = "\"Net\":{\"e\":[%s],\"bn\":\"Net\"}";
static const char *ACTION_JSON = "\"Action\":{\"e\":[%s],\"bn\":\"Action\"}";

//[INFOSPEC]
///cagent/admin/000000049F0130E0/agentactionreq
static const char *INFOSPEC_JSON = "{\"susiCommData\":{\"infoSpec\":{\"%s\":{\"%s\":{\"%s\":{\"Info\":{\"e\":[%s],\"bn\":\"Info\"},\"bn\":\"%s\",\"ver\":1},\"bn\":\"%s\",\"ver\":1},\"opTS\":{\"$date\":%lld},\"ver\":%d}},\"commCmd\":2052,\"requestID\":2001,\"agentID\":\"%s\",\"handlerName\":\"general\",\"sendTS\":%lld}}";
//@@@ Interface[s], Interface[s], InterfaceNumber[d], SenHubList[sl], Topology[sl], Interface[s], InterfaceNumber[d], Health[d], wsnMac[s], Interface[s], gwMac[s], timestamp[d]


//[DEVICEINFO]
///cagent/admin/000000049F0130E0/deviceinfo
static const char *DEVICEINFO_JSON = "{\"susiCommData\":{\"data\":{\"%s\":{\"%s\":{\"%s\":{%s,\"bn\":\"%s\",\"ver\":1},\"bn\":\"%s\"},\"opTS\":{\"$date\":%lld},\"ver\":%d}},\"commCmd\":2055,\"requestID\":2001,\"agentID\":\"%s\",\"handlerName\":\"general\",\"sendTS\":%lld}}";
//@@@ Interface[s], Interface[s], InterfaceNumber[d], SenHubList[sl], Topology[sl], Interface[s], InterfaceNumber[d], Health[d], wsnMac[s], Interface[s], gwMac[s], timestamp[d]


//[Sensor Connect]
///cagent/admin/00170d00006063c2/agentinfoack
static const char *SEN_CONNECT_JSON = "{\"susiCommData\":{\"devID\":\"%s\",\"hostname\":\"%s\",\"sn\":\"%s\",\"mac\":\"%s\",\"version\":\""VERSION"\",\"type\":\"SenHub\",\"product\":\"\",\"manufacture\":\"\",\"status\":\"1\",\"commCmd\":1,\"requestID\":30002,\"agentID\":\"%s\",\"handlerName\":\"general\",\"sendTS\":%lld}}";
//@@@ sMac[s], hostname[s]{Agriculture}, sMac[s], sMac[s], sMac[s], timestamp[d]

//[Sensor Disconnect]
///cagent/admin/00170d00006063c2/agentinfoack
static const char *SEN_DISCONNECT_JSON = "{\"susiCommData\":{\"devID\":\"%s\",\"hostname\":\"%s\",\"sn\":\"%s\",\"mac\":\"%s\",\"version\":\""VERSION"\",\"type\":\"SenHub\",\"product\":\"\",\"manufacture\":\"\",\"status\":\"0\",\"commCmd\":1,\"requestID\":30002,\"agentID\":\"%s\",\"handlerName\":\"general\",\"sendTS\":%lld}}";
//@@@ sMac[s], hostname[s]{Agriculture}, sMac[s], sMac[s], sMac[s], timestamp[d]


//[Sensor INFOSPEC]
///cagent/admin/00170d00006063c2/agentactionreq
static const char *SEN_INFOSPEC_JSON = "{\"susiCommData\":{\"infoSpec\":{\"SenHub\":{\"SenData\":{\"e\":[%s],\"bn\":\"SenData\"},\"Info\":{\"e\":[%s],\"bn\":\"Info\"},\"Net\":{\"e\":[%s],\"bn\":\"Net\"},\"Action\":{\"e\":[%s],\"bn\":\"Action\"},\"opTS\":{\"$date\":%lld},\"ver\":%d}},\"commCmd\":2052,\"requestID\":2001,\"agentID\":\"%s\",\"handlerName\":\"general\",\"sendTS\":%lld}}";
//@@@ hostname[s]{Agriculture}, senData[ss], Health[d], Topology[sl], sMac[s], timestamp[d]

static const char *SEN_INFOSPEC_SENDATA_V_JSON = "{\"n\":\"%s\",\"u\":\"%s\",\"v\":%d,\"min\":%d,\"max\":%d,\"asm\":\"%s\",\"type\":\"d\",\"rt\":\"%s\",\"st\":\"ipso\",\"exten\":\"\"}";
static const char *SEN_INFOSPEC_SENDATA_FV_JSON = "{\"n\":\"%s\",\"u\":\"%s\",\"v\":%f,\"min\":%f,\"max\":%f,\"asm\":\"%s\",\"type\":\"d\",\"rt\":\"%s\",\"st\":\"ipso\",\"exten\":\"\"}";
static const char *SEN_INFOSPEC_SENDATA_SV_JSON = "{\"n\":\"%s\",\"u\":\"%s\",\"sv\":\"%s\",\"min\":%d,\"max\":%d,\"asm\":\"%s\",\"type\":\"s\",\"rt\":\"%s\",\"st\":\"ipso\",\"exten\":\"\"}";
static const char *SEN_INFOSPEC_SENDATA_BV_JSON = "{\"n\":\"%s\",\"u\":\"%s\",\"bv\":%s,\"min\":false,\"max\":true,\"asm\":\"%s\",\"type\":\"b\",\"rt\":\"%s\",\"st\":\"ipso\",\"exten\":\"\"}";
static const char *SEN_INFOSPEC_SENDATA_CV_JSON = "{\"n\":\"%s\",\"u\":\"%s\",\"sv\":\"%s\",\"min\":%d,\"max\":%d,\"asm\":\"%s\",\"type\":\"s\",\"rt\":\"%s\",\"st\":\"ipso\",\"exten\":\"\",\"format\":\"%s\"}";
/*
@@@@@ senData
{"n":"Visible","u":"lx","v":0,"min":0,"max":2000,"asm":"r","type":"d","rt":"ucum.lx","st":"ipso","exten":""}
{"n":"Infrared","u":"lx","v":0,"min":0,"max":600,"asm":"r","type":"d","rt":"ucum.lx","st":"ipso","exten":""}
=========================
{\"n\":\"%s\",\"u\":\"%s\",\"v\":%d,\"min\":%d,\"max\":%d,\"asm\":\"r\",\"type\":\"d\",\"rt\":\"%s\",\"st\":\"ipso\",\"exten\":\"\"}
{\"n\":\"%s\",\"u\":\"%s\",\"sv\":%s,\"min\":%d,\"max\":%d,\"asm\":\"r\",\"type\":\"d\",\"rt\":\"%s\",\"st\":\"ipso\",\"exten\":\"\"}
{\"n\":\"%s\",\"u\":\"%s\",\"bv\":%d,\"min\":%d,\"max\":%d,\"asm\":\"r\",\"type\":\"d\",\"rt\":\"%s\",\"st\":\"ipso\",\"exten\":\"\"}
*/


//[Sensor DEVICEINFO]
///cagent/admin/00170d00006063c2/deviceinfo
static const char *SEN_DEVINFO_JSON = "{\"susiCommData\":{\"data\":{\"SenHub\":{%s,\"opTS\":{\"$date\":%lld},\"ver\":%d}},\"commCmd\":2055,\"requestID\":2001,\"agentID\":\"%s\",\"handlerName\":\"general\",\"sendTS\":%lld}}";
//@@@ senData[ss], Health[d], Topology[sl], sMac[s], timestamp[d]

static const char *SEN_DEVINFO_SENDATA_V_JSON = "{\"n\":\"%s\",\"v\":%d}";
static const char *SEN_DEVINFO_SENDATA_FV_JSON = "{\"n\":\"%s\",\"v\":%f}";
static const char *SEN_DEVINFO_SENDATA_SV_JSON = "{\"n\":\"%s\",\"sv\":\"%s\"}";
static const char *SEN_DEVINFO_SENDATA_BV_JSON = "{\"n\":\"%s\",\"bv\":%s}";
static const char *SEN_DEVINFO_SENDATA_CV_JSON = "{\"n\":\"%s\",\"sv\":\"%s\",\"format\":\"%s\"}";
/*
@@@@@
{"n":"Visible","v":2000}
{"n":"Infrared","v":312}
==============================
{\"n\":\"%s\",\"v\":%d}
{\"n\":\"%s\",\"sv\":%s}
{\"n\":\"%s\",\"bv\":%d}
*/

//[Event Notify]
static const char *EVENT_NOTIFY = "{\"susiCommData\":{\"commCmd\":2059,\"requestID\":2001,\"agentID\":\"%s\",\"handlerName\":\"general\",\"sendTS\":%d,\"eventnotify\":{%s}}}";

/*=====================================================================================*/

/*=====================================================================================*/

/*Defining Broker IP address and port Number*/
static char projectName[65] = {0};
static char groupName[65] = {0};
static char parentId[33] = {0};
static char gatewayId[33] = {0};
static char interfaceId[33] = {0};
static char interfaceMac[33] = {0};
static char interfaceName[65] = {0};
static int interfaceNumber = -1;
static char interfaceTag[65] = {0};
static char serviceVersion[65] = {0};


long long my_get_timetick (void* userdata)
{
	long long tick = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	tick = (long long)tv.tv_sec*1000 + (long long)tv.tv_usec/1000;
	return tick;
}


void WiseAgent_Init(char *productionName, char *wanIp, unsigned char *parentMac, unsigned char *version, WiseAgentInfoSpec *infospec, int count) {
	strncpy(projectName,productionName, sizeof(projectName));
	if(parentMac != NULL) {
		if(strlen(parentMac) == 16) {
			snprintf(parentId,sizeof(parentId),"%s",parentMac); //MACv6
		} else if(strlen(parentMac) > 12){
			snprintf(parentId,sizeof(parentId),"%s",parentMac);
		} else {
            if(strlen(parentMac) != 0) {
                snprintf(parentId,sizeof(parentId),"0000%s",parentMac);
            } else {
                snprintf(parentId,sizeof(parentId),"%s",parentMac);
            }
		}
		ToUpper(parentId);
	} else {
		memset(parentId,0,sizeof(parentId));
	}
    
    if(version != NULL) {
        if(wanIp != NULL) strncpy(groupName,wanIp, sizeof(groupName));
        else memset(groupName,0,sizeof(groupName));
        strncpy(serviceVersion,version, sizeof(serviceVersion));
    } else {
        strcpy(groupName, "IoTGW");
        if(wanIp != NULL) SetDeviceIpAddress(wanIp);
}
}

void WiseAgent_RegisterInterface(char *ifMac, char *ifName, int ifNumber, WiseAgentInfoSpec *infospec, int count) {
	int index = 0;
	WiseAgentInfoSpec *is;
    char *topic = NULL;
	if(interfaceMac[0] != 0) {
		return;
	}
	
	snprintf(interfaceMac,33,"%s",ifMac);
	ToUpper(interfaceMac);

	if(strlen(interfaceMac) == 16) {
		snprintf(gatewayId,33,"%s",interfaceMac); //MACv6
	} else if(strlen(interfaceMac) > 12){
		snprintf(gatewayId,33,"%s",interfaceMac);
	} else {
        snprintf(gatewayId,33,"0000%s",interfaceMac);
	}
    printf("<%s,%d>gatewayId = %s\n", __FILE__,__LINE__,gatewayId);
	ToUpper(gatewayId);
    
    WiseAccess_Init(projectName,gatewayId);
	//SetDeviceMacAddress(ifMac);
	
	if(strlen(interfaceMac) == 16) {
		snprintf(interfaceId,33,"%s",interfaceMac); //MACv6
	} else if(strlen(interfaceMac) > 12){
		snprintf(interfaceId,33,"%s",interfaceMac);
	} else {
		snprintf(interfaceId,33,"0007%s",interfaceMac);
	}
	ToUpper(interfaceId);
	
	strncpy(interfaceName, ifName, 65);
	interfaceNumber = ifNumber;
	if(interfaceNumber == -1) {
		sprintf(interfaceTag,"%s",interfaceName);
	} else {
		sprintf(interfaceTag,"%s%d",interfaceName,interfaceNumber);
	}
	
	topic = (char *)WiseMem_Alloc(128);
    
	sprintf(topic,WA_SUB_CBK_TOPIC, interfaceId);
	core_subscribe(topic, 0);
	
    if(strlen(serviceVersion) == 0) {
        WiseAccess_InterfaceInit(interfaceId, interfaceTag, 6);
	} else {
        WiseAccess_InterfaceInit(interfaceId, interfaceTag, 0);
    }
	
	for(index = 0 ; index < count ; index++) {
		is = &infospec[index];
		if(is->name[0] == '/')
		{
			switch(is->type) {
				case WISE_VALUE:
                case WISE_FLOAT:
				case WISE_BOOL:
				case WISE_STRING:
					WiseAccess_AddItem(interfaceId, is->name, is);
					break;
				default:
					wiseprint("Infospec datatype error!!\n");
					infiniteloop();
					break;
			}
		}
	}
    
    WiseMem_Release();
}

/************************************************************/
static int gLosted = 0;
void on_connect_cb(int socketfd, char* localip, int iplength)
{
	wiseprint("CB_Connected \n");
    if(gLosted) {
        core_device_register();
        core_platform_register();
    }
    gLosted = 0;
}

void on_lostconnect_cb(int rc)
{
	wiseprint("CB_Lostconnect \n");
    gLosted = 1;
}

void on_disconnect_cb()
{
	wiseprint("CB_Disconnect \n");
}

void on_rename(const char* name, const int cmdid, const char* sessionid)
{
	wiseprint("rename to: %s\r\n", name);
	WiseAgentData data;
	data.string = (char *)name;
	SetGWName(&data);
	WiseAccess_AssignCmd(cmdid, -1, 0, 200, NULL, NULL, (char *)sessionid, NULL, NULL);
	return;
}

void on_update(const char* loginID, const char* loginPW, const int port, const char* path, const char* md5, const int cmdid, const char* sessionid)
{
	wiseprint("Update: %s, %s, %d, %s, %s\n", loginID, loginPW, port, path, md5);
	return;
}

void on_server_reconnect(const char* devid) {
	int d = WiseAccess_FindDevice(devid);
	if(WiseAccess_FindDevice(devid) > 0) {
		WiseAccess_AssignCmd(9999, d, 0, 200, devid, NULL, NULL, NULL, NULL);		
	} else {
		if(strncmp(gatewayId,devid,sizeof(gatewayId)) == 0) {
			WiseAccess_AssignCmd(9999, -1, 0, 200, devid, NULL, NULL, NULL, NULL);		
		}
	}
}

void on_query_heartbeatrate(const char* sessionid,const char* devid) {
	WiseAccess_AssignCmd(127, -1, 0, 200, NULL, devid, sessionid, NULL, NULL);	
}

void on_update_heartbeatrate(const int heartbeatrate, const char* sessionid, const char* devid) {
	WiseAgentData data;
	data.value = heartbeatrate;
	SetHeartBeatRate(&data);
	WiseAccess_AssignCmd(129, -1, 0, 200, NULL, devid, sessionid, NULL, NULL);
}

static int WiseAgent_ConnectBySSL(char *server_url, int port, char *username, char *password, WiseAgentInfoSpec *infospec, int count) {
	int iRet = 0;
	int i = 0;
	char *cafile = NULL;
	char *clientCertificate = NULL;
	char *clientKey = NULL;
	char *keyPassword = NULL;
    char *topic = NULL;
	char *message = NULL;
    char *senhublist = NULL;
    char *infoString = NULL;
	long long ts = 0;

    iRet = core_initialize(gatewayId, (char *)GetGWName(), interfaceMac);
    if(!iRet)
	{
    	wiseprint("Unable to initialize AgentCore.\n");
		wiseprint("WiseCore Error: %s\n", core_error_string_get());
		return 0;
	}
    printf("<%s,%d>gatewayId = %s\n", __FILE__,__LINE__,gatewayId);
    core_connection_callback_set(on_connect_cb, on_lostconnect_cb, on_disconnect_cb, CmdReceive);

    core_action_callback_set(on_rename, on_update);

    core_server_reconnect_callback_set(on_server_reconnect);
	
	core_heartbeat_callback_set(on_query_heartbeatrate, on_update_heartbeatrate);

	core_time_tick_callback_set (my_get_timetick);

    if(strlen(serviceVersion) == 0) {
        core_product_info_set(interfaceMac, parentId, VERSION, "IoTGW", "", "");
	core_os_info_set("SnailOS", SNAIL_MODEL, 123, interfaceMac);
    } else {
        core_product_info_set(interfaceMac, parentId, serviceVersion, "Service", projectName, "");
        core_os_info_set("SnailOS", VERSION, 123, interfaceMac);
    }

	core_platform_info_set("", "", "Snail");

	core_local_ip_set((interfaceNumber == -1 ? GetIp() : ""));

	for(i = 0 ; i < count ; i++) {
		if(strcmp(infospec[i].name, "@cafile") == 0) {
			cafile = infospec[i].string;
		}
		if(strcmp(infospec[i].name, "@clientCertificate") == 0) {
			clientCertificate = infospec[i].string;
		}
		if(strcmp(infospec[i].name, "@clientKey") == 0) {
			clientKey = infospec[i].string;
		}
		if(strcmp(infospec[i].name, "@keyPassword") == 0) {
			keyPassword = infospec[i].string;
		}
	}

	if(clientCertificate != NULL && clientKey != NULL) {
		core_tls_set(cafile, "", clientCertificate, clientKey, keyPassword);
	}
	
	iRet = core_connect(server_url, port, username, password);
	//iRet = core_connect("172.22.12.60", 1122, "", "");
	if(iRet){
		wiseprint("Connect to broker: %s\n", server_url);
	} else {
		wiseprint("Unable to connect to broker.\n");
		wiseprint("WiseCore Error: %s\n", core_error_string_get());
		return 0;
	}

	core_device_register();
	core_platform_register();
	
    //topic = (char *)WiseMem_Alloc(128);
	//Sub
    //sprintf(topic,WA_SUB_CBK_TOPIC, gatewayId);
    //WiseAccess_Register(topic);
    //core_subscribe(topic, 0);

	//
	// publish
	//
	/*sprintf(topic, WA_PUB_CONNECT_TOPIC, GW_MACADDRESS);
	sprintf(message,CONNECT_JSON, GW_MACADDRESS, GetGWName(), MACADDRESS, MACADDRESS, "anonymous", "", 1, GW_MACADDRESS, timestamp++);
	core_publish(topic, message, strlen(message), 0, 0);*/

	/*sprintf(topic, WA_PUB_ACTION_TOPIC, GW_MACADDRESS);
	sprintf(message,OSINFO_JSON, MACADDRESS_N, GetIp(), GW_MACADDRESS, timestamp++);
	core_publish(topic, message, strlen(message), 0, 0);*/

    topic = (char *)WiseMem_Alloc(128);
    message = (char *)WiseMem_Alloc(8192);
    senhublist = (char *)WiseMem_Alloc(4096);
    infoString = (char *)WiseMem_Alloc(1024);
    printf("<%s,%d>gatewayId = %s\n", __FILE__,__LINE__,gatewayId);
	sprintf(topic, WA_PUB_ACTION_TOPIC, gatewayId);
    printf("<%s,%d>gatewayId = %s\n", __FILE__,__LINE__,gatewayId);
	sprintf(senhublist, "%s", interfaceId);
	WiseAccess_GenerateTokenCapability(interfaceId, "Info", infoString, WiseMem_Size(infoString));
	ts = my_get_timetick(NULL);
	sprintf(message,INFOSPEC_JSON, groupName ,interfaceName, interfaceId/*interfaceTag*/, infoString, interfaceId, interfaceName, ts, gJsonFmtVer, gatewayId, ts);
	wiseprint("topic[%d]:\033[33m\"%s\"\033[0m\r\n", strlen(topic), topic);
	wiseprint("message:\033[33m\"%s\"\033[0m\r\n", message);
	core_publish(topic, message, strlen(message), 0, 0);
	
	sprintf(topic, WA_PUB_DEVINFO_TOPIC, gatewayId);
	WiseAccess_GenerateTokenDataInfo(interfaceId, "Info", senhublist, WiseMem_Size(senhublist));
    sprintf(infoString, INFO_JSON, senhublist);
	ts = my_get_timetick(NULL);
	sprintf(message,DEVICEINFO_JSON, groupName, interfaceName, interfaceId/*interfaceTag*/, infoString, interfaceId, interfaceName, ts, gJsonFmtVer, gatewayId, ts);
	wiseprint("topic[%d]:\033[33m\"%s\"\033[0m\r\n", strlen(topic), topic);
	wiseprint("message:\033[33m\"%s\"\033[0m\r\n", message);
	core_publish(topic, message, strlen(message), 0, 0);

    WiseMem_Release();
	return 1;
}

int WiseAgent_Connect(char *server_url, int port, char *username, char *password, WiseAgentInfoSpec *infospec, int count) {
	return WiseAgent_ConnectBySSL(server_url, port, username, password, infospec, count);
}

int WiseAgent_PublishInterfaceInfoSpecMessage(char *gatewayId) {
    char *topic = (char *)WiseMem_Alloc(128);
    char *message = (char *)WiseMem_Alloc(8192);
    char *senhublist = (char *)WiseMem_Alloc(4096);
    char *infoString = (char *)WiseMem_Alloc(1024);
	long long ts = 0;

	sprintf(topic, WA_PUB_ACTION_TOPIC, gatewayId);
	sprintf(senhublist, "%s", interfaceId);
	WiseAccess_GenerateTokenCapability(interfaceId, "Info", infoString, WiseMem_Size(infoString));
	ts = my_get_timetick(NULL);
	sprintf(message,INFOSPEC_JSON, groupName, interfaceName, interfaceId/*interfaceTag*/, infoString, interfaceId, interfaceName, ts, gJsonFmtVer, gatewayId, ts);
	wiseprint("topic[%d]:\033[33m\"%s\"\033[0m\r\n", strlen(topic), topic);
	wiseprint("message:\033[33m\"%s\"\033[0m\r\n", message);
	core_publish(topic, message, strlen(message), 0, 0);
    WiseMem_Release();
}

int WiseAgent_PublishInterfaceDeviceInfoMessage(char *gatewayId) {
    WiseAccess_GetTopology();
    
    char *topic = (char *)WiseMem_Alloc(128);
    char *message = (char *)WiseMem_Alloc(8192);
    char *senhublist = (char *)WiseMem_Alloc(4096);
    char *infoString = (char *)WiseMem_Alloc(1024);
	long long ts = 0;
    
	sprintf(topic, WA_PUB_DEVINFO_TOPIC, gatewayId);
	WiseAccess_GenerateTokenDataInfo(interfaceId, "Info", senhublist, WiseMem_Size(infoString));
    sprintf(infoString, INFO_JSON, senhublist);
	ts = my_get_timetick(NULL);
	sprintf(message,DEVICEINFO_JSON, groupName, interfaceName, interfaceId/*interfaceTag*/, infoString, interfaceId, interfaceName, ts, gJsonFmtVer, gatewayId, ts);
	wiseprint("topic[%d]:\033[33m\"%s\"\033[0m\r\n", strlen(topic), topic);
	wiseprint("message:\033[33m\"%s\"\033[0m\r\n", message);
	core_publish(topic, message, strlen(message), 0, 0);
    WiseMem_Release();
}

int WiseAgent_PublishSensorConnectMessage(char *deviceId) {
    char *topic = (char *)WiseMem_Alloc(128);
    char *message = (char *)WiseMem_Alloc(8192);
	sprintf(topic, WA_PUB_CONNECT_TOPIC, deviceId);
	WiseAgentData shname;
	WiseAccess_Get(deviceId, "/Info/Name", &shname);
	sprintf(message,SEN_CONNECT_JSON, deviceId, shname.string, deviceId, deviceId, deviceId, my_get_timetick(NULL));
	wiseprint("topic[%d]:\033[33m\"%s\"\033[0m\r\n", strlen(topic), topic);
	wiseprint("message:\033[33m\"%s\"\033[0m\r\n", message);
	core_publish(topic, message, strlen(message), 0, 0);
    
    sprintf(topic,WA_SUB_CBK_TOPIC, deviceId);
	core_subscribe(topic, 0);
    
    WiseMem_Release();
}

int WiseAgent_PublishSensorDisconnectMessage(char *deviceId) {
    char *topic = (char *)WiseMem_Alloc(128);
    char *message = (char *)WiseMem_Alloc(8192);
	sprintf(topic, WA_PUB_CONNECT_TOPIC, deviceId);
	WiseAgentData shname;
	WiseAccess_Get(deviceId, "/Info/Name", &shname);
	sprintf(message,SEN_DISCONNECT_JSON, deviceId, shname.string, deviceId, deviceId, deviceId, my_get_timetick(NULL));
	wiseprint("topic[%d]:\033[33m\"%s\"\033[0m\r\n", strlen(topic), topic);
	wiseprint("message:\033[33m\"%s\"\033[0m\r\n", message);
	core_publish(topic, message, strlen(message), 0, 0);
    
    sprintf(topic,WA_SUB_CBK_TOPIC, deviceId);
	core_subscribe(topic, 0);
    
    WiseMem_Release();
}

void WiseAgent_RegisterSensor(char *deviceMac, char *defaultName, WiseAgentInfoSpec *infospec, int count) {
	int number = 0;
	int index = 0;
    char *topic = NULL;
    char *message = NULL;
    char *senhublist = NULL;
    char *infoString = NULL;
    char *netString = NULL;
    char *actionString = NULL;
	char *pos = senhublist;
	char *access = "rw";
	char deviceId[33] = {0};
    char formatBuffer[256];
	long long ts = 0;
    
	WiseAgentInfoSpec *is;
	
	if(strlen(deviceMac) == 16) {
		snprintf(deviceId,33,"%s",deviceMac); //MACv6
	} else if(strlen(deviceMac) > 12){
		snprintf(deviceId,33,"%s",deviceMac);
	} else {
		snprintf(deviceId,33,"0017%s",deviceMac);
	}
	ToUpper(deviceId);
	
	//sprintf(topic,WA_SUB_CBK_TOPIC, deviceId);
	//core_subscribe(topic, 0);
	
	WiseAccess_SensorInit(deviceId, defaultName);
    WiseMem_Release();
	WiseAgent_PublishInterfaceDeviceInfoMessage(gatewayId);
	
    for(index = 0 ; index < count ; index++) {
		is = &infospec[index];
		if(is->name[0] == '/') {
			switch(is->type) {
				case WISE_VALUE:
                case WISE_FLOAT:
				case WISE_BOOL:
				case WISE_STRING:
                case WISE_CUSTOMIZE:
					WiseAccess_AddItem(deviceId, is->name, is);
					break;
				default:
					wiseprint("Infospec datatype error!!\n");
					infiniteloop();
					break;
			}
		}
	}
    
    WiseAccess_ConnectionStatus(deviceId, 1);

	WiseAgent_PublishSensorConnectMessage(deviceId);

    topic = (char *)WiseMem_Alloc(128);
    message = (char *)WiseMem_Alloc(8192);
    senhublist = (char *)WiseMem_Alloc(4096);
    infoString = (char *)WiseMem_Alloc(1024);
    netString = (char *)WiseMem_Alloc(1024);
    actionString = (char *)WiseMem_Alloc(1024);
	pos = senhublist;
    
	for(index = 0 ; index < count ; index++) {
		is = &infospec[index];
		if(is->name[0] != '/') {
			if(number >= 1) pos += sprintf(pos,",");
			number++;
            
			if(is->setValue == NULL) {
				access = "r";
			} else {
				access = "rw";
				if(is->type == WISE_STRING) {
					if(is->getValue == NULL) {
						access = "r";
					}
				}
			}
			switch(is->type) {
				case WISE_VALUE:
					WiseAccess_AddItem(deviceId, is->name, is);
					pos += sprintf(pos, SEN_INFOSPEC_SENDATA_V_JSON, is->name, NULL_STRING(is->unit), (int)is->value, (int)is->min, (int)is->max, access, NULL_STRING(is->resourcetype));
					break;
                case WISE_FLOAT:
					WiseAccess_AddItem(deviceId, is->name, is);
					pos += sprintf(pos, SEN_INFOSPEC_SENDATA_FV_JSON, is->name, NULL_STRING(is->unit), is->value, is->min, is->max, access, NULL_STRING(is->resourcetype));
					break;
				case WISE_BOOL:
					WiseAccess_AddItem(deviceId, is->name, is);
					pos += sprintf(pos, SEN_INFOSPEC_SENDATA_BV_JSON, is->name, NULL_STRING(is->unit), is->value > 0 ? "true" : "false", access, NULL_STRING(is->resourcetype));
					break;
				case WISE_STRING:
                    WiseAccess_AddItem(deviceId, is->name, is);
                    pos += sprintf(pos, SEN_INFOSPEC_SENDATA_SV_JSON, is->name, NULL_STRING(is->unit), NULL_STRING(is->string), (int)is->min, (int)is->max, access, NULL_STRING(is->resourcetype));
                    break;
                case WISE_CUSTOMIZE:
					WiseAccess_AddItem(deviceId, is->name, is);
                    switch(is->format) {
                        default:
                        case WISE_BASE64:
                            base64_encode_padding(formatBuffer, is->raw->data, is->raw->len);
                            pos += sprintf(pos, SEN_INFOSPEC_SENDATA_CV_JSON, is->name, NULL_STRING(is->unit), formatBuffer, (int)is->min, (int)is->max, access, NULL_STRING(is->resourcetype), "base64");
                        break;
                    }
					break;
				default:
					wiseprint("Infospec datatype error!!\n");
					infiniteloop();
					break;
			}
		}
	}
	
	sprintf(topic, WA_PUB_ACTION_TOPIC, deviceId);
	wiseprint("senhublist:\033[33m\"%s\"\033[0m\r\n", senhublist);
	WiseAccess_GenerateTokenCapability(deviceId, "Info", infoString, WiseMem_Size(infoString));
	WiseAccess_GenerateTokenCapability(deviceId, "Net", netString, WiseMem_Size(netString));
	WiseAccess_GenerateTokenCapability(deviceId, "Action", actionString, WiseMem_Size(actionString));
	wiseprint("infoString:\033[33m\"%s\"\033[0m\r\n", infoString);
	wiseprint("netString:\033[33m\"%s\"\033[0m\r\n", netString);
	wiseprint("actionString:\033[33m\"%s\"\033[0m\r\n", actionString);
	ts = my_get_timetick(NULL);
	sprintf(message,SEN_INFOSPEC_JSON, senhublist, infoString, netString, actionString, ts, gJsonFmtVer, deviceId, ts);
	wiseprint("topic[%d]:\033[33m\"%s\"\033[0m\r\n", strlen(topic), topic);
	wiseprint("message:\033[33m\"%s\"\033[0m\r\n", message);
	core_publish(topic, message, strlen(message), 0, 0);
	WiseAccess_SetInfoSpec(deviceId, message, strlen(message)+1);
    WiseMem_Release();
}

void WiseAgent_SenHubDisconnect(char *deviceMac) {
	char deviceId[33] = {0};

	if(strlen(deviceMac) == 16) {
		snprintf(deviceId,33,"%s",deviceMac); //MACv6
	} else if(strlen(deviceMac) > 12){
		snprintf(deviceId,33,"%s",deviceMac);
	} else {
		snprintf(deviceId,33,"0017%s",deviceMac);
	}
	
	ToUpper(deviceId);
	if(WiseAccess_FindDevice(deviceId) < 0) return;
	
	WiseAccess_ConnectionStatus(deviceId, 0);
	
	WiseAgent_PublishInterfaceDeviceInfoMessage(gatewayId);
    
    WiseAgent_PublishSensorDisconnectMessage(deviceId);
}

void WiseAgent_SenHubReConnected(char *deviceMac) {
	char deviceId[33] = {0};

	if(strlen(deviceMac) == 16) {
		snprintf(deviceId,33,"%s",deviceMac); //MACv6
	} else if(strlen(deviceMac) > 12){
		snprintf(deviceId,33,"%s",deviceMac);
	} else {
		snprintf(deviceId,33,"0017%s",deviceMac);
	}
	
	ToUpper(deviceId);
	if(WiseAccess_FindDevice(deviceId) < 0) return;
	
	WiseAccess_ConnectionStatus(deviceId, 1);
	
	WiseAgent_PublishInterfaceDeviceInfoMessage(gatewayId);
    
    WiseAgent_PublishSensorConnectMessage(deviceId);
}


void WiseAgent_Write(char *deviceMac, WiseAgentData* data, int count) {
	int number = 0;
	int index = 0;
	char *pos = NULL;
	WiseAgentData* d;
	int otherInfo = 0;
	char deviceId[33] = {0};
    char formatBuffer[1024];
    char *topic = NULL;
	char *message = NULL;
    char *senhublist = NULL;
    char *infoString = NULL;
    char *netString = NULL;
    char *actionString = NULL;
	long long ts = 0;

	if(strlen(deviceMac) == 16) {
		snprintf(deviceId,33,"%s",deviceMac); //MACv6
	} else if(strlen(deviceMac) > 12){
		snprintf(deviceId,33,"%s",deviceMac);
	} else {
		snprintf(deviceId,33,"0017%s",deviceMac);
	}
	printf("deviceId = %s\n", deviceId);
	ToUpper(deviceId);
	if(WiseAccess_FindDevice(deviceId) < 0) return;
	
    topic = (char *)WiseMem_Alloc(1024);
    message = (char *)WiseMem_Alloc(8192);
    senhublist = (char *)WiseMem_Alloc(4096);
    infoString = (char *)WiseMem_Alloc(1024);
    netString = (char *)WiseMem_Alloc(1024);
    actionString = (char *)WiseMem_Alloc(1024);
    
	for(index = 0 ; index < count ; index++) {
        d = &data[index];
		if(d->name[0] != '/') {
			//sprintf(message,"/SenData/%s",d->name);
			sprintf(message,"%s",d->name);
		} else {
			sprintf(message,"%s",d->name);
			otherInfo++;
		}
        WiseAccess_Update(deviceId, message, d);
    }
	
	
	message[0] = 0;
    pos = message;
	for(index = 0 ; index < count ; index++) {
        printf("<%s,%d>index = %d(%d)\n", __FILE__,__LINE__, index, count);
		d = &data[index];
		if(d->name[0] != '/') {
			if(number != 0) pos += sprintf(pos,",");
            
            printf("<%s,%d>d->type = %d\n", __FILE__,__LINE__, d->type);
			switch(d->type) {
			case WISE_VALUE:
				pos += sprintf(pos, SEN_DEVINFO_SENDATA_V_JSON, d->name, (int)d->value);
				break;
            case WISE_FLOAT:
				pos += sprintf(pos, SEN_DEVINFO_SENDATA_FV_JSON, d->name, d->value);
				break;
            case WISE_BOOL:
				pos += sprintf(pos, SEN_DEVINFO_SENDATA_BV_JSON, d->name, d->value > 0 ? "true" : "false");
				break;
			case WISE_STRING:
				pos += sprintf(pos, SEN_DEVINFO_SENDATA_SV_JSON, d->name, NULL_STRING(d->string));
				break;
            case WISE_CUSTOMIZE:
                switch(d->format) {
                    default:
                    case WISE_BASE64:
                        base64_encode_padding(formatBuffer, d->raw->data, d->raw->len);
                        pos += sprintf(pos, SEN_DEVINFO_SENDATA_CV_JSON, d->name, formatBuffer, "base64");
                    break;
                }
                break;
			default:
				wiseprint("Datatype error!!\n");
				infiniteloop();
				break;
			}
			number++;
		}
	}

	if(strcmp(deviceId, gatewayId) == 0) {
        if(otherInfo) {
            WiseAccess_GenerateTokenDataInfo(deviceId, "Info", topic, WiseMem_Size(infoString));
            if(strlen(topic) != 0) sprintf(infoString, INFO_JSON, topic);
            WiseAccess_GenerateTokenDataInfo(deviceId, "Net", topic, WiseMem_Size(netString));
            if(strlen(topic) != 0) sprintf(netString, NET_JSON, topic);
            WiseAccess_GenerateTokenDataInfo(deviceId, "Action", topic, WiseMem_Size(actionString));
            if(strlen(topic) != 0) sprintf(actionString, ACTION_JSON, topic);
            
            pos = senhublist;
            if(strlen(message) != 0) pos += sprintf(pos, SENDATA_JSON, message);
            if(strlen(infoString) != 0) {
                if(pos != senhublist) pos += sprintf(pos, ",");
                pos += sprintf(pos, "%s", infoString);
            }
            
            if(strlen(netString) != 0) {
                if(pos != senhublist) pos += sprintf(pos, ",");
                pos += sprintf(pos, "%s", netString);
            }
            
            if(strlen(actionString) != 0) {
                if(pos != senhublist) pos += sprintf(pos, ",");
                pos += sprintf(pos, "%s", actionString);
            }
			ts = my_get_timetick(NULL);
			sprintf(message,DEVICEINFO_JSON, groupName, interfaceName, interfaceId/*interfaceTag*/, senhublist, interfaceId, interfaceName, ts, gJsonFmtVer, gatewayId, ts);
        } else {
            pos = senhublist;
            if(strlen(message) != 0) pos += sprintf(pos, SENDATA_JSON, message);
			ts = my_get_timetick(NULL);
			sprintf(message,DEVICEINFO_JSON, groupName, interfaceName, interfaceId/*interfaceTag*/, senhublist, interfaceId, interfaceName, ts, gJsonFmtVer, gatewayId, ts);
        }
    } else {
        if(otherInfo) {
            WiseAccess_GenerateTokenDataInfo(deviceId, "Info", topic, WiseMem_Size(infoString));
            if(strlen(topic) != 0) sprintf(infoString, INFO_JSON, topic);
            WiseAccess_GenerateTokenDataInfo(deviceId, "Net", topic, WiseMem_Size(netString));
            if(strlen(topic) != 0) sprintf(netString, NET_JSON, topic);
            WiseAccess_GenerateTokenDataInfo(deviceId, "Action", topic, WiseMem_Size(actionString));
            if(strlen(topic) != 0) sprintf(actionString, ACTION_JSON, topic);
            
            pos = senhublist;
            if(strlen(message) != 0) pos += sprintf(pos, SENDATA_JSON, message);
            if(strlen(infoString) != 0) {
                if(pos != senhublist) pos += sprintf(pos, ",");
                pos += sprintf(pos, "%s", infoString);
            }
            
            if(strlen(netString) != 0) {
                if(pos != senhublist) pos += sprintf(pos, ",");
                pos += sprintf(pos, "%s", netString);
            }
            
            if(strlen(actionString) != 0) {
                if(pos != senhublist) pos += sprintf(pos, ",");
                pos += sprintf(pos, "%s", actionString);
            }

			ts = my_get_timetick(NULL);
			sprintf(message,SEN_DEVINFO_JSON, senhublist, ts, gJsonFmtVer, deviceId, ts);
        } else {
            pos = senhublist;
            if(strlen(message) != 0) pos += sprintf(pos, SENDATA_JSON, message);

			ts = my_get_timetick(NULL);
			sprintf(message,SEN_DEVINFO_JSON, senhublist, ts, gJsonFmtVer, deviceId, ts);
        }
    }
    sprintf(topic, WA_PUB_DEVINFO_TOPIC, deviceId);
    wiseprint("topic[%d]:\033[33m\"%s\"\033[0m\r\n", strlen(topic), topic);
	wiseprint("message[%d]:\033[33m\"%s\"\033[0m\r\n", strlen(message), message);
	core_publish(topic, message, strlen(message), 0, 0);
    WiseMem_Release();
}

void WiseAgent_Get(char *deviceMac, char *name, WiseAgentData *data) {
	char deviceId[33] = {0};
	if(strlen(deviceMac) == 16) {
		snprintf(deviceId,33,"%s",deviceMac); //MACv6
	} else if(strlen(deviceMac) > 12){
		snprintf(deviceId,33,"%s",deviceMac);
	} else {
		snprintf(deviceId,33,"0017%s",deviceMac);
	}
	
	ToUpper(deviceId);
	if(WiseAccess_FindDevice(deviceId) < 0) return;
	
    WiseAccess_Get(deviceId, name,data);
}

void WiseAgent_Cmd_Handler() {
    WiseAccess_Handler();
}

void WiseAgent_Close() {
	//WiseMQTT_Close();
}

//Advance Functions
void WiseAgent_Publish(const char *topic, const char *msg, const int msglen, const int retain, const int qos) {
	//WiCar_MQTT_Publish(topic, msg, msglen, retain, qos);
}
