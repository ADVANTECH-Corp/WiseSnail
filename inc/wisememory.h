#ifndef WISEMEMORY_H_
#define WISEMEMORY_H_

#include <stdio.h>

void WiseMem_Init(void *address, int len);

void *__WiseMem_Alloc(int len, char *file, int line);

#define WiseMem_Alloc(len) __WiseMem_Alloc(len, __FILE__,__LINE__)

int WiseMem_Size(void *address);
void WiseMem_Release();
void WiseMem_Destory();


#endif //WISEMEMORY_H_