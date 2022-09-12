 /*
  *
  * Copyright (c), Valve Corporation, All rights reserved.

  * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the * * * following conditions are met:
  *
  * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  *
  * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  * Glue for creating and invoking WebRTC's AECM object using the existing Google Audio Processing component.
  *
  * Author: Ethan Geller <ethang@valvesoftware.com>
  */

#include "modules/audio-processing/aecm/echo_control_mobile.h"

#include "google_rtc_audio_processing.h"


// Creates an instance of GoogleRtcAudioProcessing with the tuning embedded in
// the library.
GoogleRtcAudioProcessingState *GoogleRtcAudioProcessingCreate(void)
{
    void* aecObject = WebRtcAecm_Create();
    WebRtcAecm_Init(aecObject, 16000);
    return aecObject;
}

// Frees all allocated resources in `state`.
void GoogleRtcAudioProcessingFree(GoogleRtcAudioProcessingState *state)
{
    WebRtcAecm_Free(state);
}

// Returns the framesize used for processing.
int GoogleRtcAudioProcessingGetFramesizeInMs(
    GoogleRtcAudioProcessingState *const state)
{
    // The AECM requires  160 frames at 16 kHz.
    return 10;
}

// Reconfigure the audio processing.
// Returns 0 if success and non zero if failure.
int GoogleRtcAudioProcessingReconfigure(
    GoogleRtcAudioProcessingState *const state, const uint8_t *const config,
    int config_size)
{
    if((size_t) config_size == sizeof(AecmConfig)) {
        AecmConfig* castConfig = (AecmConfig*) config;
        return WebRtcAecm_set_config(state, *castConfig);
    }
    else
    {
        return -1;
    }
}

// Processes the microphone stream.
// Accepts deinterleaved float audio with the range [-1, 1]. Each element of
// |src| points to an array of samples for the channel. At output, the channels
// will be in |dest|.
// Returns 0 if success and non zero if failure.
int GoogleRtcAudioProcessingProcessCapture_float32(
    GoogleRtcAudioProcessingState *const state, const float *const *src,
    float *const *dest)
{
    // Not implemented.
    return -1;
}

// Accepts and and produces a frame of interleaved 16 bit integer audio. `src`
// and `dest` may use the same memory, if desired.
// Returns 0 if success and non zero if failure.
int GoogleRtcAudioProcessingProcessCapture_int16(
    GoogleRtcAudioProcessingState *const state, const int16_t *const src,
    int16_t *const dest)
{
    return WebRtcAecm_Process(state, src, NULL, dest, 160, 0);
}

// Analyzes the playback stream.
// Accepts deinterleaved float audio with the range [-1, 1]. Each element
// of |src| points to an array of samples for the channel.
// Returns 0 if success and non zero if failure.
int GoogleRtcAudioProcessingAnalyzeRender_float32(
    GoogleRtcAudioProcessingState *const state, const float *const *src)
{
    // Not implemented.
    return -1;
}

// Accepts interleaved int16 audio.
// Returns 0 if success and non zero if failure.
int GoogleRtcAudioProcessingAnalyzeRender_int16(
    GoogleRtcAudioProcessingState *const state, const int16_t *const src)
{
    return WebRtcAecm_BufferFarend(state, src, 160);
}
