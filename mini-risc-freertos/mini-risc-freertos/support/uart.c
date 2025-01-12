#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "harvey_platform.h"
#include "minirisc.h"
#include "xprintf.h"
#include "uart.h"


#define UART_RX_QUEUE_LEN 128


static QueueHandle_t     uart_rx_queue = NULL;
static SemaphoreHandle_t uart_tx_mutex = NULL;
static SemaphoreHandle_t uart_tx_sem   = NULL;


void uart_rx_interrupt_handler()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	char c;

	while (UART->SR & UART_SR_RXNE) {
		c = UART->DATA;
		xQueueSendFromISR(uart_rx_queue, &c, &xHigherPriorityTaskWoken);
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void uart_tx_interrupt_handler()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	UART->CR &= ~UART_CR_TXIE;
	xSemaphoreGiveFromISR(uart_tx_sem, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


ssize_t uart_read(char *ptr, size_t len)
{
	if (len <= 0)
		return 0;

	ssize_t r = 0;

	if (xQueueReceive(uart_rx_queue, &ptr[0], portMAX_DELAY) == pdTRUE) {
		r = 1;
		while ((size_t)r < len) {
			if (xQueueReceive(uart_rx_queue, &ptr[r], 0) == pdTRUE)
				r++;
			else
				break;
		}
	}

	if (r < (ssize_t)len)
		ptr[r] = '\0';

	return r;
}


static ssize_t uart_write_no_OS(const char *ptr, size_t len)
{
	while ((UART->SR & UART_SR_TXE) == 0);
	UART->TX_DMA_ADDR = ptr;
	UART->TX_DMA_SIZE = len;
	UART->CR |= UART_CR_TXDMASTART;
	while ((UART->SR & UART_SR_TXE) == 0);
	return len;
}


static ssize_t uart_write_with_OS(const char *ptr, size_t len)
{
	UART->TX_DMA_ADDR = ptr;
	UART->TX_DMA_SIZE = len;
	UART->CR |= UART_CR_TXDMASTART;
	UART->CR |= UART_CR_TXIE;
	xSemaphoreTake(uart_tx_sem, portMAX_DELAY);
	return len;
}


ssize_t uart_write(const char *ptr, size_t len)
{
	ssize_t r = -1;
	if (xSemaphoreTake(uart_tx_mutex, portMAX_DELAY) != pdTRUE) {
		xprintf("\e[91;1mxSemaphoreTake(uart_tx_mutex, portMAX_DELAY) FAILED!\e[0m\n");
		minirisc_halt();
	}
	if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
		r = uart_write_with_OS(ptr, len);
	else
		r = uart_write_no_OS(ptr, len);
	xSemaphoreGive(uart_tx_mutex);
	return r;
}


void init_uart()
{
	uart_tx_mutex = xSemaphoreCreateMutex();
	uart_tx_sem   = xSemaphoreCreateBinary();
	uart_rx_queue = xQueueCreate(UART_RX_QUEUE_LEN, sizeof(char));

	UART->CR = UART_CR_RXIE;

	minirisc_enable_interrupt(UART_RX_INTERRUPT | UART_TX_INTERRUPT);
}

