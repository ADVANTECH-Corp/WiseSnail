/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/03/01 by Scott Chang								    */
/* Modified Date: 2016/03/01 by Scott Chang									*/
/* Abstract     : WISE Core API definition									*/
/* Reference    : None														*/
/****************************************************************************/
#include "WISECore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <time.h>
#include "liteparse.h"
#include "version.h"
#include "WISEConnector.h"
#include "susiaccess_def.h"

#ifdef WIN32
	#define snprintf(dst,size,format,...) _snprintf(dst,size,format,##__VA_ARGS__)
#endif

typedef struct{
	/*clinet info, user must define*/
	char strClientID[DEF_DEVID_LENGTH];
	char strHostName[DEF_HOSTNAME_LENGTH];
	char strMAC[DEF_MAC_LENGTH];

	/*product info*/
	char strSerialNum[DEF_SN_LENGTH];
	char strVersion[DEF_MAX_STRING_LENGTH];
	char strType[DEF_MAX_STRING_LENGTH];
	char strProduct[DEF_MAX_STRING_LENGTH];
	char strManufacture[DEF_MAX_STRING_LENGTH];
	char strParentID[DEF_DEVID_LENGTH];

	/*os info*/
	char strOSName[DEF_OSVERSION_LEN];
	char strOSArch[DEF_FILENAME_LENGTH];
	int iTotalPhysMemKB;
	char strMACs[DEF_MAC_LENGTH*16];
	char strLocalIP[DEF_MAX_STRING_LENGTH];

	/*platform info*/
	char strBIOSVersion[DEF_VERSION_LENGTH];
	char strPlatformName[DEF_FILENAME_LENGTH];
	char strProcessorName[DEF_PROCESSOR_NAME_LEN];

	/*account bind*/
	char strLoginID[DEF_USER_PASS_LENGTH];
	char strLoginPW[DEF_USER_PASS_LENGTH];

	/*client status*/
	lite_conn_status iStatus;
	int pSocketfd;
}core_contex_t;

typedef enum{
	wise_unknown_cmd = 0,
	wise_agentinfo_cmd = 21,
	//--------------------------Global command define(101--130)--------------------------------
	wise_update_cagent_req = 111,
	wise_update_cagent_rep,
	wise_cagent_rename_req = 113,
	wise_cagent_rename_rep,
	wise_cagent_osinfo_rep = 116,
	wise_server_control_req = 125,
	wise_server_control_rep,

	wise_heartbeatrate_query_req = 127,
	wise_heartbeatrate_query_rep = 128,
	wise_heartbeatrate_update_req = 129,
	wise_heartbeatrate_update_rep = 130,

	wise_error_rep = 600,

	wise_info_spec_req = 2051,
	wise_info_spec_rep = 2052,
	wise_start_auto_upload_req = 2053,
	wise_start_auto_upload_rep = 2054,
	wise_info_upload_rep = 2055,
	wise_stop_auto_upload_req = 2056,
	wise_stop_auto_upload_rep = 2057,
}wise_comm_cmd_t;

typedef enum {
	core_success = 0,               // No error.
	core_param_error,
	core_no_init, 
	core_no_connnect,
	core_buff_not_enough,
	core_internal_error,
	core_not_support = 5000
} core_result;

#define DEF_GENERAL_HANDLER					"\"handlerName\":\"general\""
#define DEF_SERVERREDUNDANCY_HANDLER		"\"handlerName\":\"ServerRedundancy\""
#define DEF_GENERAL_REQID					"\"requestID\":109"
#define DEF_WISE_COMMAND					"\"commCmd\":%d"
#define DEF_SERVERCTL_STATUS				"\"statuscode\":%d"
#define DEF_WISE_TIMESTAMP					"\"sendTS\":%d"

