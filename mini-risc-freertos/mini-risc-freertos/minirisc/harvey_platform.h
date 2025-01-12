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

/* This file contains the definitions of the Harvey platform's peripheral
 * from the point of view of the embedded (guest) software.
 */

#ifndef HARVEY_PLATFORM_H
#define HARVEY_PLATFORM_H

#include <stdint.h>

/************************* Charout device definition **************************/

typedef struct {
	volatile char     CHAR;
	volatile int32_t  DEC;
	volatile uint32_t HEX;
} charout_device_t;


/****************************** Timer definition ******************************/

typedef struct {
	volatile uint32_t CR;
	volatile uint32_t SR;
	volatile uint32_t CNT;
	volatile uint32_t ARR;
} timer_device_t;

/* Control register bits */
#define TIMER_CR_EN  0x00000001UL
#define TIMER_CR_IE  0x00000002UL

/* Status register bits */
#define TIMER_SR_UEF 0x00000001UL


/************************ Video controller definition *************************/

typedef struct {
	volatile uint32_t  CR;
	volatile uint32_t  SR;
	volatile uint32_t *DMA_ADDR;
	volatile uint32_t  WIDTH;
	volatile uint32_t  HEIGHT;
	volatile uint32_t  BPP;
	volatile uint32_t  PITCH;
	volatile uint32_t  RED_MASK;
	volatile uint32_t  GREEN_MASK;
	volatile uint32_t  BLUE_MASK;
} video_device_t;

/* Control register bits */
#define VIDEO_CR_EN    0x00000001UL
#define VIDEO_CR_IE    0x00000002UL

/* Status register bits */
#define VIDEO_SR_ERR   0x00000001UL
#define VIDEO_SR_SRF_P 0x00000002UL


/*********************** Keyboard controller definition ***********************/

typedef struct {
	volatile uint32_t CR;
	volatile uint32_t SR;
	volatile uint32_t DATA;
} keyboard_device_t;

/* Control register bits */
#define KEYBOARD_CR_IE             0x00000001UL

/* Status register bits */
#define KEYBOARD_SR_FIFO_NOT_EMPTY 0x00000001UL
#define KEYBOARD_SR_OVERFLOW       0x00000002UL

/* Data register bits */
#define KEYBOARD_SCAN_CODE(data)           (data & 0x000001ff)
#define KEYBOARD_KEY_CODE(data)            ((data >> 9) & 0x000001ff)
#define KEYBOARD_DATA_REPEAT               0x00100000
#define KEYBOARD_DATA_PRESSED              0x00200000
#define KEYBOARD_DATA_MODIFIER_LEFT_SHIFT  0x00400000
#define KEYBOARD_DATA_MODIFIER_RIGHT_SHIFT 0x00800000
#define KEYBOARD_DATA_MODIFIER_LEFT_CTRL   0x01000000
#define KEYBOARD_DATA_MODIFIER_RIGHT_CTRL  0x02000000
#define KEYBOARD_DATA_MODIFIER_LEFT_ALT    0x04000000
#define KEYBOARD_DATA_MODIFIER_RIGHT_ALT   0x08000000
#define KEYBOARD_DATA_MODIFIER_LEFT_GUI    0x10000000
#define KEYBOARD_DATA_MODIFIER_RIGHT_GUI   0x20000000
#define KEYBOARD_DATA_MODIFIER_NUM_LOCK    0x40000000
#define KEYBOARD_DATA_MODIFIER_CAPS_LOCK   0x80000000
#define KEYBOARD_DATA_MODIFIER_SHIFT       (KEYBOARD_MODIFIER_LEFT_SHIFT | KEYBOARD_MODIFIER_RIGHT_SHIFT)
#define KEYBOARD_DATA_MODIFIER_CTRL        (KEYBOARD_MODIFIER_LEFT_CTRL  | KEYBOARD_MODIFIER_RIGHT_CTRL)
#define KEYBOARD_DATA_MODIFIER_ALT         (KEYBOARD_MODIFIER_LEFT_ALT   | KEYBOARD_MODIFIER_RIGHT_ALT)
#define KEYBOARD_DATA_MODIFIER_GUI         (KEYBOARD_MODIFIER_LEFT_GUI   | KEYBOARD_MODIFIER_RIGHT_GUI)


/************************ Mouse controller definition *************************/

typedef struct {
	int16_t type;
	int8_t  amount_y;
	int8_t  amount_x;
	int16_t x;
	int16_t y;
} mouse_data_t;

typedef struct {
	volatile uint32_t     CR;
	volatile uint32_t     SR;
	volatile mouse_data_t DATA;
} mouse_device_t;

