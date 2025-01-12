/*
 * FreeRTOS Kernel V10.4.3
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 * 1 tab == 4 spaces!
 */

/* Removed the vTaskSuspendAll() / xTaskResumeAll() calls because
 * malloc reentrant protection stubs are allready provided (happens inside
 * malloc calls).
 */

/*
 * Implementation of pvPortMalloc() and vPortFree() that relies on the
 * compilers own malloc() and free() implementations.
 *
 * This file can only be used if the linker is configured to to generate
 * a heap memory area.
 *
 * See heap_1.c, heap_2.c and heap_4.c for alternative implementations, and the
 * memory management pages of https://www.FreeRTOS.org for more information.
 */

#include <stdlib.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
 * all the API functions to use the MPU wrappers.  That should only be done when
 * task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#if ( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
    #error This file must not be used if configSUPPORT_DYNAMIC_ALLOCATION is 0
#endif

/*-----------------------------------------------------------*/

void * pvPortMalloc( size_t xWantedSize )
{
    void * pvReturn;

    //vTaskSuspendAll();
    {
        pvReturn = malloc( xWantedSize );
        traceMALLOC( pvReturn, xWantedSize );
    }
    //( void ) xTaskResumeAll();

    #if ( configUSE_MALLOC_FAILED_HOOK == 1 )
        {
            if( pvReturn == NULL )
            {
                extern void vApplicationMallocFailedHook( void );
                vApplicationMallocFailedHook();
            }
        }
    #endif

    return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree( void * pv )
{
    if( pv )
    {
        //vTaskSuspendAll();
        {
            free( pv );
            traceFREE( pv, 0 );
        }
        //( void ) xTaskResumeAll();
    }
}


#if ( configSUPPORT_STATIC_ALLOCATION == 0 )
    #error This file must not be used if configSUPPORT_STATIC_ALLOCATION is 0
#endif

#if ( configUSE_RECURSIVE_MUTEXES == 0 )
    #error This file must not be used if configUSE_RECURSIVE_MUTEXES is 0
#endif

#include <malloc.h>
#include "semphr.h"
#include "xprintf.h"
#include "minirisc.h"

static StaticSemaphore_t malloc_recursive_mutex_buffer = {0};
static SemaphoreHandle_t malloc_recursive_mutex = NULL;

//#define DEBUG_PRINT_MALLOC_LOCKS

void __attribute__((constructor(101))) init_malloc_mutex()
{
//	xprintf("%s()\n", __FUNCTION__);
#ifdef DEBUG_PRINT_MALLOC_LOCKS
	xprintf("\e[93minit_malloc_mutex()\e[0m\n");
#endif
	malloc_recursive_mutex = xSemaphoreCreateRecursiveMutexStatic(&malloc_recursive_mutex_buffer);
}

#ifdef DEBUG_PRINT_MALLOC_LOCKS
static int cnt = 0;
#endif

void __malloc_lock(struct _reent *reent)
{
	(void)reent;
#ifdef DEBUG_PRINT_MALLOC_LOCKS
	xprintf("\e[93m__malloc_lock() %d\e[0m\n", cnt++);
#endif
	if (xSemaphoreTakeRecursive(malloc_recursive_mutex, portMAX_DELAY) != pdTRUE) {
		xprintf("\e[91;1mxSemaphoreTakeRecursive(malloc_recursive_mutex, portMAX_DELAY) FAILED!\e[0m\n");
		minirisc_halt();
	}
}

void __malloc_unlock(struct _reent *reent)
{
	(void)reent;
#ifdef DEBUG_PRINT_MALLOC_LOCKS
	xprintf("\e[93m__malloc_unlock() %d\e[0m\n", --cnt);
#endif
	xSemaphoreGiveRecursive(malloc_recursive_mutex);
}


//#define MY_MALLOC_HOOKS

#ifdef MY_MALLOC_HOOKS
#include <reent.h>

/* void *malloc(size_t size);
 * void free(void *ptr);
 * void *calloc(size_t nmemb, size_t size);
 * void *realloc(void *ptr, size_t size);
 * void *reallocarray(void *ptr, size_t nmemb, size_t size);
 */

void* malloc(size_t nbytes)
{
	void *r = _malloc_r(_REENT, nbytes);
	xprintf("\e[96mmalloc(%u) -> 0x%08x\e[0m\n", nbytes, (uint32_t)r);
	return r;
}


void free(void *ptr)
{
	xprintf("\e[96mfree(0x%08x)\e[0m\n", (uint32_t)ptr);
	_free_r(_REENT, ptr);
}


void* aligned_alloc(size_t align, size_t size)
{
	xprintf("\e[91maligned_alloc(%u, %u)\e[0m\n", align, size);
	return _memalign_r(_REENT, align, size);
}


void* calloc(size_t n, size_t size)
{
	xprintf("\e[91mcalloc(%u, %u)\e[0m\n", n, size);
	return _calloc_r(_REENT, n, size);
}


void* memalign(size_t align, size_t nbytes)
{
	xprintf("\e[91mmemalign(%u, %u)\e[0m\n", align, nbytes);
	return _memalign_r(_REENT, align, nbytes);
}


void* realloc(void *ap, size_t nbytes)
{
	xprintf("\e[91mrealloc(0x%08x, %u)\e[0m\n", (uint32_t)ap, nbytes);
	return _realloc_r(_REENT, ap, nbytes);
}


void* valloc(size_t nbytes)
{
	xprintf("\e[91mvalloc(%u)\e[0m\n", nbytes);
	return _valloc_r(_REENT, nbytes);
}


void* pvalloc(size_t nbytes)
{
	xprintf("\e[91mpvalloc(%u)\e[0m\n", nbytes);
	return _pvalloc_r(_REENT, nbytes);
}

#endif /* defined MY_MALLOC_HOOKS */

