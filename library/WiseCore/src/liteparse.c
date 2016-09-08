/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/01/22 by Scott Chang								    */
/* Modified Date: 2016/01/22 by Scott Chang									*/
/* Abstract     : Lite Parser API definition								*/
/* Reference    : None														*/
/****************************************************************************/
#include "liteparse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char key[128];
static char data[128];
bool lp_value_get(const char* strCmd, const char* strTag, char* value, int length)
{
	char *contents = NULL; 
	int len;
	char *cmd = NULL; 
	bool bFound = false;
	
	if(!strCmd) return false;
	if(!strTag) return false;
	
	sprintf(key,"\"%s\":", strTag);

	contents = strstr(strCmd, key);
	if(contents>0)
	{
		int result = 2;
		while(2== result){
			result = sscanf(contents, "\"%127[^\"]\":\"%127[^\"]\",%n", key, data, &len);
			if(result!=2)
				result = sscanf(contents, "\"%127[^\"]\":%127[^,],%n", key, data, &len);
			if(result!=2)
				break;
			if(!strcmp(key, strTag))
			{
				//printf("%s=%s\n", key, data);
				memset(value, 0, length);
				strncpy(value, data, length);
				bFound = true;
				break;
			}
			contents += len;
			while(contents[0] ==' ')
				contents++;			
		}
	}
	return bFound;
}
