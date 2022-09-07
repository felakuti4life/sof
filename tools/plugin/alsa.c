// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2018 Intel Corporation. All rights reserved.

/* file component for reading/writing pcm samples to/from a file */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sof/sof.h>
#include <sof/list.h>
#include <sof/audio/stream.h>
#include <sof/audio/ipc-config.h>
#include <sof/lib/clk.h>
#include <sof/ipc/driver.h>
#include <sof/audio/component.h>
#include <sof/audio/format.h>
#include <sof/audio/pipeline.h>
#include <ipc/stream.h>
#include <ipc/topology.h>

#include <alsa/asoundlib.h>

#include "plugin.h"

/* bfc7488c-75aa-4ce8-9bde-d8da08a698c2 */
DECLARE_SOF_RT_UUID("arecord", arecord_uuid, 0xbfc7488c, 0x75aa, 0x4ce8,
		    0x9d, 0xbe, 0xd8, 0xda, 0x08, 0xa6, 0x98, 0xc2);
DECLARE_TR_CTX(arecord_tr, SOF_UUID(arecord_uuid), LOG_LEVEL_INFO);

/* f599ca2c-15ac-11ed-a969-5329b9cdfd2e */
DECLARE_SOF_RT_UUID("aplay", aplay_uuid, 0xf599ca2c, 0x15ac, 0x11ed,
		    0xa9, 0x69, 0x53, 0x29, 0xb9, 0xcd, 0xfd, 0x2e);
DECLARE_TR_CTX(aplay_tr, SOF_UUID(aplay_uuid), LOG_LEVEL_INFO);

static const struct comp_driver comp_arecord;
static const struct comp_driver comp_aplay;

/* ALSA comp data */
struct alsa_comp_data {
	snd_pcm_t *handle;
	snd_pcm_info_t *info;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	snd_pcm_format_t format;
	unsigned int channels;
	unsigned int rate;
	snd_pcm_uframes_t period_size;
	snd_pcm_uframes_t buffer_size;
	char *pcm_name;
};

static void alsa_free(struct comp_dev *dev)
{
	struct alsa_comp_data *cd = comp_get_drvdata(dev);

	snd_pcm_sw_params_free(cd->sw_params);
	snd_pcm_hw_params_free(cd->hw_params);
	snd_pcm_info_free(cd->info);
	snd_pcm_close(cd->handle);
	free(cd);
	free(dev);
}

static struct comp_dev *alsa_new(const struct comp_driver *drv,
				 struct comp_ipc_config *config,
				 void *spec)
{
	struct comp_dev *dev;
	struct ipc_comp_file *ipc_file = spec;
	struct alsa_comp_data *cd;
	int err;

	dev = comp_alloc(drv, sizeof(*dev));
	if (!dev)
		return NULL;
	dev->ipc_config = *config;

	/* allocate  memory for file comp data */
	cd = rzalloc(SOF_MEM_ZONE_RUNTIME_SHARED, 0, SOF_MEM_CAPS_RAM, sizeof(*cd));
	if (!cd)
		goto error;

	comp_set_drvdata(dev, cd);

	err = snd_pcm_info_malloc(&cd->info);
	if (err < 0)
		goto error;

	err = snd_pcm_hw_params_malloc(&cd->hw_params);
	if (err < 0)
		goto error;

	err = snd_pcm_sw_params_malloc(&cd->sw_params);
	if (err < 0)
		goto error;

	//dev->direction = ipc_file->direction;
	cd->pcm_name = "default";

	return dev;

error:
	free(dev);
	return NULL;
}

static struct comp_dev *arecord_new(const struct comp_driver *drv,
				 struct comp_ipc_config *config,
				 void *spec)
{
	struct comp_dev *dev;
	struct alsa_comp_data *cd;
	int err;

	dev = alsa_new(drv, config, spec);
	if (!dev)
		return NULL;

	err = snd_pcm_open(&cd->handle, cd->pcm_name, SND_PCM_STREAM_CAPTURE, 0);
	if (err < 0) {
		fprintf(stderr, "error: cant open PCM: %s\n", snd_strerror(err));
		return NULL;
	}

	if ((err = snd_pcm_info(cd->handle, cd->info)) < 0) {
		fprintf(stderr, "error: cant get PCM info: %s\n", snd_strerror(err));
		return NULL;
	}

	dev->state = COMP_STATE_READY;
	return dev;
}

