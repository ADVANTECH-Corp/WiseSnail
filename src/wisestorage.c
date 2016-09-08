#include <stdio.h>

#include "wiseutility.h"
#include "wisestorage.h"

#define AGENT_CONFIG_FILE "/etc/agentcfg.bin"
#define DEVICE_CONFIG_FILE "/etc/%s.bin"
static char fileName[64];
static int WiseStorage_ReadCfg(char *clientId, void *cfg, int len)
{
    int iRetVal;
	//long lFileHandle;
    FILE *fp = NULL;

    if(cfg == NULL) {
        wiseprint("Input argument is null\n\r");
        return -1;
    }

    //
    // Open OTA configuration for reading
    //
	if(clientId == NULL) {
		fp = fopen(AGENT_CONFIG_FILE, "r");
	} else {
		sprintf(fileName, DEVICE_CONFIG_FILE, clientId);
		fp = fopen(fileName, "r");
	}
    /*iRetVal = ((unsigned char *)AGENT_CONFIG_FILE,
                            FS_MODE_OPEN_READ,
                            NULL,
                            &lFileHandle);*/

    //
    // If successful, Reading OTA configuration
    //
    if(fp != NULL) {
        /*iRetVal = sl_FsRead(lFileHandle,
                            0,
                            (unsigned char *)cfg,
                            sizeof(WiseAgentCfg));
        sl_FsClose(lFileHandle, NULL, NULL , 0);*/
		iRetVal = fread((unsigned char *)cfg, len, 1, fp);
		fclose(fp);
        return iRetVal;
    }

    return -1;
}

static int WiseStorage_WriteCfg(char *clientId, void *cfg, int len)
{
    int iRetVal;
    //long lFileHandle;
	FILE *fp = NULL;
	
    if(cfg == NULL) {
        wiseprint("Input argument is null\n\r");
        return -1;
    }

    //
    // Open OTA configuration for writing
    //
    /*iRetVal = sl_FsOpen((unsigned char *)AGENT_CONFIG_FILE,
                            FS_MODE_OPEN_WRITE,
                            NULL,
                            &lFileHandle);*/
	if(clientId == NULL) {
		fp = fopen(AGENT_CONFIG_FILE, "r+");
	} else {
		sprintf(fileName, DEVICE_CONFIG_FILE, clientId);
		fp = fopen(fileName, "r+");
	}
    //
    // If successful, Writing OTA configuration
    //
    if(fp != NULL) {
        /*iRetVal = sl_FsWrite(lFileHandle,
                                0,
                                (unsigned char *)cfg,
                                sizeof(WiseAgentCfg));
        sl_FsClose(lFileHandle, NULL, NULL , 0);*/
		iRetVal = fwrite((unsigned char *)cfg, len, 1, fp);
		fclose(fp);
        return iRetVal;
    }

    //
    // If failed, Create a new OTA configuration
    //
    /*iRetVal = sl_FsOpen((unsigned char *)AGENT_CONFIG_FILE,
                                FS_MODE_OPEN_CREATE(sizeof(WiseAgentCfg),
                                        _FS_FILE_OPEN_FLAG_COMMIT |
                                        _FS_FILE_PUBLIC_WRITE |
                                        _FS_FILE_PUBLIC_READ),
                                NULL,
                                &lFileHandle);*/
	if(clientId == NULL) {
		fp = fopen(AGENT_CONFIG_FILE, "w+");
	} else {
		sprintf(fileName, DEVICE_CONFIG_FILE, clientId);
		fp = fopen(fileName, "w+");
	}

    if(fp != NULL) {
        /*iRetVal = sl_FsWrite(lFileHandle,
                                0,
                                (unsigned char *)cfg,
                                sizeof(WiseAgentCfg));
        sl_FsClose(lFileHandle, NULL, NULL , 0);*/
		iRetVal = fwrite((unsigned char *)cfg, len, 1, fp);
		fclose(fp);
        return iRetVal;
    }

    return -1;
}




int WiseStorage_ReadAgent(WiseAgentCfg *cfg) {
	return WiseStorage_ReadCfg(NULL, cfg, sizeof(WiseAgentCfg));
}
int WiseStorage_WriteAgent(WiseAgentCfg *cfg) {
	return WiseStorage_WriteCfg(NULL, cfg, sizeof(WiseAgentCfg));
}
int WiseStorage_ReadDevice(char *clientId, WiseDeviceCfg *cfg) {
	return WiseStorage_ReadCfg(clientId, cfg, sizeof(WiseAgentCfg));
}
int WiseStorage_WriteDevice(char *clientId, WiseDeviceCfg *cfg) {
	return WiseStorage_WriteCfg(clientId, cfg, sizeof(WiseAgentCfg));
}
