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

.global xPortStartFirstTask
//.global timer_interrupt_handler
.global swi_interrupt_handler

.extern pxCurrentTCB
.extern pxCriticalNesting
.extern xCriticalNesting
.extern xTaskIncrementTick
.extern vTaskSwitchContext
.extern xISRStackTop

/*-----------------------------------------------------------*/

xPortStartFirstTask:
	/* Enable SoftWare Interrupt */
	addi  x5, zero, 1
	slli  x5, x5, 31
	csrrs zero, mie, x5

    lw    sp, pxCurrentTCB  /* lw pxCurrentTCB. */
    lw    sp, 0(sp)         /* Read sp from first TCB member. */

    lw    x1, 0(sp)         /* Note for starting the scheduler the exception return address is used as the function return address. */

    lw    x5,  29 * 4(sp)        /* Obtain xCriticalNesting value for this task from task's stack. */
    lw    x6, pxCriticalNesting  /* lw the address of xCriticalNesting into x6. */
    sw    x5, 0(x6)              /* Restore the critical nesting value for this task. */

    lw    x5,   2 * 4(sp)   /* Initial x5 (t0) value. */
    lw    x6,   3 * 4(sp)   /* Initial x6 (t1) value. */
    lw    x7,   4 * 4(sp)   /* t2 */
    lw    x8,   5 * 4(sp)   /* s0/fp */
    lw    x9,   6 * 4(sp)   /* s1 */
    lw    x10,  7 * 4(sp)   /* a0 */
    lw    x11,  8 * 4(sp)   /* a1 */
    lw    x12,  9 * 4(sp)   /* a2 */
    lw    x13, 10 * 4(sp)   /* a3 */
    lw    x14, 11 * 4(sp)   /* a4 */
    lw    x15, 12 * 4(sp)   /* a5 */
    lw    x16, 13 * 4(sp)   /* a6 */
    lw    x17, 14 * 4(sp)   /* a7 */
    lw    x18, 15 * 4(sp)   /* s2 */
    lw    x19, 16 * 4(sp)   /* s3 */
    lw    x20, 17 * 4(sp)   /* s4 */
    lw    x21, 18 * 4(sp)   /* s5 */
    lw    x22, 19 * 4(sp)   /* s6 */
    lw    x23, 20 * 4(sp)   /* s7 */
    lw    x24, 21 * 4(sp)   /* s8 */
    lw    x25, 22 * 4(sp)   /* s9 */
    lw    x26, 23 * 4(sp)   /* s10 */
    lw    x27, 24 * 4(sp)   /* s11 */
    lw    x28, 25 * 4(sp)   /* t3 */
    lw    x29, 26 * 4(sp)   /* t4 */
    lw    x30, 27 * 4(sp)   /* t5 */
    lw    x31, 28 * 4(sp)   /* t6 */

    addi  sp, sp, 30*4
	csrsi mstatus, 8             /* Global interrupts enabled from here */
    ret

/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

.section .minirisc_irq_handlers


