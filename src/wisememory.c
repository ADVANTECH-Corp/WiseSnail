#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wisememory.h"
#include "wiseconfig.h"

static int dynamic = 0;
static char *buffer = 0;
static int size = 0;
static int current = 0;
static int remain = 0;

void WiseMem_Init(void *address, int len) {
    if(address == 0) {
        dynamic = 1;
        buffer = (char *)malloc(len);
    } else {
        dynamic = 0;
        buffer = (char *)address;
    }
    size = len;
    current = 0;
    remain = len;
    memset(buffer,0,len);
}

void *__WiseMem_Alloc(int len, char *file, int line) {
    char *result = NULL;
    //printf("\033[30;42malloc %d, remain %d , max %d <%s, %d>\033[0m\n", len, remain, size, file, line);
    
    if(len > MAX_ALLOC_SIZE) len = MAX_ALLOC_SIZE;
    while(len+sizeof(int) > remain) {
        if(dynamic) {
            remain+=4096;
            size+=4096;
            buffer = realloc(buffer, size);
        } else return NULL;
    }
    result = buffer + current;
    *(int *)result = len;
    
    result += sizeof(int);
    current += len + sizeof(int);
    remain -= len + sizeof(int);
    return result;
}

int WiseMem_Remain() {
    return remain;
}

int WiseMem_Current() {
    return current;
}

int WiseMem_Size(void *address) {
	char *a = (char *)address;
    a -= sizeof(int);
    return *(int *)a;
}

void WiseMem_Release() {
    memset(buffer,0,current);
    current = 0;
    remain = size;
}

void WiseMem_Destory() {
    if(dynamic == 1) {
        dynamic = 0;
        free(buffer);
    }
}