/* Control register bits */
#define MOUSE_CR_IE             0x00000001UL
#define MOUSE_CR_REL            0x00000002UL

/* Status register bits */
#define MOUSE_SR_FIFO_NOT_EMPTY 0x00000001UL
#define MOUSE_SR_OVERFLOW       0x00000002UL

/* Type field in mouse_data_t */
typedef enum {
	MOUSE_MOTION             = 1,
	MOUSE_BUTTON_LEFT_DOWN   = 2,
	MOUSE_BUTTON_LEFT_UP     = 3,
	MOUSE_BUTTON_MIDDLE_DOWN = 4,
	MOUSE_BUTTON_MIDDLE_UP   = 5,
	MOUSE_BUTTON_RIGHT_DOWN  = 6,
	MOUSE_BUTTON_RIGHT_UP    = 7,
	MOUSE_WHEEL              = 8
} mouse_event_type_t;


/************************ Audio controller definition *************************/

typedef struct {
	volatile uint32_t CR;
	volatile uint32_t SR;
	volatile uint32_t FREQUENCY;
	volatile uint32_t SAMPLE_FORMAT;
	volatile uint32_t NB_CHANNELS;
	volatile uint32_t BUF_SAMPLE_SIZE;
	volatile uint32_t BUF_BYTE_SIZE;
	volatile void    *BUF_A_ADDR;
	volatile void    *BUF_B_ADDR;
} audio_device_t;

/* Control register bits */
#define AUDIO_CR_EN           0x00000001UL
#define AUDIO_CR_PAUSE        0x00000002UL
#define AUDIO_CR_IE           0x00000004UL

/* Status register bits */
#define AUDIO_SR_BUF_CONSUMED 0x00000001UL
#define AUDIO_SR_BUF_NUM      0x00000002UL

/* Sample format register bits */
#define AUDIO_SAMPLE_FORMAT_SIGNED     0x8000
#define AUDIO_SAMPLE_FORMAT_BIG_ENDIAN 0x1000
#define AUDIO_SAMPLE_FORMAT_FLOAT      0x0100
#define AUDIO_SAMPLE_FORMAT_BIT_SIZE(data) (data & 0x00ff)


/************************* Real time clock definition *************************/

typedef struct {
	union {
		volatile struct {
			volatile uint32_t SEC_LOW;
			volatile uint32_t SEC_HIGH;
		};
		volatile uint64_t SEC;
	};
	uint32_t USEC;
	uint32_t RESERVED;
	union {
		volatile struct {
			volatile uint32_t NSEC_LOW;
			volatile uint32_t NSEC_HIGH;
		};
		volatile uint64_t NSEC;
	};
} rtc_device_t;


/********************* Block device controller definition *********************/

typedef struct {
	volatile uint32_t  CR;
	volatile uint32_t  SR;
	volatile uint32_t  DISK_SIZE;
	volatile void     *DMA_ADDR;
	volatile uint32_t  SECTOR_INDEX;
	volatile uint32_t  NB_SECTORS;
} blkdev_device_t;

/* Control register bits */
#define BLKDEV_CR_RD        0x00000001UL
#define BLKDEV_CR_WR        0x00000002UL
#define BLKDEV_CR_IE        0x00000100UL

/* Status register bits */
#define BLKDEV_SR_DONE      0x00000001UL
#define BLKDEV_SR_ERROR     0x00000002UL


/************************* UART controller definition *************************/

typedef struct {
	volatile uint32_t    CR;
	volatile uint32_t    SR;
	volatile uint32_t    DATA;
	volatile const void *TX_DMA_ADDR;
	volatile uint32_t    TX_DMA_SIZE;
} uart_device_t;

/* Control register bits */
#define UART_CR_RXIE        0x00000001UL
#define UART_CR_TXIE        0x00000002UL
#define UART_CR_TXDMASTART  0x00000100UL

/* Status register bits */
#define UART_SR_RXNE        0x00000001UL
#define UART_SR_TXE         0x00000002UL
#define UART_SR_TXERR       0x00000100UL


/****************** Network interface controller definition *******************/

typedef struct {
	void    *buffer;
	uint32_t len;
} nic_dma_descriptor_t;

typedef struct {
	volatile uint32_t CR;
	volatile uint32_t SR;
	volatile nic_dma_descriptor_t *RX_DESC_BASE;
	volatile uint32_t RX_DESC_LEN;
	volatile uint32_t RX_DESC_TAIL;
	volatile uint32_t RX_DESC_HEAD;
	volatile nic_dma_descriptor_t *TX_DESC_BASE;
	volatile uint32_t TX_DESC_LEN;
	volatile uint32_t TX_DESC_TAIL;
	volatile uint32_t TX_DESC_HEAD;
} nic_device_t;