//#if 0
//timer_interrupt_handler:
//    addi  sp, sp, -30*4
//    sw    x1,  1 * 4(sp)
//    sw    x5,  2 * 4(sp)
//    sw    x6,  3 * 4(sp)
//    sw    x7,  4 * 4(sp)
//    sw    x8,  5 * 4(sp)
//    sw    x9,  6 * 4(sp)
//    sw   x10,  7 * 4(sp)
//    sw   x11,  8 * 4(sp)
//    sw   x12,  9 * 4(sp)
//    sw   x13, 10 * 4(sp)
//    sw   x14, 11 * 4(sp)
//    sw   x15, 12 * 4(sp)
//    sw   x16, 13 * 4(sp)
//    sw   x17, 14 * 4(sp)
//    sw   x18, 15 * 4(sp)
//    sw   x19, 16 * 4(sp)
//    sw   x20, 17 * 4(sp)
//    sw   x21, 18 * 4(sp)
//    sw   x22, 19 * 4(sp)
//    sw   x23, 20 * 4(sp)
//    sw   x24, 21 * 4(sp)
//    sw   x25, 22 * 4(sp)
//    sw   x26, 23 * 4(sp)
//    sw   x27, 24 * 4(sp)
//    sw   x28, 25 * 4(sp)
//    sw   x29, 26 * 4(sp)
//    sw   x30, 27 * 4(sp)
//    sw   x31, 28 * 4(sp)
//    lw   t0, xCriticalNesting         /* Load the value of xCriticalNesting into t0. */
//    sw   t0, 29 * 4(sp) /* Store the critical nesting value to the stack. */
//    lw   t0, pxCurrentTCB             /* Load pxCurrentTCB. */
//    sw   sp, 0(t0)                    /* Write sp to first TCB member. */
//    csrr a1, mepc
//    sw   a1, 0 * 4(sp)                /* Asynchronous interrupt so save unmodified exception return address. */
////    lw   sp, xISRStackTop             /* Switch to ISR stack. */
//	/*----------*/
//
//    call  xTaskIncrementTick
//    beqz  a0, exit_without_context_switch    /* Don't switch context if incrementing tick didn't unblock a task. */
//	/* Pend a SWI */
//	addi  x5, zero, 1
//	slli  x5, x5, 31
//	csrrs zero, mip, x5
//exit_without_context_switch:
//	/* Clear timer interrupt pending bit */
//	li  t1, 0x22010004
//	sw  zero,0(t1)
//
//	/*----------*/
//    lw  t1, pxCurrentTCB                /* Load pxCurrentTCB. */
//    lw  sp, 0(t1)                      /* Read sp from first TCB member. */
//    /* Load tpc with the address of the instruction in the task to run next. */
//    lw  t0, 0 * 4(sp)
//    csrw mepc, t0
//
//    lw  t0, 29 * 4(sp)    /* Obtain xCriticalNesting value for this task from task's stack. */
//    lw  t1, pxCriticalNesting           /* Load the address of xCriticalNesting into t1. */
//    sw  t0, 0(t1)                     /* Restore the critical nesting value for this task. */
//
//    lw   x1,  1 * 4(sp)
//    lw   x5,  2 * 4(sp)
//    lw   x6,  3 * 4(sp)
//    lw   x7,  4 * 4(sp)
//    lw   x8,  5 * 4(sp)
//    lw   x9,  6 * 4(sp)
//    lw  x10,  7 * 4(sp)
//    lw  x11,  8 * 4(sp)
//    lw  x12,  9 * 4(sp)
//    lw  x13, 10 * 4(sp)
//    lw  x14, 11 * 4(sp)
//    lw  x15, 12 * 4(sp)
//    lw  x16, 13 * 4(sp)
//    lw  x17, 14 * 4(sp)
//    lw  x18, 15 * 4(sp)
//    lw  x19, 16 * 4(sp)
//    lw  x20, 17 * 4(sp)
//    lw  x21, 18 * 4(sp)
//    lw  x22, 19 * 4(sp)
//    lw  x23, 20 * 4(sp)
//    lw  x24, 21 * 4(sp)
//    lw  x25, 22 * 4(sp)
//    lw  x26, 23 * 4(sp)
//    lw  x27, 24 * 4(sp)
//    lw  x28, 25 * 4(sp)
//    lw  x29, 26 * 4(sp)
//    lw  x30, 27 * 4(sp)
//    lw  x31, 28 * 4(sp)
//    addi sp, sp, 30*4
//    mret
//#endif

/*-----------------------------------------------------------*/