#define DEF_WILLMSG_TOPIC					"/cagent/admin/%s/willmessage"	/*publish*/
#define DEF_INFOACK_TOPIC					"/cagent/admin/%s/agentinfoack"	/*publish*/
#define DEF_AGENTINFO_JSON					"{\"susiCommData\":{\"devID\":\"%s\",\"parentID\":\"%s\",\"hostname\":\"%s\",\"sn\":\"%s\",\"mac\":\"%s\",\"version\":\"%s\",\"type\":\"%s\",\"product\":\"%s\",\"manufacture\":\"%s\",\"account\":\"%s\",\"passwd\":\"%s\",\"status\":%d,\"commCmd\":1,\"requestID\":21,\"agentID\":\"%s\",\"handlerName\":\"general\",\"sendTS\":%lld}}"
#define DEF_AGENTACT_TOPIC					"/cagent/admin/%s/agentactionreq"	/*publish*/
#define DEF_AGENTCAPABILITY_TOPIC			"/cagent/admin/%s/deviceinfo"	/*publish*/
#define DEF_EVENTNOTIFY_TOPIC				"/cagent/admin/%s/eventnotify"	/*publish*/
#define DEF_CALLBACKREQ_TOPIC				"/cagent/admin/%s/agentcallbackreq"	/*Subscribe*/
#define DEF_ACTIONACK_TOPIC					"/cagent/admin/%s/agentactionack"	/*Subscribe*/
#define DEF_AGENTCONTROL_TOPIC				"/server/admin/+/agentctrl"	/*Subscribe*/
#define DEF_HEARTBEAT_TOPIC					"/cagent/admin/%s/notify"	/*publish*/
#define DEF_OSINFO_JSON						"{\"susiCommData\":{\"osInfo\":{\"cagentVersion\":\"%s\",\"cagentType\":\"%s\",\"osVersion\":\"%s\",\"biosVersion\":\"%s\",\"platformName\":\"%s\",\"processorName\":\"%s\",\"osArch\":\"%s\",\"totalPhysMemKB\":%d,\"macs\":\"%s\",\"IP\":\"%s\"},\"commCmd\":116,\"requestID\":109,\"agentID\":\"%s\",\"handlerName\":\"general\",\"sendTS\":%lld}}"
#define DEF_ACTION_RESPONSE_SESSION_JSON	"{\"susiCommData\":{\"commCmd\":%d,\"catalogID\":4,\"handlerName\":\"general\",\"result\":\"%s\",\"sessionID\":\"%s\"}}"
#define DEF_ACTION_RESPONSE_JSON			"{\"susiCommData\":{\"commCmd\":%d,\"catalogID\":4,\"handlerName\":\"general\",\"result\":\"%s\"}}"
#define DEF_HEARTBEAT_MESSAGE_JSON			"{\"hb\":{\"devID\":\"%s\"}}"
#define DEF_HEARTBEATRATE_RESPONSE_SESSION_JSON	"{\"susiCommData\":{\"commCmd\":%d,\"catalogID\":4,\"handlerName\":\"general\",\"heartbeatrate\":%d,\"sessionID\":\"%s\"}}"

CORE_CONNECTED_CALLBACK g_on_connect_cb = NULL;
CORE_LOSTCONNECTED_CALLBACK g_on_lostconnect_cb = NULL;
CORE_DISCONNECT_CALLBACK g_on_disconnect_cb = NULL;
CORE_MESSAGE_RECV_CALLBACK g_on_msg_recv_cb = NULL;
CORE_RENAME_CALLBACK g_on_rename_cb = NULL;
CORE_UPDATE_CALLBACK g_on_update_cb = NULL;
CORE_SERVER_RECONNECT_CALLBACK g_on_server_reconnect = NULL;
CORE_GET_CAPABILITY_CALLBACK g_on_get_capability = NULL;
CORE_START_REPORT_CALLBACK g_on_start_report = NULL;
CORE_STOP_REPORT_CALLBACK g_on_stop_report = NULL;
CORE_QUERY_HEARTBEATRATE_CALLBACK g_on_query_heartbeatrate = NULL;
CORE_UPDATE_HEARTBEATRATE_CALLBACK g_on_update_heartbeatrate = NULL;
CORE_GET_TIME_TICK_CALLBACK g_on_get_timetick = NULL;

core_contex_t g_tHandleCtx;
bool g_bInited = false;

static char strPayloadBuff[4096] = {0};
static char strTopicBuff[128] = {0};
static int g_iErrorCode;
static long long g_tick = 0;

bool _get_agentinfo_string(core_contex_t* pHandle, lite_conn_status iStatus, char* strInfo, int iLength)
{
	int iRet = 0;
	
	long long tick = 0;

	if(!pHandle)
	{
		g_iErrorCode = core_param_error;
		return false;
	}

	if(!strInfo)
	{
		g_iErrorCode = core_param_error;
		return false;
	}

	if(g_on_get_timetick)
		tick = g_on_get_timetick();
	else
	{
		//tick = (long long) time((time_t *) NULL);
		tick = g_tick;
		g_tick++;
	}

	iRet = snprintf(strInfo, iLength, DEF_AGENTINFO_JSON, pHandle->strClientID?pHandle->strClientID:"",
												 pHandle->strParentID?pHandle->strParentID:"",
												 pHandle->strHostName?pHandle->strHostName:"",
												 pHandle->strSerialNum?pHandle->strSerialNum:(pHandle->strMAC?pHandle->strMAC:""),
												 pHandle->strMAC?pHandle->strMAC:"",
												 pHandle->strVersion?pHandle->strVersion:"",
												 pHandle->strType?pHandle->strType:"IPC",
												 pHandle->strProduct?pHandle->strProduct:"",
												 pHandle->strManufacture?pHandle->strManufacture:"",
												 pHandle->strLoginID?pHandle->strLoginID:"anonymous",
												 pHandle->strLoginPW?pHandle->strLoginPW:"",
												 iStatus,
												 pHandle->strClientID,
												 tick);
	if(iRet>=0)
	{
		g_iErrorCode = core_success;
		return true;
	}
	else
	{
		g_iErrorCode = core_buff_not_enough;
		return false;
	}
}

