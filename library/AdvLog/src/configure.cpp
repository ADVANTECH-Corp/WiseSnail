#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <getopt.h>

#include <string>
#include <map>

#include "fplatform.h"
#include "tool.h"
#include "configure.h"

#include "AdvJSON.h"

using namespace std;

typedef struct {
		int externalConfKey;
		int externalConfSize;
		//information
		int pid;
		int flush;
		char configureFile[256];
		char configureName[256];
		
		//configure
		int limit;
		int number;
		char compress[8];
		char path[256];
		int staticLevel;
		int staticInfo;
		int staticGray;
		int dynamicLevel;
		int dynamicInfo;
		int dynamicGray;
		char hide[256];
} AdvLogConf;


AdvLogConf defaultconf = {
	0,					//externalConfKey
	0,					//externalConfSize
	-1,					//pid
	0,					//flush
	"",					//configureFile
	"default",			//configureName
	102400,				//limit
	2,					//number
	"NONE",				//compress
	"./logs",			//path
	5,					//staticLevel
	1,					//staticInfo
	0,					//StaticGray
	5,					//dynamicLevel
	0,					//dynamicInfo
	0,					//dynamicGray
	""					//hide
};


static map<string,AdvLogConf *> globalConf;

void *mountKeyMemory(unsigned int sharedKey, int size) {
	char *ptr = NULL;
	//printf("sharedKey = 0x%08X\n",sharedKey);
	if (sharedKey == 0xFFFFFFFF) {
		printf("<%s,%s,%d> key:%08X failed\n", __FILE__, __func__, __LINE__, sharedKey);
		return (void *)ptr;
	}
	long shm_id;
	shm_id = shmget(sharedKey, size, 0666 | IPC_CREAT | IPC_EXCL);
	//printf("shm_id = %d\n",shm_id);
	if (shm_id == -1) {
		//printf("size = %d\n",size);
		shm_id = shmget(sharedKey, size, 0666);
		//printf("shm_id = %d\n",shm_id);
		if (shm_id == -1) {
			printf("<%s,%s,%d> key:%08X failed\n", __FILE__, __func__, __LINE__, sharedKey);
			return (void *)ptr;
		}
		else {
			ptr = (char *)shmat(shm_id, NULL, 0);
			return (void *)ptr;
		}
	}
	else {
		ptr = (char *)shmat(shm_id, NULL, 0);
		memset(ptr, 0, size);
		return (void *)ptr;
	}
	return (void *)ptr;
}


void unmountKeyMemory(void *addr) {
	shmdt(addr);
}


static AdvLogConf *configure = &defaultconf;
static AdvLogConf *externalConf = NULL;

static char *jsonbuffer = NULL;
static AdvJSON jsonconf;

inline static void AdvLog_Configure_Default(AdvLogConf *conf) {
	
	conf->externalConfKey = 0;
	conf->externalConfSize = 0;
	
	//pid
	conf->flush = defaultconf.flush;
	strcpy(conf->configureFile,defaultconf.configureFile);
	strcpy(conf->configureName,defaultconf.configureName);
	conf->limit = defaultconf.limit;
	conf->number = defaultconf.number;
	strcpy(conf->compress,defaultconf.compress);
	strcpy(conf->path,defaultconf.path);
	conf->staticLevel = defaultconf.staticLevel;
	conf->staticInfo = defaultconf.staticInfo;
	conf->staticGray = defaultconf.staticGray;
	conf->dynamicLevel = defaultconf.dynamicLevel;
	conf->dynamicInfo = defaultconf.dynamicInfo;
	conf->dynamicGray = defaultconf.dynamicGray;
	strcpy(conf->hide,defaultconf.hide);
}

inline static AdvLogConf *AdvLog_Configure_Attach(int pid) {
	if(configure == &defaultconf) {
		int sharedKey = 0x20150707 - pid;
		configure = (AdvLogConf *)mountKeyMemory(sharedKey, sizeof(AdvLogConf));
		if(configure->pid == pid) {
			if(configure->externalConfKey != 0) {
				externalConf = (AdvLogConf *)mountKeyMemory(configure->externalConfKey, sizeof(AdvLogConf) * configure->externalConfSize);
				AdvLogConf *target = NULL;
				for(int i = 0 ; i < configure->externalConfSize ; i++) {
					target = &externalConf[i];
					globalConf[target->configureName] = target;
				}
			}
		} else {
			configure->pid = pid;
			AdvLog_Configure_Default(configure);
		}
	}
	return configure;
}


