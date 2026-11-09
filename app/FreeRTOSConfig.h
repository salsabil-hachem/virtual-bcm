/*
 * FreeRTOSConfig.h
 * Reglages de FreeRTOS (version "POSIX" qui tourne sur un PC Linux).
 */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <limits.h>   /* PTHREAD_STACK_MIN */

#define configUSE_PREEMPTION                    1        /* une tache prioritaire interrompt les autres */
#define configTICK_RATE_HZ                      ( 1000 ) /* horloge de FreeRTOS : 1 tick = 1 ms         */
#define configMAX_PRIORITIES                    ( 5 )
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) PTHREAD_STACK_MIN )
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 64 * 1024 ) )
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configUSE_16_BIT_TICKS                  0
#define configUSE_MUTEXES                       1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_TIMERS                        0
#define configCHECK_FOR_STACK_OVERFLOW          0
#define configUSE_MALLOC_FAILED_HOOK            0

#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskDelayUntil                 1
#define INCLUDE_xTaskGetSchedulerState          1

#endif /* FREERTOS_CONFIG_H */