bool _send_agent_connect(core_contex_t* pHandle)
{
	if(pHandle->iStatus != core_online)
	{
		g_iErrorCode = core_no_connnect;
		return false;
	}

	if(!_get_agentinfo_string(pHandle, core_online, strPayloadBuff, sizeof(strPayloadBuff)))
		return false;
	sprintf(strTopicBuff, DEF_INFOACK_TOPIC, pHandle->strClientID);
	
	if(wc_publish((char *)strTopicBuff, strPayloadBuff, strlen(strPayloadBuff), true, 0))
	{
		g_iErrorCode = core_success;
		return true;
	}
	else
	{
		g_iErrorCode = core_internal_error;
		return false;
	}
}

bool _send_agent_disconnect(core_contex_t* pHandle)
{
	if(pHandle->iStatus != core_online)
	{
		g_iErrorCode = core_no_connnect;
		return false;
	}

	if(!_get_agentinfo_string(pHandle, core_offline, strPayloadBuff, sizeof(strPayloadBuff)))
		return false;
	sprintf(strTopicBuff, DEF_INFOACK_TOPIC, pHandle->strClientID);

	if(wc_publish((char *)strTopicBuff, strPayloadBuff, strlen(strPayloadBuff), true, 0))
	{
		g_iErrorCode = core_success;
		return true;
	}
	else
	{
		g_iErrorCode = core_internal_error;
		return false;
	}
}

bool _send_os_info(core_contex_t* pHandle)
{
	long long tick = 0;
	if(pHandle->iStatus != core_online)
	{
		g_iErrorCode = core_no_connnect;
		return false;
	}

	if(g_on_get_timetick)
		tick = g_on_get_timetick();
	else
	{
		//tick = (long long) time((time_t *) NULL);
		tick = g_tick;
		g_tick++;
	}

	snprintf(strPayloadBuff, sizeof(strPayloadBuff), DEF_OSINFO_JSON, pHandle->strVersion?pHandle->strVersion:"",
												 pHandle->strType?pHandle->strType:"IPC",
												 pHandle->strOSName?pHandle->strOSName:"",
												 pHandle->strBIOSVersion?pHandle->strBIOSVersion:"",
												 pHandle->strPlatformName?pHandle->strPlatformName:"",
												 pHandle->strProcessorName?pHandle->strProcessorName:"",
												 pHandle->strOSArch?pHandle->strOSArch:"",
												 pHandle->iTotalPhysMemKB,
												 pHandle->strMACs?pHandle->strMACs:pHandle->strMAC,
												 pHandle->strLocalIP?pHandle->strLocalIP:"",
												 pHandle->strClientID,
												 tick);

	sprintf(strTopicBuff, DEF_AGENTACT_TOPIC, pHandle->strClientID);
	
	if(wc_publish((char *)strTopicBuff, strPayloadBuff, strlen(strPayloadBuff), true, 0))
	{
		g_iErrorCode = core_success;
		return true;
	}
	else
	{
		g_iErrorCode = core_internal_error;
		return false;
	}
}

void _on_connect_cb(void *pUserData)
{
	core_contex_t* pHandle = (core_contex_t*)pUserData;
		
	if(pHandle)
	{
		pHandle->pSocketfd = 0;
		pHandle->iStatus = core_online;

		if(strlen(pHandle->strLocalIP)<=0)
		{
			char strLocalIP[16] = {0};
			if(wc_address_get(strLocalIP))
			{
				strncpy(pHandle->strLocalIP, strLocalIP, sizeof(pHandle->strLocalIP));
			}
		}
	}

	if(g_on_connect_cb)
		g_on_connect_cb();
}

