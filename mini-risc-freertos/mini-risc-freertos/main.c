#include <string.h>
#include <stdio.h>
#include <time.h>
#include<stdlib.h>
#include "minirisc.h"
#include "harvey_platform.h"
#include "xprintf.h"
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//pour les fichiers audio
#include "audio_server.h" //init_audio_mixer(int audio_task_priority)
#include "samples.h"      //définit le format des echantillions attendu par le mixeur audios 


#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

static uint32_t frame_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
volatile uint32_t color = 0x00ff0000;

static SemaphoreHandle_t video_tx_sem   = NULL;
//static SemaphoreHandle_t key_tx_sem   = NULL;
void init_video()
{
	memset(frame_buffer, 0, sizeof(frame_buffer)); // clear frame buffer to black
	VIDEO->WIDTH  = SCREEN_WIDTH;
	VIDEO->HEIGHT = SCREEN_HEIGHT;
	VIDEO->DMA_ADDR = frame_buffer;
	VIDEO->CR = VIDEO_CR_IE | VIDEO_CR_EN;
	video_tx_sem = xSemaphoreCreateBinary();
	//key_tx_sem = xSemaphoreCreateBinary();
}


/* Hue must be between 0 and 1536 not included */
uint32_t hue_to_color(unsigned int hue)
{
	uint32_t r, g, b;
	if (hue < 256) {
		r = 255;
		g = hue;
		b = 0;
	} else if (hue < 512) {
		r = 511 - hue;
		g = 255;
		b = 0;
	} else if (hue < 768) {
		r = 0;
		g = 255;
		b = hue - 512;
	} else if (hue < 1024) {
		r = 0;
		g = 1023 - hue;
		b = 255;
	} else if (hue < 1280) {
		r = hue - 1024;
		g = 0;
		b = 255;
	} else if (hue < 1536) {
		r = 255;
		g = 0;
		b = 1535 - hue;
	} else {
		r = 0;
		g = 0;
		b = 0;
	}
	return (r << 16) | (g << 8) | b;
}



void video_interrupt_handler()
{
	static unsigned int hue = 0;
	VIDEO->SR = 0;
	
	color = hue_to_color(hue);
	hue += 7;
	if (hue >= 1536) {
		hue -= 1536;
	}
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	//Instead of refreshenvent give
        xSemaphoreGiveFromISR(video_tx_sem, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void draw_square(int x, int y, int width,int height, uint32_t color)
{
	if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
		return;
	}
	int i, j;
	int x_start = x < 0 ? 0 : x;
	int y_start = y < 0 ? 0 : y;
	int x_end = x + width;
	int y_end = y + height;
	if (x_end > SCREEN_WIDTH) {
		x_end = SCREEN_WIDTH;
	}
	if (y_end > SCREEN_HEIGHT) {
		y_end = SCREEN_HEIGHT;
	}
	for (j = y_start; j < y_end; j++) {
		for (i = x_start; i < x_end; i++) {
			frame_buffer[j*SCREEN_WIDTH + i] = color;
		}
	}
}


void draw_disk(int x, int y, int diam, uint32_t color)
{
	if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
		return;
	}
	int i, j;
	int x_start = x < 0 ? 0 : x;
	int y_start = y < 0 ? 0 : y;
	int x_end = x + diam;
	int y_end = y + diam;
	int rad2 = diam*diam/4;
	int xc = x + diam / 2;
	int yc = y + diam / 2;
	if (x_end > SCREEN_WIDTH) {
		x_end = SCREEN_WIDTH;
	}
	if (y_end > SCREEN_HEIGHT) {
		y_end = SCREEN_HEIGHT;
	}
	for (j = y_start; j < y_end; j++) {
		int j2 = (yc - j)*(yc - j);
		for (i = x_start; i < x_end; i++) {
			int i2 = (xc - i)*(xc - i);
			if (j2 + i2 <= rad2) {
				frame_buffer[j*SCREEN_WIDTH + i] = color;
			}
		}
	}
}


volatile int mouse_x = 0;
volatile int mouse_y = 0;
volatile int mouse_draw = 0;
volatile int brush_radius = 3;

void mouse_interrupt_handler()
{
	static int left_button_is_pressed = 0;
	mouse_data_t mouse_event;
	while (MOUSE->SR & MOUSE_SR_FIFO_NOT_EMPTY) {
		mouse_event = MOUSE->DATA;
		switch (mouse_event.type) {
			case MOUSE_MOTION:
				if (left_button_is_pressed) {
					mouse_x = mouse_event.x;
					mouse_y = mouse_event.y;
					mouse_draw = 1;
				}
				break;
			case MOUSE_BUTTON_LEFT_DOWN:
				left_button_is_pressed = 1;
				mouse_x = mouse_event.x;
				mouse_y = mouse_event.y;
				mouse_draw = 1;
				break;
			case MOUSE_BUTTON_LEFT_UP:
				left_button_is_pressed = 0;
				break;
			case MOUSE_WHEEL:
				brush_radius += mouse_event.amount_y;
				if (brush_radius < 1) {
					brush_radius = 1;
				} else if (brush_radius > 64) {
					brush_radius = 64;
				}
				break;
		}
	}
}

volatile uint32_t kdata= 0;
void keyboard_interrupt_handler()
{

	while (KEYBOARD->SR & KEYBOARD_SR_FIFO_NOT_EMPTY) {
		kdata = KEYBOARD->DATA;
		if (kdata & KEYBOARD_DATA_PRESSED) {
		       // BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                       // xSemaphoreGiveFromISR(key_tx_sem, &xHigherPriorityTaskWoken);
	               // portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			xprintf("key code: %d\n", KEYBOARD_KEY_CODE(kdata));
			switch (KEYBOARD_KEY_CODE(kdata)) {
				case 113: // Q
					minirisc_halt();
					break;
				case 32: // space
					memset(frame_buffer, 0, sizeof(frame_buffer)); // clear frame buffer
					break;
			}
		}
	}
}
#define black  0x00000000
#define white  0xffffffff
#define diam 20

volatile int x = 41;
volatile int y = 41;
volatile int xmouse=SCREEN_WIDTH/2;
volatile int xkey=SCREEN_WIDTH/2;
void aff_task(void *arg)
{
	(void)arg;
       // uint32_t black = 0x00000000;
       // uint32_t white = 0xffffffff;
       // int x = 0,y = 0;
	int ox, oy;
	//int diam = 20;
        //int xmouse=SCREEN_WIDTH/2;
        draw_square(SCREEN_WIDTH/2, SCREEN_HEIGHT-20, 70,20,white);
	draw_square(SCREEN_WIDTH/2, 0, 70,20,white);
	while (1) {

                        xSemaphoreTake(video_tx_sem, portMAX_DELAY);
                       // xSemaphoreTake(key_tx_sem, portMAX_DELAY);
			// draw
			draw_square(ox, oy, diam+8,diam+8 ,black);
                        draw_disk(x, y, diam, color);
                        ox = x;
                        oy = y;
 			draw_square(xmouse, SCREEN_HEIGHT-20, 70,20,black);
			int new_mousex = mouse_x;
			int new_keyx;
                        draw_square(new_mousex, SCREEN_HEIGHT-20, 70,20,white);
                        xmouse=new_mousex;
			mouse_draw = 0;
			switch (KEYBOARD_KEY_CODE(kdata)) {
				case 80: // left
                                 draw_square(xkey, 0, 70,20,black);
			         new_keyx = xkey-10;
                                 draw_square(new_keyx, 0, 70,20,white);
                                 xkey=new_keyx;
					break;
				case 79: // right
                                 draw_square(xkey, 0, 70,20,black);
			         new_keyx = xkey+10;
                                 draw_square(new_keyx, 0, 70,20,white);
                                 xkey=new_keyx;
					break;
			}
			
			
		}
}
	

void T_task(void *arg)
{
	(void)arg;
	int xdir = 1;
	int ydir = 1;
        
        int sample_num = 0;
	//pour tester la collision
	int restart=1;
	int ask_again=1;
	int R=1;
	int p1=0;
	int p2=0;

while(restart)
{
	while (R) {
	           vTaskDelay(MS2TICKS(10));
		   x += xdir;
		   y += ydir;
		   int bump = 0;

	
		  if (x >= (SCREEN_WIDTH - diam)) {
			xdir = -1;
			bump = 1;
		  } else if (x < 0) {
			xdir =  1;
			bump = 1;
		  }
		  if (y >= (SCREEN_HEIGHT - diam)) {
			ydir = -1;
			bump = 1;
		  } else if (y < 0) {
			ydir = 1;
			bump = 1;
		  }
		  if(y>=(SCREEN_HEIGHT-20-diam))
		  {
		    if(x < (xmouse+70) && x > xmouse)
		    {
		      	ydir=-1;
			bump = 1;
		    
		    }
		    else{ 
		    //xprintf("xmouse = %d, x = %d\n", xmouse, x); 
		    p1++;
		    xprintf("Upper_Player Win \n");
		    xprintf("Score: UP %d - LP %d \n",p1,p2);
		    R=0;
		    ask_again=1;
		    }
		   }
		   if(y<=20){
		    if(x < (xkey+70) && x > xkey)
		    {
		      	ydir=1;
		      	bump=1;
		    
		    }
		    else{ 
		    //xprintf("xmouse = %d, x = %d\n", xmouse, x); 
		    p2++;
		    xprintf("Lower_Player Win \n");
		    xprintf("Score: UP %d - LP %d \n",p1,p2);		    
		    R=0;
		    ask_again=1;
		    
		    }			    	  
		  }

		   if(bump){
		     Mix_PlayChannel(-1, sound_samples[sample_num], MIX_MAX_VOLUME);
		     //sample_num = (sample_num + 1)%(NB_SOUND_SAMPLES - 4);
		   }
		  
	}
      
      while(ask_again)
      {
        xprintf("Play again?Press SPACE \n");
        xprintf("End the game?Press ESC \n");
        switch (KEYBOARD_KEY_CODE(kdata)){
		case 32: // SPACE
		
                restart=1;
                R=1;
                ask_again=0;
                x=41;
                y=41;
                xdir=1;
                ydir=1;
		break;
		
		case 27: // ESC
                restart=0;
                ask_again=0;
		break;
		
		default:
		xprintf("Waiting...\n");
		vTaskDelay(MS2TICKS(10000));
		ask_again=1;
		break;                       
	    }
	}		
    }
}



int main()
{
	init_video();
	//initialisation du controleur audio et creation d'une tache pour mixer l'audio
	// il faut lui associé une priorité le plus haute
	init_audio_mixer(4);

	
	MOUSE->CR |= MOUSE_CR_IE;
	KEYBOARD->CR |= KEYBOARD_CR_IE;
        
       

	minirisc_enable_interrupt(VIDEO_INTERRUPT | MOUSE_INTERRUPT | KEYBOARD_INTERRUPT );

	minirisc_enable_global_interrupts();
        init_uart();

	xTaskCreate(aff_task, "hello", 1024, NULL, 1, NULL);
	xTaskCreate(T_task,  "echo",  1024, NULL, 1, NULL);
	vTaskStartScheduler();



	return 0;
}


