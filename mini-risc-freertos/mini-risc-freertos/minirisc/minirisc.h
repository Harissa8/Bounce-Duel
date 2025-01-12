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

#ifndef MINIRISC_H
#define MINIRISC_H

#include <stdint.h>


void minirisc_halt();
void minirisc_enable_global_interrupts();
void minirisc_disable_global_interrupts();
void minirisc_wait_for_interrupt();
void minirisc_enable_interrupt(uint32_t mask);
void minirisc_disable_interrupt(uint32_t mask);
uint32_t minirisc_get_pending_interrupts();
void minirisc_raise_software_interrupt();
void minirisc_clear_software_interrupt();
uint64_t minirisc_nb_instruction_retired();


void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) default_exception_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) instruction_address_misaligned_exception_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) instruction_access_fault_exception_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) illegal_instruction_exception_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) load_address_misaligned_exception_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) store_address_misaligned_exception_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) load_access_fault_exception_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) store_access_fault_exception_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) environment_call_exception_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) environment_break_exception_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) default_interrupt_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) audio_interrupt_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) mouse_interrupt_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) keyboard_interrupt_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) video_interrupt_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) timer_interrupt_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) blkdev_interrupt_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) uart_rx_interrupt_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) uart_tx_interrupt_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) nic_rx_interrupt_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) nic_tx_interrupt_handler();
void __attribute__((section(".minirisc_irq_handlers"),interrupt("machine"))) swi_interrupt_handler();


/* Intrinsincs function definitions */

// Taken from OpenSBI.
// According to https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html :
// "Using the "memory" clobber effectively forms a read/write memory barrier for the compiler."

#define csr_swap(csr_num, value)                          \
	({                                                    \
		uint32_t __v = (uint32_t)value;                   \
		__asm__ __volatile__("csrrw %0, " #csr_num ", %1" \
				: "=r"(__v)                               \
				: "rK"(__v)                               \
				: "memory");                              \
		__v;                                              \
	})

#define csr_read(csr_num)                         \
	({                                            \
		register uint32_t __v;                    \
		__asm__ __volatile__("csrr %0, " #csr_num \
				: "=r"(__v)                       \
				:                                 \
				: "memory");                      \
		__v;                                      \
	})

#define csr_write(csr_num, value)                    \
	({                                               \
		uint32_t __v = (uint32_t)value;              \
		__asm__ __volatile__("csrw " #csr_num ", %0" \
				:                                    \
				: "rK"(__v)                          \
				: "memory");                         \
	})

#define csr_read_and_setbits(csr_num, value)              \
	({                                                    \
		uint32_t __v = (uint32_t)value;                   \
		__asm__ __volatile__("csrrs %0, " #csr_num ", %1" \
				: "=r"(__v)                               \
				: "rK"(__v)                               \
				: "memory");                              \
		__v;                                              \
	})

#define csr_setbits(csr_num, value)                  \
	({                                               \
		uint32_t __v = (uint32_t)value;              \
		__asm__ __volatile__("csrs " #csr_num ", %0" \
				:                                    \
				: "rK"(__v)                          \
				: "memory");                         \
	})

#define csr_read_and_clearbits(csr_num, value)            \
	({                                                    \
		uint32_t __v = (uint32_t)value;                   \
		__asm__ __volatile__("csrrc %0, " #csr_num ", %1" \
				: "=r"(__v)                               \
				: "rK"(__v)                               \
				: "memory");                              \
		__v;                                              \
	})

#define csr_clearbits(csr_num, value)                \
	({                                               \
		uint32_t __v = (uint32_t)value;              \
		__asm__ __volatile__("csrc " #csr_num ", %0" \
				:                                    \
				: "rK"(__v)                          \
				: "memory");                         \
	})


#endif /* MINIRISC_H */