void _on_lostconnect_cb(void *pUserData)
{
	core_contex_t* pHandle = (core_contex_t*)pUserData;

	if(pHandle)
	{
		pHandle->pSocketfd = -1;
		pHandle->iStatus = core_offline;
	}

	if(g_on_lostconnect_cb)
		g_on_lostconnect_cb();
}

void _on_disconnect_cb(void *pUserData)
{
	core_contex_t* pHandle = (core_contex_t*)pUserData;

	if(pHandle)
	{
		pHandle->iStatus = core_offline;
		pHandle->pSocketfd = -1;
	}

	if(g_on_disconnect_cb)
		g_on_disconnect_cb();
}

void _on_rename(core_contex_t* pHandle, char* cmd, const char* strDevID)
{
	// {"susiCommData":{"commCmd":113,"catalogID":4,"handlerName":"general","sessionID":"0BD843BFB2A34E60A56C3B686BB41C90", "devName":"TestClient_123"}}
	char strName[DEF_HOSTNAME_LENGTH] = {0};
	char strSessionID[33] = {0};
	lp_value_get(cmd, "sessionID", strSessionID, sizeof(strSessionID));
	lp_value_get(cmd, "devName", strName, sizeof(strName));

	strncpy(pHandle->strHostName, strName, sizeof(pHandle->strHostName));

	if(g_on_rename_cb)
		g_on_rename_cb(strName, wise_cagent_rename_rep, strSessionID, strDevID);
}

void _on_update(core_contex_t* pHandle, char* cmd, const char* strDevID)
{
	/*{"susiCommData":{"commCmd":111,"catalogID":4,"requestID":16,"params":{"userName":"sa30Read","pwd":"sa30Read","port":2121,"path":"/upgrade/SA30Agent_V3.0.15.exe","md5":"758C9D0A8654A93D09F375D33E262507"}}}*/
	char strUserName[DEF_USER_PASS_LENGTH] = {0};
	char strPwd[DEF_USER_PASS_LENGTH] = {0};
	char strPath[DEF_MAX_PATH] = {0};
	char strMD5[33] = {0};
	char strPort[6] ={0};
	char strSessionID[33] = {0};
	int iPort = 2121;
	lp_value_get(cmd, "sessionID", strSessionID, sizeof(strSessionID));
	lp_value_get(cmd, "userName", strUserName, sizeof(strUserName));
	lp_value_get(cmd, "pwd", strPwd, sizeof(strPwd));
	lp_value_get(cmd, "path", strPath, sizeof(strPath));
	lp_value_get(cmd, "md5", strMD5, sizeof(strMD5));
	if(lp_value_get(cmd, "port", strPort, sizeof(strPort)))
	{
		iPort = atoi(strPort);
	}

	if(g_on_update_cb)
		g_on_update_cb(strUserName, strPwd, iPort, strPath, strMD5, wise_update_cagent_rep, strSessionID, strDevID);
}

void _on_heartbeatrate_query(core_contex_t* pHandle, char* cmd, const char* strDevID)
{
	// {"susiCommData":{"commCmd":127,"catalogID":4,"handlerName":"general","sessionID":"0BD843BFB2A34E60A56C3B686BB41C90"}}
	char strSessionID[33] = {0};
	lp_value_get(cmd, "sessionID", strSessionID, sizeof(strSessionID));

	if(g_on_query_heartbeatrate)
		g_on_query_heartbeatrate(strSessionID, strDevID);
}

void _on_heartbeatrate_update(core_contex_t* pHandle, char* cmd, const char* strDevID)
{
	// {"susiCommData":{"commCmd":129,"catalogID":4,"handlerName":"general","sessionID":"0BD843BFB2A34E60A56C3B686BB41C90", "heartbeatrate":60}}
	char strRate[33] = {0};
	char strSessionID[33] = {0};
	int iRate;
	lp_value_get(cmd, "sessionID", strSessionID, sizeof(strSessionID));
	lp_value_get(cmd, "heartbeatrate", strRate, sizeof(strRate));
	iRate = atoi(strRate);
	if(g_on_update_heartbeatrate)
		g_on_update_heartbeatrate(iRate, strSessionID, strDevID);
}

void _on_server_reconnect(core_contex_t* pHandle, const char* strDevID)
{
	if(g_on_server_reconnect)
		g_on_server_reconnect(strDevID);
}

void _get_devid(const char* topic, char* devid)
{
	char *start = NULL, *end = NULL;
	if(topic == NULL) return;
	if(devid == NULL) return;
	start = strstr(topic, "/cagent/admin/");
	if(start)
	{
		start += strlen("/cagent/admin/");
	end = strstr(start, "/");
		if(end)
	strncpy(devid, start, end-start);
}
}