static struct comp_dev *aplay_new(const struct comp_driver *drv,
				 struct comp_ipc_config *config,
				 void *spec)
{
	struct comp_dev *dev;
	struct alsa_comp_data *cd;
	int err;

	dev = alsa_new(drv, config, spec);
	if (!dev)
		return NULL;

	err = snd_pcm_open(&cd->handle, cd->pcm_name, SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		fprintf(stderr, "error: cant open PCM: %s\n", snd_strerror(err));
		return NULL;
	}

	if ((err = snd_pcm_info(cd->handle, cd->info)) < 0) {
		fprintf(stderr, "error: cant get PCM info: %s\n", snd_strerror(err));
		return NULL;
	}

	dev->state = COMP_STATE_READY;
	return dev;
}

static int set_params(struct alsa_comp_data *cd)
{
	int err;

	/* is sound card HW configuration valid ? */
	err = snd_pcm_hw_params_any(cd->handle, cd->hw_params);
	if (err < 0) {
		fprintf(stderr, "error: cant get PCM hw_params: %s\n", snd_strerror(err));
		return err;
	}

	/* set interleaved buffer format */
	err = snd_pcm_hw_params_set_access(cd->handle, cd->hw_params,
					   SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		fprintf(stderr, "error: PCM can't set interleaved: %s\n", snd_strerror(err));
		return err;
	}

	/* set sample format */
	err = snd_pcm_hw_params_set_format(cd->handle, cd->hw_params, cd->format);
	if (err < 0) {
		fprintf(stderr, "error: PCM can't set format: %s\n", snd_strerror(err));
		return err;
	}

	/* set number of channels */
	err = snd_pcm_hw_params_set_channels(cd->handle, cd->hw_params, cd->channels);
	if (err < 0) {
		fprintf(stderr, "error: PCM can't set channels: %s\n", snd_strerror(err));
		return err;
	}

	/* set sample rate */
	err = snd_pcm_hw_params_set_rate(cd->handle, cd->hw_params, cd->rate, 0);
	if (err < 0) {
		fprintf(stderr, "error: PCM can't set rate: %s\n", snd_strerror(err));
		return err;
	}

	/* commit the hw params */
	err = snd_pcm_hw_params(cd->handle, cd->hw_params);
	if (err < 0) {
		fprintf(stderr, "error: PCM can't commit hw_params: %s\n", snd_strerror(err));
		snd_pcm_hw_params_dump(cd->hw_params, SND_OUTPUT_STDIO);
		return err;
	}

	/* get period and buffer size - TODO: back propagate through pipeline */
	snd_pcm_hw_params_get_period_size(cd->hw_params, &cd->period_size, 0);
	snd_pcm_hw_params_get_buffer_size(cd->hw_params, &cd->buffer_size);

	/* get the initial SW params */
	err = snd_pcm_sw_params_current(cd->handle, cd->sw_params);
	if (err < 0) {
		fprintf(stderr, "error: PCM can't get sw params: %s\n", snd_strerror(err));
		return err;
	}

	/* set the avail min to the period size */
	err = snd_pcm_sw_params_set_avail_min(cd->handle, cd->sw_params, cd->period_size);
	if (err < 0) {
		fprintf(stderr, "error: PCM can't set avail min: %s\n", snd_strerror(err));
		return err;
	}

	/* PCM should start after receiving first periods worth of data */
	err = snd_pcm_sw_params_set_start_threshold(cd->handle, cd->sw_params, cd->period_size);
	if (err < 0) {
		fprintf(stderr, "error: PCM can't set start threshold: %s\n", snd_strerror(err));
		return err;
	}

	/* PCM should stop if only 1/4 period worth of data is available */
	err = snd_pcm_sw_params_set_stop_threshold(cd->handle, cd->sw_params, cd->period_size / 4);
	if (err < 0) {
		fprintf(stderr, "error: PCM can't set stop threshold: %s\n", snd_strerror(err));
		return err;
	}

	/* commit the sw params */
	if (snd_pcm_sw_params(cd->handle, cd->sw_params) < 0) {
		fprintf(stderr, "error: PCM can't commit sw_params: %s\n", snd_strerror(err));
		snd_pcm_sw_params_dump(cd->sw_params, SND_OUTPUT_STDIO);
		return err;
	}

	return 0;
}


