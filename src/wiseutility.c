#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "wiseutility.h"

#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN 6
#endif

void infiniteloop() {
	while(1) {
		usecSleep(10000);
	}
}

/******************************************************************************************/
static bool loadmac = true;
static unsigned char macAddressVal[MAC_ADDR_LEN];
static char mac[16];
static char macL[32];
static char gwmac[32];
static char snmmac[32];
static char snmac[32];
void SetDeviceMacAddress(unsigned char *macAddress) {
	memcpy(macAddressVal, macAddress, MAC_ADDR_LEN);
}
void QueryDeviceMacAddress() {
	//sl_NetCfgGet(SL_MAC_ADDRESS_GET,NULL,&macAddressLen,(_u8 *)macAddressVal);
    sprintf(mac, "%02X%02X%02X%02X%02X%02X", macAddressVal[0], macAddressVal[1],
            macAddressVal[2], macAddressVal[3], macAddressVal[4],
            macAddressVal[5]);
    sprintf(macL, "%02X:%02X:%02X:%02X:%02X:%02X", macAddressVal[0],
            macAddressVal[1], macAddressVal[2], macAddressVal[3],
            macAddressVal[4], macAddressVal[5]);
	sprintf(gwmac,"0000%s",mac);
	sprintf(snmmac,"0007%s",mac);
	sprintf(snmac,"0017%s",mac);
}
char *GetMac() {
    if (loadmac) {
        QueryDeviceMacAddress();
        loadmac = false;
    }
    return mac;
}
char *GetMacL() {
    if (loadmac) {
        QueryDeviceMacAddress();
        loadmac = false;
    }
    return macL;
}
char *GetGWMac() {
    if (loadmac) {
        QueryDeviceMacAddress();
        loadmac = false;
    }
    return gwmac;
}
char *GetSNMMac() {
    if (loadmac) {
        QueryDeviceMacAddress();
        loadmac = false;
    }
    return snmmac;
}
char *GetSNMac() {
    if (loadmac) {
        QueryDeviceMacAddress();
        loadmac = false;
    }
    return snmac;
}

static bool loadip = true;
static char ip[16] = {0};
void SetDeviceIpAddress(char *ipaddress) {
	if(ipaddress != NULL) {
		memcpy(ip, ipaddress, 16);
	} else {
		strcpy(ip,"0.0.0.0");
	}
}
void QueryDeviceIpAddress() {
	//unsigned long ulIP = 0;
	//Network_IF_IpConfigGet(&ulIP,NULL,NULL,NULL);
	/*sprintf(ip,"%d.%d.%d.%d",
			SL_IPV4_BYTE(ulIP, 3),SL_IPV4_BYTE(ulIP, 2),
			SL_IPV4_BYTE(ulIP, 1),SL_IPV4_BYTE(ulIP, 0));*/
}

char *GetIp() {
    if (loadip) {
        QueryDeviceIpAddress();
        loadip = false;
    }
    return ip;
}

char *ToUpper(char *string)
{
	int i;
	int len = strlen(string);
	for(i = 0; i < len ; i++) {
		string[i]=toupper(string[i]);
	}

	return(string);
}
/******************************************************************************************/