bool check_cmd(char *payload, char *fmt, wise_comm_cmd_t comm) {
	char cmd[16] = {0};
	sprintf(cmd, fmt, comm);
	return strstr((char*)payload, cmd);
}

void _on_message_recv(const char* topic, const void* payload, const int payloadlen, void *pUserData)
{
	core_contex_t* pHandle = (core_contex_t*)pUserData;
	char* devID[16] = {0};
	_get_devid(topic, devID);
	if(strstr((char*)payload, DEF_GENERAL_HANDLER))
	{
		if(g_on_rename_cb && check_cmd(payload, DEF_WISE_COMMAND, wise_cagent_rename_req))
		{
			_on_rename(pHandle, (char*)payload, devID);
			return;
	    }
		if(g_on_update_cb && check_cmd(payload, DEF_WISE_COMMAND, wise_update_cagent_req))
			{
			_on_update(pHandle, (char*)payload, devID);
			return;
			}
		if(check_cmd(payload, DEF_WISE_COMMAND, wise_server_control_req))
		{
			if(check_cmd(payload, DEF_SERVERCTL_STATUS, 4)) //server reconnect.
			{
				_on_server_reconnect(pHandle, devID);
				return;
			}
		}
		if(check_cmd(payload, DEF_WISE_COMMAND, wise_heartbeatrate_query_req))
		{
			_on_heartbeatrate_query(pHandle, (char*)payload, devID);
			return;
		}
		if(check_cmd(payload, DEF_WISE_COMMAND, wise_heartbeatrate_update_req))
		{
			_on_heartbeatrate_update(pHandle, (char*)payload, devID);
			return;
		}
		if(g_on_get_capability && check_cmd(payload, DEF_WISE_COMMAND, wise_info_spec_req))
		{
			g_on_get_capability(payload, payloadlen, devID);
			return;
		}
		if(g_on_start_report && check_cmd(payload, DEF_WISE_COMMAND, wise_start_auto_upload_req))
			{
			g_on_start_report(payload, payloadlen, devID);
			return;
			}
		if(g_on_stop_report && check_cmd(payload, DEF_WISE_COMMAND, wise_stop_auto_upload_req))
		{
			g_on_stop_report(payload, payloadlen, devID);
			return;
		}
	}
	else if(strstr((char*)payload, DEF_SERVERREDUNDANCY_HANDLER))
	{
		if(check_cmd(payload, DEF_WISE_COMMAND, wise_server_control_req))
		{
			if(check_cmd(payload, DEF_SERVERCTL_STATUS, 4)) //server reconnect.
			{
				_on_server_reconnect(pHandle, devID);
				return;
			}
		}
	}

	if(g_on_msg_recv_cb)
		g_on_msg_recv_cb(topic, payload, payloadlen);
}


WISECORE_API bool WISECORE_CALL core_initialize(char* strClientID, char* strHostName, char* strMAC)
{
	if(!strClientID)
	{
		g_iErrorCode = core_param_error;
		return false;
	}

	if(!strHostName)
	{
		g_iErrorCode = core_param_error;
		return false;
	}

	if(!strMAC)
	{
		g_iErrorCode = core_param_error;
		return false;
	}

	memset(&g_tHandleCtx, 0, sizeof(core_contex_t));
	g_tick = 0;
	strncpy(g_tHandleCtx.strClientID, strClientID, sizeof(g_tHandleCtx.strClientID));
	strncpy(g_tHandleCtx.strHostName, strHostName, sizeof(g_tHandleCtx.strHostName));
	strncpy(g_tHandleCtx.strMAC, strMAC, sizeof(g_tHandleCtx.strMAC));

	if(!wc_initialize(strMAC, &g_tHandleCtx))
	{
		g_iErrorCode = core_internal_error;
		return false;
	}

	wc_callback_set(_on_connect_cb, _on_lostconnect_cb, _on_disconnect_cb,_on_message_recv);
	g_bInited = true;
	g_iErrorCode = core_success;
	return true;
}

WISECORE_API void WISECORE_CALL core_uninitialize()
{
	g_iErrorCode = core_success;
	if(g_bInited)
	{
		wc_callback_set(NULL, NULL, NULL, NULL);
		wc_uninitialize();
	}
	g_tick = 0;
}

