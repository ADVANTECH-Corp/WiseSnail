#include <stdio.h>

#include <string.h>   //strncpy
#include <unistd.h>   //close

#include "WiseSnail.h"
#if defined(WIN32)
#pragma comment(lib, "WiseSnail.lib")
#endif
//#define SERVER_URL            "dev-wisepaas.cloudapp.net"
//#define SERVER_URL            "dev-wisepaas-ssl.cloudapp.net"
//#define SERVER_URL 			"dev-wisepaas-ssl.eastasia.cloudapp.azure.com"
//#define SERVER_URL 				"dev-wisepaas.eastasia.cloudapp.azure.com"
//#define SERVER_URL 				"rmm.wise-paas.com"
//#define SERVER_URL            "172.22.12.9"
#define SERVER_URL            "172.22.12.178"
/*Info/reset
Action/AutoReport*/

char message[64] = "I'm Fred.";

int Reset(WiseSnail_Data *data) {
    printf("\n###############Reset name %d\n\n",data->value);
}

int Get(WiseSnail_Data *data) {
    printf("\n###############Get value %s\n\n",message);
    strncpy(data->string,message, sizeof(data->string));
}

int Set(WiseSnail_Data *data) {
    printf("\n###############Set value %s\n\n",data->string);
    strncpy(message, data->string, sizeof(data->string));
}

WiseSnail_InfoSpec servicelist[] = {
		{
				WISE_BOOL,
				"/Info/reset",
				"",
				0,
				0,
				1,
				"",
				Reset
		},
        {
				WISE_STRING,
				"/Info/message",
				"",
				.string = "",
				0,
				1,
				"",
				Set,
                Get
		},
};

WiseSnail_Data data[] = {
		{
				WISE_STRING,
				"/Info/message",
				.string = ""
		}
};

void sleepOneSecond() {
	sleep(1);
}

int main() {
	char message[32];

	WiseSnail_Service_Init("Algorithm","2.0", NULL, 0);
    printf("<%s,%d>\n",__FILE__,__LINE__);
	char *serviceId = WiseSnail_Service_RegisterEntry("12345", "Add", servicelist, 2);
    if(WiseSnail_Service_Connect(SERVER_URL, 1883, "", "", NULL, 0) == 0) {
    	//
		// no succesful connection to broker
		//
		return -1;
    }
	
	int count = 0;
    int second = 0;
	int test = 0;
//    return -1;
    for(;;) {

        if(second == 0) {
			/*HDC1050_GetSensorData(&Temperature, &Humidity);
			data[0].value = Temperature;
			data[1].value = Humidity;
            data[2].value = GetGPIO_1();
            data[3].value = GetGPIO_2();*/
			
			/*if(data[0].value < 100) {
				data[0].value++;
			} else {
				data[0].value = 0;
			}
			
			if(data[1].value < 100) {
				data[1].value++;
			} else {
				data[1].value = 0;
			}*/
			
			sprintf(message, "count = %d",count);
            data[0].string = message;
            printf("\r\n****** \033[33mSend update.\033[0m ******\r\n");
			if(test==0) {
            WiseSnail_Service_Update(serviceId, data, 1);
				test = 1;
			}
			//WiseSnail_Service_Update("000E4C000001", data, 4);
			count++;
        }

		WiseSnail_MainLoop(sleepOneSecond);
		
		second = (second+1)%5;

    }
	return 0; 
}