/**
 * \brief Sets file component audio stream parameters.
 * \param[in,out] dev Volume base component device.
 * \param[in] params Audio (PCM) stream parameters (ignored for this component)
 * \return Error code.
 *
 * All done in prepare() since we need to know source and sink component params.
 */
static int arecord_params(struct comp_dev *dev,
		       struct sof_ipc_stream_params *params)
{
	struct comp_buffer *buffer;
	struct dai_data *dd = comp_get_drvdata(dev);
	struct alsa_comp_data *cd = comp_get_drvdata(dd->dai);
	struct audio_stream *stream;
	int periods;
	int samples;
	int ret;

	ret = comp_verify_params(dev, 0, params);
	if (ret < 0) {
		comp_err(dev, "alsa_params(): pcm params verification failed.");
		return ret;
	}

	/* file component sink/source buffer period count */
	buffer = list_first_item(&dev->bsink_list, struct comp_buffer, source_list);
	periods = dev->ipc_config.periods_sink;


	/* set downstream buffer size */
	stream = &buffer->stream;
	buffer_reset_pos(buffer, NULL);

	ret = set_params(cd);

	return 0;
}

static int aplay_params(struct comp_dev *dev,
		       struct sof_ipc_stream_params *params)
{
	struct comp_buffer *buffer;
	struct dai_data *dd = comp_get_drvdata(dev);
	struct alsa_comp_data *cd = comp_get_drvdata(dd->dai);
	struct audio_stream *stream;
	int periods;
	int samples;
	int ret;

	ret = comp_verify_params(dev, 0, params);
	if (ret < 0) {
		comp_err(dev, "alsa_params(): pcm params verification failed.");
		return ret;
	}

	/* file component sink/source buffer period count */
	buffer = list_first_item(&dev->bsource_list, struct comp_buffer, sink_list);
	periods = dev->ipc_config.periods_source;

	/* set downstream buffer size */
	stream = &buffer->stream;
	buffer_reset_pos(buffer, NULL);

	ret = set_params(cd);

	return 0;
}

static int alsa_trigger(struct comp_dev *dev, int cmd)
{
	comp_info(dev, "alsa_trigger()");
	return comp_set_state(dev, cmd);
}

/* used to pass standard and bespoke commands (with data) to component */
static int alsa_cmd(struct comp_dev *dev, int cmd, void *data,
		    int max_data_size)
{
	struct sof_ipc_ctrl_data *cdata = ASSUME_ALIGNED(data, 4);
	int ret = 0;

	switch (cmd) {
	case COMP_CMD_SET_DATA:
		fprintf(stderr, "Warning: not implemented %d\n", cmd);
		break;
	default:
		fprintf(stderr, "Warning: Unknown file command %d\n", cmd);
		return -EINVAL;
	}

	return ret;
}

/*
 * copy and process stream samples
 * returns the number of bytes copied
 */