WISECORE_API bool WISECORE_CALL core_product_info_set(char* strSerialNum, char* strParentID, char* strVersion, char* strType, char* strProduct, char* strManufacture)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(strSerialNum)
		strncpy(g_tHandleCtx.strSerialNum, strSerialNum, sizeof(g_tHandleCtx.strSerialNum));

	if(strVersion)
		strncpy(g_tHandleCtx.strVersion, strVersion, sizeof(g_tHandleCtx.strVersion));
	else
	{
		sprintf(g_tHandleCtx.strVersion, "%d.%d.%d.%d", VER_MAJOR, VER_MINOR, VER_BUILD, VER_FIX);
	}

	if(strType)
		strncpy(g_tHandleCtx.strType, strType, sizeof(g_tHandleCtx.strType));
	
	if(strProduct)
		strncpy(g_tHandleCtx.strProduct, strProduct, sizeof(g_tHandleCtx.strProduct));

	if(strManufacture)
		strncpy(g_tHandleCtx.strManufacture, strManufacture, sizeof(g_tHandleCtx.strManufacture));

	if(strParentID)
		strncpy(g_tHandleCtx.strParentID, strParentID, sizeof(g_tHandleCtx.strParentID));

	g_iErrorCode = core_success;
	return true;
}

WISECORE_API bool WISECORE_CALL core_os_info_set(char* strOSName, char* strOSArch, int iTotalPhysMemKB, char* strMACs)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(strOSName)
		strncpy(g_tHandleCtx.strOSName, strOSName, sizeof(g_tHandleCtx.strOSName));

	if(strOSArch)
		strncpy(g_tHandleCtx.strOSArch, strOSArch, sizeof(g_tHandleCtx.strOSArch));

	if(strMACs)
		strncpy(g_tHandleCtx.strMACs, strMACs, sizeof(g_tHandleCtx.strMACs));
	
	g_tHandleCtx.iTotalPhysMemKB = iTotalPhysMemKB;

	g_iErrorCode = core_success;
	return true;
}

WISECORE_API bool WISECORE_CALL core_platform_info_set(char* strBIOSVersion, char* strPlatformName, char* strProcessorName)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(strBIOSVersion)
		strncpy(g_tHandleCtx.strBIOSVersion, strBIOSVersion, sizeof(g_tHandleCtx.strBIOSVersion));

	if(strPlatformName)
		strncpy(g_tHandleCtx.strPlatformName, strPlatformName, sizeof(g_tHandleCtx.strPlatformName));

	if(strProcessorName)
		strncpy(g_tHandleCtx.strProcessorName, strProcessorName, sizeof(g_tHandleCtx.strProcessorName));

	g_iErrorCode = core_success;
	return true;
}

WISECORE_API bool WISECORE_CALL core_local_ip_set(char* strLocalIP)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(strLocalIP)
		strncpy(g_tHandleCtx.strLocalIP, strLocalIP, sizeof(g_tHandleCtx.strLocalIP));

	g_iErrorCode = core_success;
	return true;
}

WISECORE_API bool WISECORE_CALL core_account_bind(char* strLoginID, char* strLoginPW)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(strLoginID)
		strncpy(g_tHandleCtx.strLoginID, strLoginID, sizeof(g_tHandleCtx.strLoginID));
	
	if(strLoginPW)
		strncpy(g_tHandleCtx.strLoginPW, strLoginPW, sizeof(g_tHandleCtx.strLoginPW));

	g_iErrorCode = core_success;
	return true;
}

WISECORE_API bool WISECORE_CALL core_tls_set(const char *cafile, const char *capath, const char *certfile, const char *keyfile, const char *password)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(!cafile && !capath)
	{
		g_iErrorCode = core_param_error;
		return false;
	}

	if(!certfile)
	{
		g_iErrorCode = core_param_error;
		return false;
	}

	if(!keyfile)
	{
		g_iErrorCode = core_param_error;
		return false;
	}

	if(wc_tls_set(cafile, capath, certfile, keyfile, password))
	{
		g_iErrorCode = core_success;
		return true;
	}
	else
	{
		g_iErrorCode = core_internal_error;
		return false;
	}
}

WISECORE_API bool WISECORE_CALL core_tls_psk_set(const char *psk, const char *identity, const char *ciphers)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(wc_tls_psk_set(psk, identity?identity:g_tHandleCtx.strClientID, ciphers))
	{
		g_iErrorCode = core_success;
		return true;
	}
	else
	{
		g_iErrorCode = core_internal_error;
		return false;
	}
}