int AdvLog_Configure_Init(int pid) {
	AdvLogConf *conf = AdvLog_Configure_Attach(pid);
	if(conf != NULL) return pid;
	else return 0;
}

static void AdvLog_Configure_Import_From_JSON(AdvLogConf *conf, char *jsonstring) {
	AdvJSON json = jsonstring;
	if(json.IsNULL()) return;
	if(conf->externalConfKey != 0) return;
	
	int externalConfKey = 0x20150707 + conf->pid;
	int externalConfSize = json.Size();
	externalConf = (AdvLogConf *)mountKeyMemory(externalConfKey, sizeof(AdvLogConf) * externalConfSize);
	conf->externalConfKey = externalConfKey;
	conf->externalConfSize = externalConfSize;

	string value;
	//json.Print();

	//printf("Assing: conf->configureName = %s\n", conf->configureName);
	AdvLogConf *target = NULL;
	for(int i = 0 ; i < externalConfSize ; i++) {
		target = &externalConf[i];
		AdvLog_Configure_Default(target);
		
		strcpy(target->configureName,json[i].Key().c_str());

		value = json[target->configureName]["path"].Value();
		if(value != "NULL") {
			strcpy(target->path,value.c_str());
		}
		//printf("Assing: value = %s\n", value.c_str());
		//printf("Assing: conf->path = %s\n", conf->path);
		value = json[target->configureName]["limit"].Value();
		if(value != "NULL") {
			target->limit = atoi(value.c_str());
		}
		
		value = json[target->configureName]["number"].Value();
		if(value != "NULL") {
			target->number = atoi(value.c_str());
		}
		
		value = json[target->configureName]["compress"].Value();
		if(value != "NULL") {
			strncpy(target->compress,value.c_str(),sizeof(target->compress)-1);
			target->compress[strlen(target->compress)] = 0;
		}
		
		value = json[target->configureName]["static"]["level"].Value();
		if(value != "NULL") {
			target->staticLevel = atoi(value.c_str());
			if(target->staticLevel < 0) {
				target->staticGray = 1;
				target->staticLevel = -target->staticLevel;
			} else {
				target->staticGray = 0;
			}
			//printf("conf->staticLevel = %d\n", conf->staticLevel);
		}
		
		value = json[target->configureName]["static"]["information"].Value();
		if(value != "NULL") {
			target->staticInfo = atoi(value.c_str());
		}
		
		
		value = json[target->configureName]["dynamic"]["level"].Value();
		if(value != "NULL") {
			target->dynamicLevel = atoi(value.c_str());
			if(target->dynamicLevel < 0) {
				target->dynamicGray = 1;
				target->dynamicLevel = -target->dynamicLevel;
			} else {
				target->dynamicGray = 0;
			}
		}
		
		value = json[target->configureName]["dynamic"]["information"].Value();
		if(value != "NULL") {
			target->dynamicInfo = atoi(value.c_str());
		}
		//printf("Assing: json[conf->configureName][dynamic][information].Value() = %s\n", json[conf->configureName]["dynamic"]["information"].Value().c_str());
		//printf("Assing: conf->dynamicInfo = %d\n", conf->dynamicInfo);
		
		value = json[target->configureName]["dynamic"]["hide"].Value();
		if(value != "NULL") {
			strcpy(target->hide,value.c_str());
		}
		
		globalConf[target->configureName] = target;
	}
}

