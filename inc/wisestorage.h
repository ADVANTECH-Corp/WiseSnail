/*
 * wiseaccess.h
 *
 *  Created on: 2016¦~1¤ë20¤é
 *      Author: Fred.Chang
 */

#ifndef WISESTORAGE_H_
#define WISESTORAGE_H_

typedef struct WiseAgentCfg {
    char gwName[64];
} WiseAgentCfg;

typedef struct WiseDeviceCfg {
    char infoName[64];
} WiseDeviceCfg;

typedef struct WiseDeviceInfo {
	char infoSw[64];
	char netSw[64];
	char netNeighbor[1024];
} WiseDeviceINfo;

int WiseStorage_ReadAgent(WiseAgentCfg *cfg);
int WiseStorage_WriteAgent(WiseAgentCfg *cfg);
int WiseStorage_ReadDevice(char *clientId, WiseDeviceCfg *cfg);
int WiseStorage_WriteDevice(char *clientId, WiseDeviceCfg *cfg);
#endif /* WISESTORAGE_H_ */