WISECORE_API bool WISECORE_CALL core_connect(char* strServerIP, int iServerPort, char* strConnID, char* strConnPW)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(!strServerIP)
	{
		g_iErrorCode = core_param_error;
		return false;
	}

	if(!_get_agentinfo_string(&g_tHandleCtx, core_offline, strPayloadBuff, sizeof(strPayloadBuff)))
		return false;

	sprintf(strTopicBuff, DEF_WILLMSG_TOPIC, g_tHandleCtx.strClientID);

	if(wc_connect(strServerIP, iServerPort, strConnID, strConnPW, 120, strTopicBuff, strPayloadBuff, strlen(strPayloadBuff)))
	{
		g_iErrorCode = core_success;
		return true;
	}
	else
	{
		g_iErrorCode = core_internal_error;
		return false;
	}
}

WISECORE_API void WISECORE_CALL core_disconnect(bool bForce)
{
	g_iErrorCode = core_success;
	if(!g_bInited)
		return;

	if(g_tHandleCtx.iStatus == core_online)
	{
		_send_agent_disconnect(&g_tHandleCtx);
	}

	wc_disconnect(bForce);
	g_iErrorCode = core_success;
	return;
}

WISECORE_API bool WISECORE_CALL core_connection_callback_set(CORE_CONNECTED_CALLBACK on_connect, CORE_LOSTCONNECTED_CALLBACK on_lostconnect, CORE_DISCONNECT_CALLBACK on_disconnect, CORE_MESSAGE_RECV_CALLBACK on_msg_recv)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	g_on_connect_cb = on_connect;
	g_on_lostconnect_cb = on_lostconnect;
	g_on_disconnect_cb = on_disconnect;
	g_on_msg_recv_cb = on_msg_recv;

	g_iErrorCode = core_success;
	return true;
}

WISECORE_API bool WISECORE_CALL core_action_callback_set(CORE_RENAME_CALLBACK on_rename, CORE_UPDATE_CALLBACK on_update)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	g_on_rename_cb = on_rename;
	g_on_update_cb = on_update;

	g_iErrorCode = core_success;
	return true;
}

WISECORE_API bool WISECORE_CALL core_action_response(const int cmdid, const char * sessoinid, bool success, const char* devid)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(g_tHandleCtx.iStatus != core_online)
	{
		g_iErrorCode = core_no_connnect;
		return false;
	}

	if(sessoinid)
		snprintf(strPayloadBuff, sizeof(strPayloadBuff), DEF_ACTION_RESPONSE_SESSION_JSON, cmdid, success?"SUCCESS":"FALSE", sessoinid);
	else
		snprintf(strPayloadBuff, sizeof(strPayloadBuff), DEF_ACTION_RESPONSE_JSON, cmdid, success?"SUCCESS":"FALSE");

	sprintf(strTopicBuff, DEF_AGENTACT_TOPIC, devid?devid:g_tHandleCtx.strClientID);
	if(wc_publish(strTopicBuff, strPayloadBuff, strlen(strPayloadBuff), false, 0))
	{
		g_iErrorCode = core_success;
		return true;
	}
	else
	{
		g_iErrorCode = core_internal_error;
		return false;
	}
}

WISECORE_API bool WISECORE_CALL core_server_reconnect_callback_set(CORE_SERVER_RECONNECT_CALLBACK on_server_reconnect)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	g_on_server_reconnect = on_server_reconnect;

	g_iErrorCode = core_success;
	return true;
}

WISECORE_API bool WISECORE_CALL core_iot_callback_set(CORE_GET_CAPABILITY_CALLBACK on_get_capability, CORE_START_REPORT_CALLBACK on_start_report, CORE_STOP_REPORT_CALLBACK on_stop_report)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	g_on_get_capability = on_get_capability;
	g_on_start_report = on_start_report;
	g_on_stop_report = on_stop_report;

	g_iErrorCode = core_success;
	return true;
}

WISECORE_API bool WISECORE_CALL core_time_tick_callback_set(CORE_GET_TIME_TICK_CALLBACK get_time_tick)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}
	g_on_get_timetick = get_time_tick;

	g_iErrorCode = core_success;
	return true;
}

WISECORE_API bool WISECORE_CALL core_heartbeat_callback_set(CORE_QUERY_HEARTBEATRATE_CALLBACK on_query_heartbeatrate, CORE_UPDATE_HEARTBEATRATE_CALLBACK on_update_heartbeatrate)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	g_on_query_heartbeatrate = on_query_heartbeatrate;
	g_on_update_heartbeatrate = on_update_heartbeatrate;

	g_iErrorCode = core_success;
	return true;
}