static int arecord_copy(struct comp_dev *dev)
{
	struct alsa_comp_data *cd = comp_get_drvdata(dev);
	struct comp_buffer *buffer;
	struct audio_stream *sink;
	snd_pcm_sframes_t frames;
	snd_pcm_uframes_t remain;
	snd_pcm_uframes_t total = 0;
	unsigned frame_bytes;
	void *pos;

	/* file component sink buffer */
	buffer = list_first_item(&dev->bsink_list, struct comp_buffer,
				 source_list);
	sink = &buffer->stream;
	pos = sink->w_ptr;
	remain = audio_stream_get_free_frames(sink);
	frame_bytes = audio_stream_frame_bytes(sink);

	while (remain) {
		frames = audio_stream_frames_without_wrap(sink, pos);

		/* read PCM samples from file */
		frames = snd_pcm_readi(cd->handle, pos, frames);
		if (frames < 0) {
			fprintf(stderr, "failed to read: %s: %s\n",
				cd->pcm_name, snd_strerror(frames));
			return frames;
		}

		remain -= frames;
		pos = audio_stream_wrap(sink, pos + frames * frame_bytes);
		total += frames;
	}

	/* update sink buffer pointers */
	comp_update_buffer_produce(buffer, total * frame_bytes);

	return total * frame_bytes;
}

/*
 * copy and process stream samples
 * returns the number of bytes copied
 */
static int aplay_copy(struct comp_dev *dev)
{
	struct alsa_comp_data *cd = comp_get_drvdata(dev);
	struct comp_buffer *buffer;
	struct audio_stream *source;
	snd_pcm_sframes_t frames;
	snd_pcm_uframes_t remain;
	snd_pcm_uframes_t total = 0;
	unsigned frame_bytes;
	void *pos;

	/* file component source buffer */
	buffer = list_first_item(&dev->bsource_list, struct comp_buffer,
				 sink_list);
	source = &buffer->stream;
	pos = source->r_ptr;
	remain = audio_stream_get_free_frames(source);
	frame_bytes = audio_stream_frame_bytes(source);

	while (remain) {
		frames = audio_stream_bytes_without_wrap(source, pos);

		/* read PCM samples from file */
		frames = snd_pcm_writei(cd->handle, pos, frames);
		if (frames < 0) {
			fprintf(stderr, "failed to write: %s: %s\n",
				cd->pcm_name, snd_strerror(frames));
			return frames;
		}

		remain -= frames;
		pos = audio_stream_wrap(source, pos + frames * frame_bytes);
		total += frames;
	}

	/* update sink buffer pointers */
	comp_update_buffer_consume(buffer, total * frame_bytes);

	return total * frame_bytes;
}

static int alsa_prepare(struct comp_dev *dev)
{
	int ret = 0;

	ret = comp_set_state(dev, COMP_TRIGGER_PREPARE);
	if (ret < 0)
		return ret;

	if (ret == COMP_STATUS_STATE_ALREADY_SET)
		return PPL_STATUS_PATH_STOP;

	dev->state = COMP_STATE_PREPARE;
	return ret;
}

static int alsa_reset(struct comp_dev *dev)
{
	comp_set_state(dev, COMP_TRIGGER_RESET);
	return 0;
}

static const struct comp_driver comp_arecord = {
	.type = SOF_COMP_FILEREAD,
	.uid = SOF_RT_UUID(arecord_uuid),
	.tctx	= &arecord_tr,
	.ops = {
		.create = arecord_new,
		.free = alsa_free,
		.params = arecord_params,
		.cmd = alsa_cmd,
		.trigger = alsa_trigger,
		.copy = arecord_copy,
		.prepare = alsa_prepare,
		.reset = alsa_reset,
	},
};

static const struct comp_driver comp_aplay = {
	.type = SOF_COMP_FILEWRITE,
	.uid = SOF_RT_UUID(aplay_uuid),
	.tctx	= &aplay_tr,
	.ops = {
		.create = aplay_new,
		.free = alsa_free,
		.params = aplay_params,
		.cmd = alsa_cmd,
		.trigger = alsa_trigger,
		.copy = aplay_copy,
		.prepare = alsa_prepare,
		.reset = alsa_reset,
	},
};

static struct comp_driver_info comp_arecord_info = {
	.drv = &comp_arecord,
};

static struct comp_driver_info comp_aplay_info = {
	.drv = &comp_aplay,
};

static void sys_comp_alsa_init(void)
{
	comp_register(&comp_arecord_info);
	comp_register(&comp_aplay_info);
}

DECLARE_MODULE(sys_comp_alsa_init);
