/* Copyright (C) 2023-2024 Théotime Bollengier <theotime.bollengier@ensta-bretagne.fr>
 *
 * This file is part of Harvey. <https://gitlab.ensta-bretagne.fr/mini-risc/harvey>
 *
 * Harvey is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Harvey is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Harvey.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "minirisc.h"
#include "harvey_platform.h"
#include "xprintf.h"


void minirisc_halt()
{
	POWERDOWN->REQ = POWERDOWN_REQUEST;
}


void minirisc_enable_global_interrupts()
{
	csr_setbits(mstatus, 0x00000009);
}


void minirisc_disable_global_interrupts()
{
	csr_clearbits(mstatus, 0x00000009);
}


void minirisc_wait_for_interrupt()
{
	__asm__ __volatile__("wfi");
}


void minirisc_enable_interrupt(uint32_t mask)
{
	csr_setbits(mie, mask);
}


void minirisc_disable_interrupt(uint32_t mask)
{
	csr_clearbits(mie, mask);
}


uint32_t minirisc_get_pending_interrupts()
{
	return csr_read(mip);
}


void minirisc_raise_software_interrupt()
{
	csr_setbits(mip, SWI_INTERRUPT);
}


void minirisc_clear_software_interrupt()
{
	csr_clearbits(mip, SWI_INTERRUPT);
}


uint64_t minirisc_nb_instruction_retired()
{
	uint32_t h1, h2, l;
	do {
		h1 = csr_read(minstreth);
		l  = csr_read(minstret);
		h2 = csr_read(minstreth);
	} while (h2 != h1);
	return ((uint64_t)h1 << 32) | (uint64_t)l;
}


void __attribute__((weak)) default_exception_handler()
{
	xprintf("default_exception_handler(), mepc: 0x%08x, mcause: %d, mtval: 0x%08x\n",
		 csr_read(mepc), csr_read(mcause), csr_read(mtval));
	minirisc_halt();
}


void __attribute__((weak)) default_interrupt_handler()
{
	xprintf("default_interrupt_handler(), trap number %u\n", csr_read(mcause));
	minirisc_halt();
}


void __attribute__((weak)) instruction_address_misaligned_exception_handler()
{
	xprintf("Instruction address misaligned exception @ PC: 0x%08x, target: 0x%08x\n",
		 csr_read(mepc), csr_read(mtval));
	minirisc_halt();
}


void __attribute__((weak)) instruction_access_fault_exception_handler()
{
	xprintf("Instruction access fault exception @ PC: 0x%08x\n", csr_read(mepc));
	minirisc_halt();
}


void __attribute__((weak)) illegal_instruction_exception_handler()
{
	xprintf("Illegal instruction exception @ PC: 0x%08x, instruction: 0x%08x\n",
		 csr_read(mepc), csr_read(mtval));
	minirisc_halt();
}


void __attribute__((weak)) load_address_misaligned_exception_handler()
{
	xprintf("Load address misaligned exception @ PC: 0x%08x, target: 0x%08x\n",
		 csr_read(mepc), csr_read(mtval));
	minirisc_halt();
}


void __attribute__((weak)) store_address_misaligned_exception_handler()
{
	xprintf("Store address misaligned exception @ PC: 0x%08x, target: 0x%08x\n",
		 csr_read(mepc), csr_read(mtval));
	minirisc_halt();
}


void __attribute__((weak)) load_access_fault_exception_handler()
{
	xprintf("Load access fault exception @ PC: 0x%08x, target: 0x%08x\n",
		 csr_read(mepc), csr_read(mtval));
	minirisc_halt();
}


void __attribute__((weak)) store_access_fault_exception_handler()
{
	xprintf("Store access fault exception @ PC: 0x%08x, target: 0x%08x\n",
		 csr_read(mepc), csr_read(mtval));
	minirisc_halt();
}


void __attribute__((weak)) environment_call_exception_handler()
{
	xprintf("Environment call exception @ PC: 0x%08x\n", csr_read(mepc));
}


void __attribute__((weak)) environment_break_exception_handler()
{
	xprintf("Environment break exception @ PC: 0x%08x\n", csr_read(mepc));
}


void __attribute__((weak,alias("default_interrupt_handler"))) audio_interrupt_handler();
void __attribute__((weak,alias("default_interrupt_handler"))) mouse_interrupt_handler();
void __attribute__((weak,alias("default_interrupt_handler"))) keyboard_interrupt_handler();
void __attribute__((weak,alias("default_interrupt_handler"))) video_interrupt_handler();
void __attribute__((weak,alias("default_interrupt_handler"))) timer_interrupt_handler();
void __attribute__((weak,alias("default_interrupt_handler"))) blkdev_interrupt_handler();
void __attribute__((weak,alias("default_interrupt_handler"))) uart_rx_interrupt_handler();
void __attribute__((weak,alias("default_interrupt_handler"))) uart_tx_interrupt_handler();
void __attribute__((weak,alias("default_interrupt_handler"))) nic_rx_interrupt_handler();
void __attribute__((weak,alias("default_interrupt_handler"))) nic_tx_interrupt_handler();
void __attribute__((weak,alias("default_interrupt_handler"))) swi_interrupt_handler();


