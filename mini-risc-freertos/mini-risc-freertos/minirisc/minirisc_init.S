.section .minirisc_init,"ax"
.global _minirisc_init
.type   _minirisc_init, @function


_minirisc_init:
	# Set trap handler table
	la t1, trap_vector
	ori t1, t1, 1 # use vectorized mode
	csrw mtvec, t1

	# Set stack pointer to the end of the main memory
	la sp, __stack_top

	# Call libc's _start routine, which will eventually call main()
	call _start

	/* Powerdown */
	li t3,0x220a0000
	li t2,0xdeadbeef
	sw t2,0(t3)
	

.section .minirisc_irq_handlers

.align 4
trap_vector:
	j default_exception_handler
	j instruction_address_misaligned_exception_handler
	j instruction_access_fault_exception_handler
	j illegal_instruction_exception_handler
	j load_address_misaligned_exception_handler
	j store_address_misaligned_exception_handler
	j load_access_fault_exception_handler
	j store_access_fault_exception_handler
	j environment_call_exception_handler
	j environment_break_exception_handler
	j default_exception_handler
	j default_exception_handler
	j default_exception_handler
	j default_exception_handler
	j default_exception_handler
	j default_exception_handler
	j audio_interrupt_handler
	j mouse_interrupt_handler
	j keyboard_interrupt_handler
	j video_interrupt_handler
	j timer_interrupt_handler
	j blkdev_interrupt_handler
	j uart_rx_interrupt_handler
	j uart_tx_interrupt_handler
	j nic_rx_interrupt_handler
	j nic_tx_interrupt_handler
	j default_interrupt_handler
	j default_interrupt_handler
	j default_interrupt_handler
	j default_interrupt_handler
	j default_interrupt_handler
	j swi_interrupt_handler