static void AdvLog_Configure_Export_To_File(AdvLogConf *conf, char *filename) {
	AdvJSON json("{}");
	
	if(conf->externalConfKey == 0) { 
#if __cplusplus > 199711L
		AdvJSONCreator C(json);
		json.New()["default"][{"path","limit", "number", "compress", "static", "dynamic"}] = {
			conf->path,
			conf->limit,
			conf->number,
			conf->compress,
			C[{"level","information"}]({
				(conf->staticGray == 1 ? -conf->staticLevel : conf->staticLevel),
				conf->staticInfo
				}),
			C[{"level","information", "hide"}]({
				(conf->dynamicGray == 1 ? -conf->dynamicLevel : conf->dynamicLevel),
				conf->dynamicInfo,
				conf->hide
				}),
		};
#else
		json.New()["default"]["path"] = conf->path;
		json.New()["default"]["limit"] = conf->limit;
		json.New()["default"]["number"] = conf->number;
		json.New()["default"]["compress"] = conf->compress;
		json.New()["default"]["static"]["level"] = (conf->staticGray == 1 ? -conf->staticLevel : conf->staticLevel);
		json.New()["default"]["static"]["information"] = conf->staticInfo;
		json.New()["default"]["dynamic"]["level"] = (conf->dynamicGray == 1 ? -conf->dynamicLevel : conf->dynamicLevel);
		json.New()["default"]["dynamic"]["information"] = conf->dynamicInfo;
		json.New()["default"]["dynamic"]["hide"] = conf->hide;
#endif
	} else {
		AdvLogConf *target = NULL;
		for(int i = 0 ; i < conf->externalConfSize ; i++) {
			target = &externalConf[i];
			json.New()[target->configureName]["path"] = target->path;
			json.New()[target->configureName]["limit"] = target->limit;
			json.New()[target->configureName]["number"] = target->number;
			json.New()[target->configureName]["compress"] = target->compress;
			json.New()[target->configureName]["static"]["level"] = (target->staticGray == 1 ? -target->staticLevel : target->staticLevel);
			json.New()[target->configureName]["static"]["information"] = target->staticInfo;
			json.New()[target->configureName]["dynamic"]["level"] = (target->dynamicGray == 1 ? -target->dynamicLevel : target->dynamicLevel);
			json.New()[target->configureName]["dynamic"]["information"] = target->dynamicInfo;
			json.New()[target->configureName]["dynamic"]["hide"] = target->hide;
		}
	}
	//json.Print();
	
	char buffer[4096] = {0};
	json.Print(buffer, sizeof(buffer),0);
	
	FILE *fp = fopen(filename,"w+");
	fprintf(fp,"%s",buffer);
	fclose(fp);
}

static void PrintHelp(int pid) {
	printf("Option -p [pid]: Set pid (must be the first parameter)\n");
	printf("\n");
	printf("Option -s [level]: Set STATIC level (default:5)(negative means gray mode)\n");
	printf("Option -i [0|1]: Enable STATIC infomation (default:1)\n");
	printf("\n");
	printf("Option -d [level]: Set DYNAMIC level (default:5)(negative means gray mode)\n");
	printf("Option -j [0|1]: Enable DYNAMIC info (default:0)\n");
	printf("Option -b {level string}: Hide DYNAMIC message (default:\"\", example:\"1,3,5\")\n");
	printf("\n");
	printf("Option -l [limit]: set file max size (unit: byte)(default:102400)\n");
	printf("Option -x {path}: set log files path (default:./logs)\n");
	
	if(pid == configure->pid) { //Not Support on Remote.
		printf("Option -f: import configure file\n");
	}
	
	printf("Option -e: export configure file\n");
	printf("Option -v: Show all parameter\n");
}