/* Control register bits */
#define NIC_CR_EN         0x00000001
#define NIC_CR_RXIE       0x00000002
#define NIC_CR_TXIE       0x00000004

/* Status register bits */
#define NIC_SR_CON        0x00000001
#define NIC_SR_RX_DMA_ERR 0x00000002
#define NIC_SR_RX_STARVE  0x00000004
#define NIC_SR_RX_UPDATE  0x00000008
#define NIC_SR_TX_DMA_ERR 0x00000010
#define NIC_SR_TX_STARVE  0x00000020
#define NIC_SR_TX_UPDATE  0x00000040

/************************ Power controller definition *************************/

typedef struct {
	volatile uint32_t REQ;
} powerdown_device_t;

#define POWERDOWN_REQUEST 0xdeadbeef


/******************** Platform peripherals base addresses *********************/

#define CHAROUT_BASE   0x10000000UL
#define TIMER_BASE     0x22010000UL
#define VIDEO_BASE     0x22020000UL
#define KEYBOARD_BASE  0x22030000UL
#define MOUSE_BASE     0x22040000UL
#define AUDIO_BASE     0x22050000UL
#define RTC_BASE       0x22060000UL
#define BLKDEV_BASE    0x22070000UL
#define UART_BASE      0x22080000UL
#define NIC_BASE       0x22090000UL
#define POWERDOWN_BASE 0x220a0000UL


/**************************** Platform peripherals ****************************/

#define CHAROUT     ((volatile charout_device_t*)    CHAROUT_BASE)
#define TIMER       ((volatile timer_device_t*)      TIMER_BASE)
#define VIDEO       ((volatile video_device_t*)      VIDEO_BASE)
#define KEYBOARD    ((volatile keyboard_device_t*)   KEYBOARD_BASE)
#define MOUSE       ((volatile mouse_device_t*)      MOUSE_BASE)
#define AUDIO       ((volatile audio_device_t*)      AUDIO_BASE)
#define RTC         ((volatile rtc_device_t*)        RTC_BASE)
#define BLKDEV      ((volatile blkdev_device_t*)     BLKDEV_BASE)
#define UART        ((volatile uart_device_t*)       UART_BASE)
#define NIC         ((volatile nic_device_t *)       NIC_BASE)
#define POWERDOWN   ((volatile powerdown_device_t *) POWERDOWN_BASE)


/************************** Interrupt lines numbers ***************************/

#define AUDIO_INTERRUPT_NUMBER    16
#define MOUSE_INTERRUPT_NUMBER    17
#define KEYBOARD_INTERRUPT_NUMBER 18
#define VIDEO_INTERRUPT_NUMBER    19
#define TIMER_INTERRUPT_NUMBER    20
#define BLKDEV_INTERRUPT_NUMBER   21
#define UART_RX_INTERRUPT_NUMBER  22
#define UART_TX_INTERRUPT_NUMBER  23
#define NIC_RX_INTERRUPT_NUMBER   24
#define NIC_TX_INTERRUPT_NUMBER   25
#define SWI_INTERRUPT_NUMBER      31


/****************************** Interrupt masks *******************************/

#define AUDIO_INTERRUPT    (1UL << AUDIO_INTERRUPT_NUMBER)
#define MOUSE_INTERRUPT    (1UL << MOUSE_INTERRUPT_NUMBER)
#define KEYBOARD_INTERRUPT (1UL << KEYBOARD_INTERRUPT_NUMBER)
#define VIDEO_INTERRUPT    (1UL << VIDEO_INTERRUPT_NUMBER)
#define TIMER_INTERRUPT    (1UL << TIMER_INTERRUPT_NUMBER)
#define BLKDEV_INTERRUPT   (1UL << BLKDEV_INTERRUPT_NUMBER)
#define UART_RX_INTERRUPT  (1UL << UART_RX_INTERRUPT_NUMBER)
#define UART_TX_INTERRUPT  (1UL << UART_TX_INTERRUPT_NUMBER)
#define NIC_RX_INTERRUPT   (1UL << NIC_RX_INTERRUPT_NUMBER)
#define NIC_TX_INTERRUPT   (1UL << NIC_TX_INTERRUPT_NUMBER)
#define SWI_INTERRUPT      (1UL << SWI_INTERRUPT_NUMBER)


#endif /* HARVEY_PLATFORM_H */
