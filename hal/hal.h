#ifndef __HAL_H
#define __HAL_H

#include "infra_types.h"


void HAL_Free(void *ptr);
void *HAL_Malloc(uint32_t size);
void *HAL_MutexCreate(void);
void HAL_MutexDestroy(void *mutex);
void HAL_MutexLock(void *mutex);
void HAL_MutexUnlock(void *mutex);
void HAL_Printf(const char *fmt, ...);


#endif