WISECORE_API bool WISECORE_CALL core_heartbeatratequery_response(const int heartbeatrate, const char * sessoinid, const char* devid)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(g_tHandleCtx.iStatus != core_online)
	{
		g_iErrorCode = core_no_connnect;
		return false;
	}
	sprintf(strPayloadBuff, DEF_HEARTBEATRATE_RESPONSE_SESSION_JSON, wise_heartbeatrate_query_rep, heartbeatrate, sessoinid);
	sprintf(strTopicBuff, DEF_AGENTACT_TOPIC, devid);
	if(wc_publish((char *)strTopicBuff, strPayloadBuff, strlen(strPayloadBuff), false, 0))
	{
		g_iErrorCode = core_success;
		return true;
	}
	else
	{
		g_iErrorCode = core_internal_error;
		return false;
	}
}

WISECORE_API bool WISECORE_CALL core_publish(char const * topic, void * pkt, long pktlength, int retain, int qos)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(!topic)
	{
		g_iErrorCode = core_param_error;
		return false;
	}

	if(!pkt)
	{
		g_iErrorCode = core_param_error;
		return false;
	}

	if(g_tHandleCtx.iStatus != core_online)
	{
		g_iErrorCode = core_no_connnect;
		return false;
	}

	if(wc_publish((char *)topic, pkt, pktlength, retain==1?true:false, qos))
	{
		g_iErrorCode = core_success;
		return true;
	}
	else
	{
		g_iErrorCode = core_internal_error;
		return false;
	}
}

WISECORE_API bool WISECORE_CALL core_device_register()
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(g_tHandleCtx.iStatus != core_online)
	{
		g_iErrorCode = core_no_connnect;
		return false;
	}

	sprintf(strTopicBuff, DEF_CALLBACKREQ_TOPIC, g_tHandleCtx.strClientID);
	wc_subscribe(strTopicBuff, 0);

	sprintf(strTopicBuff, DEF_ACTIONACK_TOPIC, g_tHandleCtx.strClientID);
	wc_subscribe(strTopicBuff, 0);

	wc_subscribe(DEF_AGENTCONTROL_TOPIC, 0);

	return _send_agent_connect(&g_tHandleCtx);
}

WISECORE_API bool WISECORE_CALL core_platform_register()
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(g_tHandleCtx.iStatus != core_online)
	{
		g_iErrorCode = core_no_connnect;
		return false;
	}

	return _send_os_info(&g_tHandleCtx);
}

WISECORE_API bool WISECORE_CALL core_heartbeat_send()
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(g_tHandleCtx.iStatus != core_online)
	{
		g_iErrorCode = core_no_connnect;
		return false;
	}
	sprintf(strPayloadBuff, DEF_HEARTBEAT_MESSAGE_JSON, g_tHandleCtx.strClientID);
	sprintf(strTopicBuff, DEF_HEARTBEAT_TOPIC, g_tHandleCtx.strClientID);
	if(wc_publish((char *)strTopicBuff, strPayloadBuff, strlen(strPayloadBuff), false, 0))
	{
		g_iErrorCode = core_success;
		return true;
	}
	else
	{
		g_iErrorCode = core_internal_error;
		return false;
	}
}

WISECORE_API bool WISECORE_CALL core_subscribe(char const * topic, int qos)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(!topic)
	{
		g_iErrorCode = core_param_error;
		return false;
	}

	if(wc_subscribe((char *)topic, qos))
	{
		g_iErrorCode = core_success;
		return true;
	}
	else
	{
		g_iErrorCode = core_internal_error;
		return false;
	}
}

WISECORE_API bool WISECORE_CALL core_unsubscribe(char const * topic)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(!topic)
	{
		g_iErrorCode = core_param_error;
		return false;
	}


	if(wc_unsubscribe((char *)topic))
	{
		g_iErrorCode = core_success;
		return true;
	}
	else
	{
		g_iErrorCode = core_internal_error;
		return false;
	}
}

WISECORE_API bool WISECORE_CALL core_address_get(char *address)
{
	if(!g_bInited)
	{
		g_iErrorCode = core_no_init;
		return false;
	}

	if(g_tHandleCtx.iStatus != core_online)
	{
		g_iErrorCode = core_no_connnect;
		return false;
	}

	if(wc_address_get(address))
	{
		g_iErrorCode = core_success;
		return true;
	}
	else
	{
		g_iErrorCode = core_internal_error;
		return false;
	}

}

WISECORE_API const char* WISECORE_CALL core_error_string_get()
{
	switch(g_iErrorCode){
		case core_success:
			return "No error.";
		case core_param_error:
			return "Invalided parameters.";
		case core_no_init:
			return "No initialized.";
		case core_no_connnect:
			return "No connected.";
		case core_buff_not_enough:
			return "Created buffer size not enough.";
		default:
		case core_internal_error:
			return wc_current_error_string_get();
		case core_not_support:
			return "Not support!";
	}
}