int AdvLog_Configure_OptParser(int argc, char **argv, int pid) {
	int index;
	int c;
	AdvLogConf *conf = &defaultconf;
	//printf("<%s,%d>\n",__FILE__,__LINE__);
	/*if(argc <= 1) {
		PrintHelp();
	}*/
	if(argc == 1) return 0;
	//printf("<%s,%d> pid = %d\n",__FILE__,__LINE__, pid);
	if(pid != -1) {
		conf = AdvLog_Configure_Attach(pid);
	}
	//printf("<%s,%d>\n",__FILE__,__LINE__);
	while ((c = getopt (argc, argv, "p:s:i:d:j:b:l:x:vf:e:h?")) != -1) {
		//printf("<%s,%d> c = %c\n",__FILE__,__LINE__,c);
		switch (c)
		{
			case 'p':
				pid = atoi(optarg);
				conf = AdvLog_Configure_Attach(pid);
			break;
			case 's':
				if(pid > 0) {
					conf->staticLevel = atoi(optarg);
					if(conf->staticLevel < 0) {
						conf->staticGray = 1;
					} else {
						conf->staticGray = 0;
					}
					conf->staticLevel = ABS(conf->staticLevel);
				} else {
					printf("Error: pid must be the first parameter.\n");
				}
			break;
			case 'i':
				if(pid > 0) {
					conf->staticInfo = atoi(optarg);
				} else {
					printf("Error: pid must be the first parameter.\n");
				}
			break;
			case 'd':
				if(pid > 0) {
					conf->dynamicLevel = atoi(optarg);
					if(conf->dynamicLevel < 0) {
						conf->dynamicGray = 1;
					} else {
						conf->dynamicGray = 0;
					}
					conf->dynamicLevel = ABS(conf->dynamicLevel);
				} else {
					printf("Error: pid must be the first parameter.\n");
				}
			break;
			case 'j':
				if(pid > 0) {
					conf->dynamicInfo = atoi(optarg);
				} else {
					printf("Error: pid must be the first parameter.\n");
				}
			break;
			case 'b':
				if(pid > 0) {
					strncpy(conf->hide, optarg, sizeof(conf->hide));
				} else {
					printf("Error: pid must be the first parameter.\n");
				}
			break;
			case 'l':
				if(pid > 0) {
					conf->limit = atoi(optarg);
				} else {
					printf("Error: pid must be the first parameter.\n");
				}
			break;
			case 'x':
				if(pid > 0) {
					strncpy(conf->path, optarg, sizeof(conf->path));
				} else {
					printf("Error: pid must be the first parameter.\n");
				}
			break;
			case 'v':
				if(pid > 0) {
					printf("[x]Path: %s\n", conf->path);
					printf("[s]Static Level: %d\n", conf->staticGray == 0 ? conf->staticLevel : -conf->staticLevel);
					printf("[i]Static Info: %d\n", conf->staticInfo);
					printf("[d]Dynamic Level: %d\n", conf->dynamicGray == 0 ? conf->dynamicLevel : -conf->dynamicLevel);
					printf("[j]Dynamic Info: %d\n", conf->dynamicInfo);
					printf("[l]Limit: %d\n", conf->limit);
					printf("[m]Number: %d\n", conf->number);
					printf("[c]Compress: %s\n", conf->compress);
					printf("[f]Configure file: %s\n", conf->configureFile);
					printf("[n]Configure name: %s\n", conf->configureName);
					printf("[b]Hide: %s\n", conf->hide);
				} else {
					printf("Error: pid must be the first parameter.\n");
				}
			break;
			case 'f':
				{
					if(pid > 0) {
						strncpy(conf->configureFile,optarg,sizeof(conf->configureFile));
						if(jsonbuffer != NULL) free(jsonbuffer);
						jsonbuffer = fmalloc(conf->configureFile);
						if(jsonbuffer == NULL) {
							AdvLog_Configure_Export_To_File(conf, conf->configureFile);
							jsonbuffer = fmalloc(conf->configureFile);
						}
						//jsonconf.Release();
						//printf("<%s,%d> jsonbuffer = %s\n",__FILE__,__LINE__,jsonbuffer);
						//jsonconf = jsonbuffer;
						//jsonconf.Print();
						//printf("<%s,%d> jsonconf.IsNULL() = %d\n",__FILE__,__LINE__,jsonconf.IsNULL());
						AdvLog_Configure_Import_From_JSON(conf, jsonbuffer);
						
						if(jsonbuffer != NULL) free(jsonbuffer);
						jsonbuffer = NULL;
						
					} else {
						printf("Error: pid must be the first parameter.\n");
					}
				}
			break;
			case 'e':
				{
					if(pid > 0) {
						AdvLog_Configure_Export_To_File(conf, optarg);
					} else {
						printf("Error: pid must be the first parameter.\n");
					}
				}
			break;
			case '?':
			case 'h':
				PrintHelp(pid);
			return 1;
			default:
				return 1;
		}
	}

	for (index = optind; index < argc; index++)
		printf ("Non-option argument %s\n", argv[index]);
	return 0;
}

int AdvLog_Configure_GetPid() {
	return configure->pid;
}

int AdvLog_Configure_GetStaticLevel() {
	return configure->staticLevel;
}

int AdvLog_Configure_GetStaticGray() {
	return configure->staticGray;
}

int AdvLog_Configure_GetStaticInfo() {
	return configure->staticInfo;
}

int AdvLog_Configure_GetDynamicLevel() {
	return configure->dynamicLevel;
}

int AdvLog_Configure_GetDynamicGray() {
	return configure->dynamicGray;
}

int AdvLog_Configure_GetDynamicInfo() {
	return configure->dynamicInfo;
}
const char *AdvLog_Configure_GetPath() {
	return configure->path;
}
int AdvLog_Configure_GetLimit() {
	return configure->limit;
}
int AdvLog_Configure_Hide_Enable() {
	return configure->hide[0] == 0 ? 0 : 1;
}

int AdvLog_Configure_Is_Hiden(int level) {
	if(configure->hide[0] == 0) return 0;
	return strchr(configure->hide,level+48) == 0 ? 0 : 1;
}

void AdvLog_Configure_Uninit() {
	if(configure != NULL) {
		jsonconf.Release();
		unmountKeyMemory(configure);
		configure = NULL;
		if(externalConf != NULL) {
			unmountKeyMemory(externalConf);
			externalConf = NULL;
		}
	}
}