swi_interrupt_handler:
    addi  sp, sp, -30*4
    sw    x1,  1 * 4(sp)
    sw    x5,  2 * 4(sp)
    sw    x6,  3 * 4(sp)
    sw    x7,  4 * 4(sp)
    sw    x8,  5 * 4(sp)
    sw    x9,  6 * 4(sp)
    sw   x10,  7 * 4(sp)
    sw   x11,  8 * 4(sp)
    sw   x12,  9 * 4(sp)
    sw   x13, 10 * 4(sp)
    sw   x14, 11 * 4(sp)
    sw   x15, 12 * 4(sp)
    sw   x16, 13 * 4(sp)
    sw   x17, 14 * 4(sp)
    sw   x18, 15 * 4(sp)
    sw   x19, 16 * 4(sp)
    sw   x20, 17 * 4(sp)
    sw   x21, 18 * 4(sp)
    sw   x22, 19 * 4(sp)
    sw   x23, 20 * 4(sp)
    sw   x24, 21 * 4(sp)
    sw   x25, 22 * 4(sp)
    sw   x26, 23 * 4(sp)
    sw   x27, 24 * 4(sp)
    sw   x28, 25 * 4(sp)
    sw   x29, 26 * 4(sp)
    sw   x30, 27 * 4(sp)
    sw   x31, 28 * 4(sp)
//    lw   t0, xCriticalNesting /* Load the value of xCriticalNesting into t0. */
	lw   t1, pxCriticalNesting
	lw   t0, 0(t1)
//
    sw   t0, 29 * 4(sp)       /* Store the critical nesting value to the stack. */
    lw   t0, pxCurrentTCB     /* Load pxCurrentTCB. */
    sw   sp, 0(t0)            /* Write sp to first TCB member. */
    csrr a1, mepc
    sw   a1, 0 * 4(sp)        /* Asynchronous interrupt so save unmodified exception return address. */
//    lw   sp, xISRStackTop     /* Switch to ISR stack. */
	/*----------*/

	/* Clear SWI pending bit */
	addi t1, zero, 1
	slli t1, t1, 31
	csrrc zero, mip, t1

    call vTaskSwitchContext

	/*----------*/
    lw  t1, pxCurrentTCB      /* Load pxCurrentTCB. */
    lw  sp, 0(t1)             /* Read sp from first TCB member. */
    /* Load tpc with the address of the instruction in the task to run next. */
    lw  t0, 0 * 4(sp)
    csrw mepc, t0

    lw  t0, 29 * 4(sp)        /* Obtain xCriticalNesting value for this task from task's stack. */
    lw  t1, pxCriticalNesting /* Load the address of xCriticalNesting into t1. */
    sw t0, 0(t1)              /* Restore the critical nesting value for this task. */

    lw   x1,  1 * 4(sp)
    lw   x5,  2 * 4(sp)
    lw   x6,  3 * 4(sp)
    lw   x7,  4 * 4(sp)
    lw   x8,  5 * 4(sp)
    lw   x9,  6 * 4(sp)
    lw  x10,  7 * 4(sp)
    lw  x11,  8 * 4(sp)
    lw  x12,  9 * 4(sp)
    lw  x13, 10 * 4(sp)
    lw  x14, 11 * 4(sp)
    lw  x15, 12 * 4(sp)
    lw  x16, 13 * 4(sp)
    lw  x17, 14 * 4(sp)
    lw  x18, 15 * 4(sp)
    lw  x19, 16 * 4(sp)
    lw  x20, 17 * 4(sp)
    lw  x21, 18 * 4(sp)
    lw  x22, 19 * 4(sp)
    lw  x23, 20 * 4(sp)
    lw  x24, 21 * 4(sp)
    lw  x25, 22 * 4(sp)
    lw  x26, 23 * 4(sp)
    lw  x27, 24 * 4(sp)
    lw  x28, 25 * 4(sp)
    lw  x29, 26 * 4(sp)
    lw  x30, 27 * 4(sp)
    lw  x31, 28 * 4(sp)
    addi sp, sp, 30*4
	mret

/*-----------------------------------------------------------*/

