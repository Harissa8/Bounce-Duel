#include <string.h>
#include <sys/types.h>
#include "FreeRTOS.h"
#include "minirisc.h"
#include "harvey_platform.h"
#include "task.h"
#include "task.h"
#include "semphr.h"
#include "xprintf.h"

#if ( configAPPLICATION_ALLOCATED_HEAP == 1 )
//uint8_t ucHeap[configTOTAL_HEAP_SIZE] __attribute__((section(".ram_d1")));
uint8_t ucHeap[configTOTAL_HEAP_SIZE];
#endif


#if (configSUPPORT_STATIC_ALLOCATION == 1)
/* External Idle and Timer task static memory allocation functions */

/* vApplicationGetIdleTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
 * equals to 1 and is required for static memory allocation support.
 */
__attribute__((weak)) void vApplicationGetIdleTaskMemory (StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
	/* Idle task control block and stack */
	static StaticTask_t Idle_TCB;
	static StackType_t  Idle_Stack[configMINIMAL_STACK_SIZE];

	*ppxIdleTaskTCBBuffer   = &Idle_TCB;
	*ppxIdleTaskStackBuffer = &Idle_Stack[0];
	*pulIdleTaskStackSize   = (uint32_t)configMINIMAL_STACK_SIZE;
}

/* vApplicationGetTimerTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
 * equals to 1 and is required for static memory allocation support.
 */
__attribute__((weak)) void vApplicationGetTimerTaskMemory (StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
	/* Timer task control block and stack */
	static StaticTask_t Timer_TCB;
	static StackType_t  Timer_Stack[configTIMER_TASK_STACK_DEPTH];

	*ppxTimerTaskTCBBuffer   = &Timer_TCB;
	*ppxTimerTaskStackBuffer = &Timer_Stack[0];
	*pulTimerTaskStackSize   = (uint32_t)configTIMER_TASK_STACK_DEPTH;
}
#endif


#if ( configGENERATE_RUN_TIME_STATS == 1 )
void configure_timer_for_run_time_stats()
{
}

configRUN_TIME_COUNTER_TYPE get_run_time_counter_value()
{
	/* Return time in nano-seconds since emulator startup.
	 * 1h 11m 35s before 32-bit warp-arround.
	 */
	return (configRUN_TIME_COUNTER_TYPE)RTC->NSEC;
}
#endif


#if ( configCHECK_FOR_STACK_OVERFLOW != 0 )
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	(void)xTask;
	minirisc_disable_global_interrupts();
	xprintf("\e[1;31mTStack overflow for task \"%s\"!\e[0m\n", pcTaskName);
	minirisc_halt();
}
#endif


#if ( configUSE_MALLOC_FAILED_HOOK != 0 )
void vApplicationMallocFailedHook()
{
	minirisc_disable_global_interrupts();
	xprintf("\e[1;31mMalloc failed hook!\e[0m\n");
	minirisc_halt();
}
#endif


#if ( configUSE_IDLE_HOOK != 0 )
void vApplicationIdleHook()
{
	minirisc_wait_for_interrupt();
}
#endif


int usleep(useconds_t usec)
{
	vTaskDelay(MS2TICKS(usec / 1000UL));
	return 0;
}


