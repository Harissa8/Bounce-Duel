#ifndef MINIRISC_AUDIO_SERVER
#define MINIRISC_AUDIO_SERVER

#include <stdint.h>
#include "samples.h"

#define MIX_MAX_VOLUME 128
#define MIX_CHANNELS    48

/**
 * Initialise the audio device and create the audio mixer task.
 * Must be called before vTaskStartScheduler() and any Mix_* function is called.
 * \param audio_task_priority the audio mixer task priority. It Should be set to the highest priority so that audio do not glitch.
 */
void init_audio_mixer(int audio_task_priority);

/**
 * Start playing a sound sample, at the specified channle with the given volume.
 * If channel is negative, try to find a free channel.
 * \param volume Main volume of the channel, between 0 and MIX_MAX_VOLUME.
 * \return 0 on success, -1 on error.
 */
int Mix_PlayChannel(int channel, const sound_sample_t *sample, uint8_t volume);

/**
 * Stop playing the specified channel.
 * If channel is negative, stops of all channels.
 * \return 0 on success, -1 on error.
 */
int Mix_HaltChannel(int channel);

/**
 * Query if the specified channel is playing.
 * If the specified channel is negative, returns the number of playing channels.
 * \return 1 if the channel is playing, 0 otherwise.
 */
int Mix_Playing(int channel);

/**
 * Query the audio mixer parameters.
 * \return 0 on success, -1 on error.
 */
int Mix_QuerySpec(int *frequency, uint16_t *format, int *channels);

/**
 * Set the left and right volume of a given channel.
 * If channel is negative, set the panning of all channels.
 * \param left Volume of the left side, between 0 and MIX_MAX_VOLUME.
 * \param right Volume of the right side, between 0 and MIX_MAX_VOLUME.
 * \return 0 on success, -1 on error.
 */
int Mix_SetPanning(int channel, uint8_t left, uint8_t right);

/**
 * Set the main volume of a given channel.
 * If channel is negative, set the main volume of all channels.
 * \param volume Main volume of the channel, between 0 and MIX_MAX_VOLUME.
 * \return 0 on success, -1 on error.
 */
int Mix_Volume(int channel, uint8_t volume);

/**
 * Set the global volume of the whole mixer.
 * If the specified volume is negative, do not change the volume
 * and return the current volume.
 * \param volume Global volume of the mixer, between 0 and MIX_MAX_VOLUME.
 * \return The volume before this function was called.
 */
int Mix_MasterVolume(int volume);


#endif /* MINIRISC_AUDIO_SERVER */
