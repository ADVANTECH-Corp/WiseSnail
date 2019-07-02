#ifndef WISESNAIL_H_
#define WISESNAIL_H_

#if defined(WIN32)
	#pragma once
	#define WISESNAIL_CALL __stdcall
	#define WISESNAIL_EXPORT __declspec(dllexport)
	#define __func__ __FUNCTION__
#else
	#define WISESNAIL_CALL
	#define WISESNAIL_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define IPSO_Digital_Input      3200
#define IPSO_Digital_Output     3201
#define IPSO_Analogue_Input     3202
#define IPSO_Analogue_Output    3203
#define IPSO_Generic_Sensor     3300
#define IPSO_Illuminance_Sensor 3301
#define IPSO_Presence_Sensor    3302
#define IPSO_Temperature_Sensor 3303
#define IPSO_Humidity_Sensor    3304
#define IPSO_Power_Measurement  3305
#define IPSO_Actuation          3306
#define IPSO_Set_Point          3308
#define IPSO_Load_Control       3310
#define IPSO_Light_Control      3311
#define IPSO_Power_Control      3312
#define IPSO_Accelerometer      3313
#define IPSO_Magnetometer       3314
#define IPSO_Barometer          3315



typedef enum WiseSnail_DataType
{
    WISE_SYSTEM,
    WISE_VALUE,
	WISE_FLOAT,
	WISE_STRING,
	WISE_BOOL,
    WISE_CUSTOMIZE,
}WiseSnail_DataType;

typedef enum WiseSnail_FormatType
{
    WISE_RAW = 0,
    WISE_BASE64,
}WiseSnail_FormatType;


typedef struct WiseSnail_InfoSpec WiseSnail_InfoSpec;

typedef struct WiseSnail_RAW {
    char *data;
    int len;
} WiseSnail_RAW;

typedef struct WiseSnail_Data{
    WiseSnail_DataType type;
    char *name;
    union {
		double value;
        char *string;
        WiseSnail_RAW *raw;
    };
    WiseSnail_FormatType format;
    
    //hidden
	char *clientId;
	WiseSnail_InfoSpec *info;
} WiseSnail_Data;

typedef int (*WiseSnail_SetValue)(WiseSnail_Data *data);
typedef int (*WiseSnail_GetValue)(WiseSnail_Data *data);
typedef void (*WiseSnail_SleepOneSecond)(void);

struct WiseSnail_InfoSpec{
	WiseSnail_DataType type;
	char *name;
	char *unit;
	union {
        double value;
        char *string;
        WiseSnail_RAW *raw;
	};
    double min;
    double max;
	char *resourcetype;
	WiseSnail_SetValue setValue;
	WiseSnail_GetValue getValue;
    WiseSnail_FormatType format;
};




//GW
WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_Init(char *productionName, char *wanIp, unsigned char *parentMac, WiseSnail_InfoSpec *infospec, int count);

WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_RegisterInterface(char *ifMac, char *ifName, int ifNumber, WiseSnail_InfoSpec *infospec, int count);

WISESNAIL_EXPORT int WISESNAIL_CALL WiseSnail_Connect(char *server_url, int port, char *username, char *password, WiseSnail_InfoSpec *infospec, int count);

WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_RegisterSensor(char *deviceMac, char *defaultName, WiseSnail_InfoSpec *infospec, int count);

WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_Update(char *deviceMac, WiseSnail_Data* data, int count);

WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_Get(char *deviceMac, char *name, WiseSnail_Data *data);
WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_MainLoop(WiseSnail_SleepOneSecond sleepOneSec);
WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_Uninit();


#define WiseSnail_Open WiseSnail_Connect
#define WiseSnail_Write WiseSnail_Update
#define WiseSnail_Read WiseSnail_Get
#define WiseSnail_Cmd_Handler WiseSnail_MainLoop
#define WiseSnail_Close WiseSnail_Uninit

//Sensor Hub
//WISESNAIL_EXPORT char *WISESNAIL_CALL WiseSnail_JoinSenHub(char *deviceMac, char *defaultName, WiseSnail_InfoSpec *sencorSpec, int count);
WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_SenHubDisconnect(char *deviceMac);
WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_SenHubReConnected(char *deviceMac);


//Service
WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_Service_Init(char *serviceGroup, char *version, WiseSnail_InfoSpec *infospec, int count);
WISESNAIL_EXPORT char *WISESNAIL_CALL WiseSnail_Service_RegisterEntry(char *uuid, char *serveName,WiseSnail_InfoSpec *infospec, int count);
//WISESNAIL_EXPORT char *WISESNAIL_CALL WiseSnail_JoinService(char *uuid, char *serviceGroup, char *serviceName, char *version, WiseSnail_InfoSpec *serviceSpec, int count);


#define WiseSnail_Service_Connect WiseSnail_Connect
#define WiseSnail_Service_Update WiseSnail_Update
#define WiseSnail_Service_MainLoop WiseSnail_MainLoop
#define WiseSnail_Service_Uninit WiseSnail_Uninit

#ifdef __cplusplus
}
#endif
#endif //WISESNAIL_H_
