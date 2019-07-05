#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "WiseSnail.h"
#if defined(WIN32)
#pragma comment(lib, "WiseSnail.lib")
#endif

#define SERVER_URL		"127.0.0.1"
#define SERVER_PORT		1883

#define SENSOR_DEVICE1_MAC	"000E4C000000"
#define SENSOR_DEVICE2_MAC	"000E4C000001"

/*Info/reset
Action/AutoReport*/


int Reset(WiseSnail_Data *data) {
	printf("\n###############Reset name %d\n\n",data->value);
}

WiseSnail_InfoSpec interface1[] = {
	{
		WISE_BOOL,
		"/Info/reset",
		"",
		0,
		0,
		1,
		"",
		Reset
	}
};

char SHName[128] = "123";
int SetSHName(WiseSnail_Data *mydata) {
	printf("###############Set SenHub name %s\n",mydata->string);
	strcpy(SHName, mydata->string);
}

int GetSHName(WiseSnail_Data *mydata) {
	printf("###############Get SenHub name %s\n",mydata->string);
	strcpy(mydata->string, SHName);
}

int SetGPIO_1(WiseSnail_Data *mydata) {
	printf("\n###############Set GPIO1 to %d\n\n", (int)mydata->value);
}

int GetGPIO_1(WiseSnail_Data *mydata) {
	printf("\n###############Get %d from GPIO1\n\n", (int)mydata->value);
}

char rawData[128] = "ABCDE";
WiseSnail_RAW rawItem = {
	.data = rawData,
	.len = 5
};

int SetRAW(WiseSnail_Data *data) {
	printf("<%s,%d>\n",__FILE__,__LINE__);
	printf("###############type = %d\n\n", data->type);
	if(data->type == WISE_CUSTOMIZE) {
		printf("###############len = %d\n\n", data->raw->len);
		printf("###############Set RAW to %s\n\n", data->raw->data);
		memcpy(&rawItem, data->raw, sizeof(rawItem));
	}
}

int GetRAW(WiseSnail_Data *data) {
	printf("<%s,%d>\n",__FILE__,__LINE__);
	if(data->type == WISE_CUSTOMIZE) {
		printf("###############Get %s from RAW\n\n", data->raw->data);
		memcpy(data->raw, &rawItem, sizeof(rawItem));
	}
}

WiseSnail_InfoSpec infospec1[] = {
	{
		WISE_STRING,
		"/Info/Name",
		"",
		.string = "SenHub1",
		0,
		0,
		"",
		//SetSHName,
		.getValue = GetSHName
	},
	{
		WISE_VALUE,
		"Temperature",
		"Cel",
		0,
		-100,
		200,
		"ucum.Cel"
	},
	{
		WISE_VALUE,
		"Humidity",
		"%",
		0,
		0,
		100,
		"ucum.%"
	},
	{
		WISE_BOOL,
		"GPIO1",
		"",
		0,
		0,
		1,
		"",
		SetGPIO_1,
		GetGPIO_1
	},
	{
		WISE_BOOL,
		"GPIO2",
		"",
		0,
		0,
		1,
		"",
		NULL//SetGPIO_2
	},
	{
		WISE_CUSTOMIZE,
		"RAW",
		"",
		.raw = &rawItem,
		0,
		1,
		"",
		SetRAW,
		GetRAW,
		WISE_BASE64
	}
};

WiseSnail_InfoSpec infospec2[] = {
	{
		WISE_STRING,
		"/Info/Name",
		"",
		.string = "SenHub2",
		0,
		0,
		""
	},
	{
		WISE_VALUE,
		"Temperature",
		"Cel",
		0,
		-100,
		200,
		"ucum.Cel"
	},
	{
		WISE_VALUE,
		"Humidity",
		"%",
		0,
		0,
		100,
		"ucum.%"
	},
	{
		WISE_BOOL,
		"GPIO1",
		"",
		0,
		0,
		1,
		"",
		NULL//SetGPIO_1
	},
	{
		WISE_BOOL,
		"GPIO2",
		"",
		0,
		0,
		1,
		"",
		NULL//SetGPIO_2
	}
};


WiseSnail_Data data[] = {
	{
		WISE_VALUE,
		"Temperature",
		100
	},
	{
		WISE_VALUE,
		"Humidity",
		55
	},
	{
        WISE_BOOL,
		"GPIO1",
		0
	},
	{
		WISE_BOOL,
		"GPIO2",
		0
	},
	{
		WISE_CUSTOMIZE,
		"RAW",
		.raw = &rawItem,
		WISE_BASE64
	},
	{
		WISE_STRING,
		"/Info/Name",
		.string = "123456789012345678901234567890",
	},
	{
		WISE_STRING,
		"/Info/sw",
		.string = "1.0.00",
	},
	{
		WISE_STRING,
		"/Net/sw",
		.string = "1.0.00",
	},
	{
		WISE_STRING,
		"/Net/Neighbor",
		.string = "",
	},
	{
		WISE_VALUE,
		"/Net/Health",
		100,
	}
};

void sleepOneSecond() {
	sleep(1);
}

int main (void)
{
	int count = 0;
    int second = 0;

	WiseSnail_Init("IotGW",NULL, NULL, NULL, 0);
	WiseSnail_RegisterInterface("000E4CAB1234", "Ethernet", -1, interface1, 1);

	if(WiseSnail_Connect(SERVER_URL, SERVER_PORT, "", "", NULL, 0) == 0) {
    	//
		// no succesful connection to broker
		//
		return -1;
	} else {
		sleep(1);
		WiseSnail_RegisterSensor(SENSOR_DEVICE1_MAC, "OnBoard", infospec1, 6);
		//sleep(1);
		//WiseSnail_RegisterSensor(SENSOR_DEVICE2_MAC, "OnBoard2", infospec2, sizeof(infospec2)/sizeof(WiseSnail_InfoSpec));
    }

	for(;;) {
		if(second == 0) {
			/*HDC1050_GetSensorData(&Temperature, &Humidity);
			data[0].value = Temperature;
			data[1].value = Humidity;
			data[2].value = GetGPIO_1();
			data[3].value = GetGPIO_2();*/

			if(data[0].value < 100) {
				data[0].value++;
			} else {
				data[0].value = 0;
			}

			if(data[1].value < 100) {
				data[1].value++;
			} else {
				data[1].value = 0;
			}

			data[4].raw = &rawItem;

			printf("\r\n****** \033[33mSend update.\033[0m ******\r\n");
			WiseSnail_Update(SENSOR_DEVICE1_MAC, data, 5);
			//WiseSnail_Update(SENSOR_DEVICE2_MAC, data, 4);
			count++;
        }
		WiseSnail_MainLoop(sleepOneSecond);
		second = (second+1)%5;
	}
	return 0;
}
