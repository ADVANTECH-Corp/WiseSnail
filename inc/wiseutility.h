/*
 * wisemqtt.h
 *
 *  Created on: 2015¦~12¤ë2¤é
 *      Author: Fred.Chang
 */

#ifndef WISEUTILITY_H_
#define WISEUTILITY_H_
#include <stdio.h>
#include <unistd.h>

#define NULL_STRING(x) (x == NULL ? "":x)
#define wiseprint printf
#define usecSleep usleep

void infiniteloop();

char *GetMac();
char *GetMacL();
char *GetGWMac();
char *GetSNMMac();
char *GetSNMac();

/*#define MACADDRESS					GetMac()
#define MACADDRESS_N				GetMacL()
#define GW_MACADDRESS				GetGWMac()
#define SNM_MACADDRESS				GetSNMMac()
#define SN_MACADDRESS				GetSNMac()*/

void SetDeviceMacAddress(unsigned char *macAddress);
void SetDeviceIpAddress(char *ipaddress);

char *GetIp();
char *ToUpper(char *string);

//global buffer

#endif /* WISEUTILITY_H_ */
