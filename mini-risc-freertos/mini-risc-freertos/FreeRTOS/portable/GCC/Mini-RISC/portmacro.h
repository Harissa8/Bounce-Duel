/* Copyright (C) 2023 Théotime Bollengier <theotime.bollengier@ensta-bretagne.fr>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef PORTMACRO_H
#define PORTMACRO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portSTACK_TYPE          uint32_t
#define portBASE_TYPE           int32_t
#define portUBASE_TYPE          uint32_t
#define portMAX_DELAY           ( TickType_t ) 0xffffffffUL
//#define portPOINTER_SIZE_TYPE   uint32_t

typedef portSTACK_TYPE StackType_t;
typedef portBASE_TYPE  BaseType_t;
typedef portUBASE_TYPE UBaseType_t;
typedef portUBASE_TYPE TickType_t;

/* Legacy type definitions. */
#define portCHAR            char
#define portFLOAT           float
#define portDOUBLE          double
#define portLONG            long
#define portSHORT           short

/* 32-bit tick type on a 32-bit architecture, so reads of the tick count do
 * not need to be guarded with a critical section. */
#define portTICK_TYPE_IS_ATOMIC 1
/*-----------------------------------------------------------*/

/* Architecture specifics. */
#define portSTACK_GROWTH            ( -1 )
#define portTICK_PERIOD_MS          ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT          16

/*-----------------------------------------------------------*/

/* Scheduler utilities. */
#define portYIELD()                                                  \
	({                                                               \
		register uint32_t __v = 0x80000000;                          \
		__asm__ __volatile__("csrrs zero, mip, %0" :: "rK"(__v) : "memory"); \
	})
#define portYIELD_WITHIN_API() portYIELD()
#define portEND_SWITCHING_ISR( xSwitchRequired ) do { if( xSwitchRequired ) portYIELD(); } while( 0 )
#define portYIELD_FROM_ISR( x ) portEND_SWITCHING_ISR( x )
/*-----------------------------------------------------------*/

/* Critical section management. */
#define portCRITICAL_NESTING_IN_TCB                             0

#if 0
#define portSET_INTERRUPT_MASK_FROM_ISR()                       0
#define portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedStatusValue ) ( void ) uxSavedStatusValue
#endif

#define portDISABLE_INTERRUPTS()    __asm volatile( "csrci mstatus,9" )
#define portENABLE_INTERRUPTS()     __asm volatile( "csrsi mstatus,9" )

extern size_t xCriticalNesting;
#define portENTER_CRITICAL()            \
{                                       \
    portDISABLE_INTERRUPTS();           \
    xCriticalNesting++;                 \
}

#define portEXIT_CRITICAL()             \
{                                       \
    xCriticalNesting--;                 \
    if( xCriticalNesting == 0 )         \
    {                                   \
        portENABLE_INTERRUPTS();        \
    }                                   \
}

/*-----------------------------------------------------------*/

/* Architecture specific optimisations. */
#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
    #define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#endif

#if( configUSE_PORT_OPTIMISED_TASK_SELECTION == 1 )

    /* Check the configuration. */
    #if( configMAX_PRIORITIES > 32 )
        #error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 32.  It is very rare that a system requires more than 10 to 15 difference priorities as tasks that share a priority will time slice.
    #endif

    /* Store/clear the ready priorities in a bit map. */
    #define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )
    #define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )

    /*-----------------------------------------------------------*/

    #define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities ) uxTopPriority = ( 31UL - __builtin_clz( uxReadyPriorities ) )

#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */


/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. These are
 * not necessary for to use this port.  They are defined so the common demo
 * files (which build with all the ports) will build. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )

/*-----------------------------------------------------------*/

#define portNOP()    __asm volatile( " nop " )
#define portINLINE   __inline

#ifndef portFORCE_INLINE
    #define portFORCE_INLINE inline __attribute__(( always_inline))
#endif

#define portMEMORY_BARRIER() __asm volatile( "" ::: "memory" )
/*-----------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */
