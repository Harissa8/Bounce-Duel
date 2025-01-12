#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "minirisc.h"
#include "harvey_platform.h"
#include "audio_server.h"


static inline int save_global_interrupts_and_disable()
{
	uint32_t v;
	__asm__ __volatile__("csrrci %0,mstatus,9" : "=r"(v) : : "memory");
	return (v | (v >> 3)) & 1;
}


static inline void restore_global_interrupts(int en)
{
	if (en) {
		__asm__ __volatile__("csrrsi zero,mstatus,9" : : : "memory");
	}
}


typedef struct {
	volatile const sound_sample_t *volatile sound_sample;
	volatile uint32_t   pos;
	volatile uint8_t    volume;
	volatile uint8_t    volume_left;
	volatile uint8_t    volume_right;
} mix_channel_t;


static mix_channel_t mix_channels[MIX_CHANNELS];
static uint32_t *mix_buffer_A = NULL;
static uint32_t *mix_buffer_B = NULL;
static  int32_t *mix_buffer = NULL;
static uint32_t *volatile mix_buffer_current;
static volatile uint8_t  mix_master_volume = MIX_MAX_VOLUME;
static uint32_t  mix_buffer_byte_size;

static StaticSemaphore_t audio_server_sem_buffer = {0};
static SemaphoreHandle_t audio_server_sem = NULL;

static uint32_t nextpowtwo(uint32_t v)
{
	v--;
	v |= v >>  1;
	v |= v >>  2;
	v |= v >>  4;
	v |= v >>  8;
	v |= v >> 16;
	v++;
	return v;
}


static void mix_clear_channel(mix_channel_t *channel)
{
	channel->sound_sample = NULL;
	channel->pos = 0;
	channel->volume = MIX_MAX_VOLUME;
	channel->volume_left = MIX_MAX_VOLUME;
	channel->volume_right = MIX_MAX_VOLUME;
}


static int Mix_OpenAudio(int frequency, int chunksize)
{
	if (mix_buffer_A) {
		free(mix_buffer_A);
		mix_buffer_A = NULL;
	}
	if (mix_buffer_B) {
		free(mix_buffer_B);
		mix_buffer_B = NULL;
	}
	if (mix_buffer) {
		free(mix_buffer);
		mix_buffer = NULL;
	}
	AUDIO->CR = 0;
	AUDIO->SR = 0;
	AUDIO->FREQUENCY = frequency;
	AUDIO->SAMPLE_FORMAT = 0x8010;
	AUDIO->NB_CHANNELS = 2;
	AUDIO->BUF_SAMPLE_SIZE = nextpowtwo(chunksize);
	AUDIO->BUF_A_ADDR = (void*)0x80000000;
	AUDIO->BUF_B_ADDR = (void*)0x80000000;

	AUDIO->CR = AUDIO_CR_EN | AUDIO_CR_PAUSE;
	if ((AUDIO->CR & AUDIO_CR_EN) == 0) {
		AUDIO->CR = 0;
		return -1;
	}

	mix_buffer_A = malloc(AUDIO->BUF_BYTE_SIZE);
	if (!mix_buffer_A) {
		return -1;
	}
	mix_buffer_B = malloc(AUDIO->BUF_BYTE_SIZE);
	if (!mix_buffer_B) {
		free(mix_buffer_A);
		mix_buffer_A = NULL;
		return -1;
	}
	mix_buffer_byte_size = AUDIO->BUF_SAMPLE_SIZE * AUDIO->NB_CHANNELS * sizeof(int32_t);
	mix_buffer = malloc(mix_buffer_byte_size);
	if (!mix_buffer) {
		free(mix_buffer_A);
		free(mix_buffer_B);
		mix_buffer_A = NULL;
		mix_buffer_B = NULL;
		return -1;
	}
	memset(mix_buffer_A, 0, AUDIO->BUF_BYTE_SIZE);
	memset(mix_buffer_B, 0, AUDIO->BUF_BYTE_SIZE);
	memset(mix_buffer,   0, mix_buffer_byte_size);

	AUDIO->BUF_A_ADDR = mix_buffer_A;
	AUDIO->BUF_B_ADDR = mix_buffer_B;

	for (int i = 0; i < MIX_CHANNELS; i++) {
		mix_clear_channel(&mix_channels[i]);
	}

	mix_master_volume = MIX_MAX_VOLUME;

	AUDIO->CR |= AUDIO_CR_IE;
	minirisc_enable_interrupt(AUDIO_INTERRUPT);

	return 0;
}


void audio_interrupt_handler()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	mix_buffer_current = (AUDIO->SR & AUDIO_SR_BUF_NUM) ? mix_buffer_B : mix_buffer_A;
	xSemaphoreGiveFromISR(audio_server_sem, &xHigherPriorityTaskWoken);
	AUDIO->SR = 0;
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


static void audio_mixer_task(void *arg)
{
	(void)arg;
	uint32_t mv, chv, chvl, chvr, bs, pos, maxpos, ppos;
	int32_t volume_left;
	int32_t volume_right;
	const int16_t *cbuf;
	int sample_stoped;

	uint32_t buf_nb_samples = AUDIO->BUF_SAMPLE_SIZE;

	while (1) {
		if (xSemaphoreTake(audio_server_sem, portMAX_DELAY) != pdTRUE) {
			goto audio_mixer_error;
		}

		int16_t *obuf = (int16_t*)mix_buffer_current;
		memset(mix_buffer, 0, mix_buffer_byte_size);

		mv = mix_master_volume;
		int nb_playing_channels = 0;
		for (int c = 0; c < MIX_CHANNELS; c++) {
			int gie = save_global_interrupts_and_disable();
			if (mix_channels[c].sound_sample) {
				cbuf = mix_channels[c].sound_sample->samples;
				bs   = mix_channels[c].sound_sample->nb_samples;
				chv  = mix_channels[c].volume;
				chvl = mix_channels[c].volume_left;
				chvr = mix_channels[c].volume_right;
				pos  = mix_channels[c].pos;
			} else {
				cbuf = NULL;
			}
			restore_global_interrupts(gie);
			if (!cbuf) {
				continue;
			}

			nb_playing_channels++;

			ppos = pos;
			volume_left  = (mv * chv * chvl) >> 5;
			volume_right = (mv * chv * chvr) >> 5;
			maxpos = bs;
			sample_stoped = 0;
			for (uint32_t i = 0; i < buf_nb_samples; i++, pos++) {
				if (pos >= maxpos) {
					sample_stoped = 1;
					break;
				}
				int32_t sample = cbuf[pos];
				mix_buffer[2*i+0] = ((sample * volume_left ) >> 16) + mix_buffer[2*i+0];
				mix_buffer[2*i+1] = ((sample * volume_right) >> 16) + mix_buffer[2*i+1];
			}
			if (pos >= maxpos) {
				sample_stoped = 1;
			}
			gie = save_global_interrupts_and_disable();
			if (mix_channels[c].sound_sample &&
				mix_channels[c].sound_sample->samples == cbuf &&
				mix_channels[c].pos == ppos) {
				if (sample_stoped) {
					mix_channels[c].sound_sample = NULL;
				} else {
					mix_channels[c].pos = pos;
				}
			}
			restore_global_interrupts(gie);
		}
		for (uint32_t i = 0; i < buf_nb_samples; i++) {
			int32_t sample_left  = mix_buffer[2*i+0];
			int32_t sample_right = mix_buffer[2*i+1];
			if (sample_left < -32768)
				sample_left = -32768;
			else if (sample_left > 32767)
				sample_left =  32767;
			if (sample_right < -32768)
				sample_right = -32768;
			else if (sample_right > 32767)
				sample_right =  32767;
			obuf[2*i+0] = (int16_t)sample_left;
			obuf[2*i+1] = (int16_t)sample_right;
		}
		if (nb_playing_channels == 0) {
			AUDIO->CR |= AUDIO_CR_PAUSE;
		}
	}

audio_mixer_error:

	while (1) {
		vTaskDelay(portMAX_DELAY);
	}
}


void init_audio_mixer(int audio_task_priority)
{
	static int audio_mixer_task_created = 0;

	if (audio_mixer_task_created) {
		return;
	}

	if (Mix_OpenAudio(24000, 512)) {
		return;
	}

	audio_server_sem = xSemaphoreCreateBinaryStatic(&audio_server_sem_buffer);

	xTaskCreate(audio_mixer_task, "audio_mixer", 1024, NULL, audio_task_priority, NULL);

	audio_mixer_task_created = 1;
}


int Mix_QuerySpec(int *frequency, uint16_t *format, int *channels)
{
	if ((AUDIO->CR & AUDIO_CR_EN) == 0) {
		return -1;
	}
	if (frequency) {
		*frequency = AUDIO->FREQUENCY;
	}
	if (format) {
		*format = AUDIO->SAMPLE_FORMAT;
	}
	if (channels) {
		*channels = AUDIO->NB_CHANNELS;
	}
	return 0;
}


int Mix_SetPanning(int channel, uint8_t left, uint8_t right)
{
	int gie;
	if (left > MIX_MAX_VOLUME) {
		left = MIX_MAX_VOLUME;
	}
	if (right > MIX_MAX_VOLUME) {
		right = MIX_MAX_VOLUME;
	}
	if (channel < 0) {
		gie = save_global_interrupts_and_disable();
		for (int i = 0; i < MIX_CHANNELS; i++) {
			mix_channels[i].volume_left  = left;
			mix_channels[i].volume_right = right;
		}
		restore_global_interrupts(gie);
		return 0;
	}
	if (channel >= MIX_CHANNELS) {
		return -1;
	}
	mix_channel_t *ch = &mix_channels[channel];
	gie = save_global_interrupts_and_disable();
	ch->volume_left  = left;
	ch->volume_right = right;
	restore_global_interrupts(gie);
	return 0;
}


int Mix_Volume(int channel, uint8_t volume)
{
	if (volume > MIX_MAX_VOLUME) {
		volume = MIX_MAX_VOLUME;
	}
	if (channel < 0) {
		int gie = save_global_interrupts_and_disable();
		for (int i = 0; i < MIX_CHANNELS; i++) {
			mix_channels[i].volume = volume;
		}
		restore_global_interrupts(gie);
		return 0;
	}
	if (channel >= MIX_CHANNELS) {
		return -1;
	}
	mix_channels[channel].volume = volume;
	return 0;
}


int Mix_MasterVolume(int volume)
{
	int prev_volume = mix_master_volume;
	if (volume < 0) {
		return prev_volume;
	}
	if (volume > MIX_MAX_VOLUME) {
		volume = MIX_MAX_VOLUME;
	}
	mix_master_volume = volume;
	return prev_volume;
}


int Mix_HaltChannel(int channel)
{
	if (channel < 0) {
		int gie = save_global_interrupts_and_disable();
		for (int i = 0; i < MIX_CHANNELS; i++) {
			mix_channels[i].sound_sample = NULL;
		}
		restore_global_interrupts(gie);
		return 0;
	}
	if (channel >= MIX_CHANNELS) {
		return -1;
	}
	mix_channels[channel].sound_sample = NULL;
	return 0;
}


int Mix_Playing(int channel)
{
	if (channel < 0) {
		int cnt = 0;
		int gie = save_global_interrupts_and_disable();
		for (int i = 0; i < MIX_CHANNELS; i++) {
			cnt += mix_channels[i].sound_sample != NULL ? 1 : 0;
		}
		restore_global_interrupts(gie);
		return cnt;
	}
	if (channel >= MIX_CHANNELS) {
		return 0;
	}
	return mix_channels[channel].sound_sample != NULL ? 1 : 0;
}


int Mix_PlayChannel(int channel, const sound_sample_t *sample, uint8_t volume)
{
	if ((AUDIO->CR & AUDIO_CR_EN) == 0 || channel >= MIX_CHANNELS || sample == NULL) {
		return -1;
	}
	mix_channel_t *ch = NULL;
	int gie = save_global_interrupts_and_disable();
	if (channel < 0) {
		for (int i = 0; i < MIX_CHANNELS; i++) {
			if (mix_channels[i].sound_sample == NULL) {
				ch = &mix_channels[i];
				break;
			}
		}
		if (ch == NULL) {
			restore_global_interrupts(gie);
			return -1;
		}
	} else {
		ch = &mix_channels[channel];
	}
	ch->sound_sample = sample;
	ch->pos = 0;
	if (ch->volume > MIX_MAX_VOLUME) {
		ch->volume = MIX_MAX_VOLUME;
	} else {
		ch->volume = volume;
	}
	AUDIO->CR &= ~AUDIO_CR_PAUSE;
	restore_global_interrupts(gie);
	return 0;
}


