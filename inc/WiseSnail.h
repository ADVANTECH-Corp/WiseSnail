#ifndef WISESNAIL_H_
#define WISESNAIL_H_

#if defined(WIN32)
	#pragma once
	#ifdef WISESNAIL_EXPORTS
		#define WISESNAIL_CALL __stdcall
		#define WISESNAIL_EXPORT __declspec(dllexport)
	#else
		#define WISESNAIL_CALL __stdcall
		#define WISESNAIL_EXPORT
	#endif
	#define __func__ __FUNCTION__
#else
	#define WISESNAIL_CALL
	#define WISESNAIL_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif



typedef enum WiseSnail_DataType
{
    WISE_SYSTEM,
    WISE_VALUE,
	WISE_FLOAT,
	WISE_STRING,
	WISE_BOOL,
}WiseSnail_DataType;

typedef struct WiseSnail_InfoSpec WiseSnail_InfoSpec;

typedef struct WiseSnail_Data{
    WiseSnail_DataType type;
    char *name;
    union {
		double value;
        char *string;
    };
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
	};
    double min;
    double max;
	char *resourcetype;
	WiseSnail_SetValue setValue;
	WiseSnail_GetValue getValue;
};



WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_Init(char *productionName, char *wanIp, unsigned char *parentMac, WiseSnail_InfoSpec *infospec, int count);

WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_RegisterInterface(char *ifMac, char *ifName, int ifNumber, WiseSnail_InfoSpec *infospec, int count);

WISESNAIL_EXPORT int WISESNAIL_CALL WiseSnail_Connect(char *server_url, int port, char *username, char *password, WiseSnail_InfoSpec *infospec, int count);

WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_RegisterSensor(char *deviceMac, char *defaultName, WiseSnail_InfoSpec *infospec, int count);

WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_SenHubDisconnect(char *deviceMac);
WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_SenHubReConnected(char *deviceMac);


WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_Update(char *deviceMac, WiseSnail_Data* data, int count);
WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_Get(char *deviceMac, char *name, WiseSnail_Data *data);
WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_MainLoop(WiseSnail_SleepOneSecond sleepOneSec);
WISESNAIL_EXPORT void WISESNAIL_CALL WiseSnail_Uninit();

#define WiseSnail_Cmd_Handler WiseSnail_MainLoop
#define WiseSnail_Open WiseSnail_Connect
#define WiseSnail_Write WiseSnail_Update
#define WiseSnail_Read WiseSnail_Get
#define WiseSnail_Close WiseSnail_Uninit

#ifdef __cplusplus
}
#endif
#endif //WISESNAIL_H_
