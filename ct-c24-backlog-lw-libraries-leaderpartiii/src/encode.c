#include "../include/simpleArray.h"
#include "../return_codes.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum
{
	single,
	non_planar_first,
	non_planar_second,
	planar_first,
	planar_second,
} sample_types;

static double getSample(const enum AVSampleFormat format, const uint8_t *buffer, int32_t sampleIndex)
{
	int64_t val = 0;
	double ret;
	int32_t sampleSize = av_get_bytes_per_sample(format);
	switch (sampleSize)
	{
	case 1:
		val = ((uint8_t *)buffer)[sampleIndex];
		val -= 127;
		break;
	case 2:
		val = ((int16_t *)buffer)[sampleIndex];
		break;
	case 4:
		val = ((int32_t *)buffer)[sampleIndex];
		break;
	case 8:
		val = ((int64_t *)buffer)[sampleIndex];
		break;
	default:
		fprintf(stderr, "Invalid sample size %d.\n", sampleSize);
		return INFINITY;
	}
	switch (format)
	{
	case AV_SAMPLE_FMT_S16P:
	case AV_SAMPLE_FMT_S32P:
		ret = (float)val / ((double)((1 << (sampleSize * 8 - 1)) - 1));
		break;
	case AV_SAMPLE_FMT_FLT:
	case AV_SAMPLE_FMT_FLTP:
		ret = *((float *)&val);
		break;
	case AV_SAMPLE_FMT_DBL:
	case AV_SAMPLE_FMT_DBLP:
		ret = (double)(*((double *)&val));
		break;
	default:
		fprintf(stderr, "Invalid sample format %s.\n", av_get_sample_fmt_name(format));
		return INFINITY;
	}
	return ret;
}

static int32_t fill_array(Array *res, sample_types typeof_samples, AVFrame *frame)
{
	int32_t code_error;
	int32_t delta = ((typeof_samples == non_planar_first || typeof_samples == non_planar_second) ? 2 : 1);
	for (int32_t i = (typeof_samples == non_planar_second); i < frame->nb_samples; i += delta)
	{
		double temp = getSample(frame->format, frame->extended_data[typeof_samples == planar_second], i);
		if (temp == INFINITY)
		{
			fprintf(stderr, "Invalid extended data in frame\n");
			free_array_with_size(res);
			return ERROR_DATA_INVALID;
		}
		if ((code_error = add(res, temp)))
		{
			return code_error;
		}
	}
	return SUCCESS;
}

static int32_t averror_to_local(int32_t code_error)
{
	switch (code_error)
	{
	case AVERROR_UNKNOWN:
		return ERROR_UNKNOWN;
	case AVERROR_DECODER_NOT_FOUND | AVERROR_DEMUXER_NOT_FOUND | AVERROR_ENCODER_NOT_FOUND | AVERROR_FILTER_NOT_FOUND |
		AVERROR_BSF_NOT_FOUND | AVERROR_MUXER_NOT_FOUND | AVERROR_OPTION_NOT_FOUND | AVERROR_PROTOCOL_NOT_FOUND | AVERROR_STREAM_NOT_FOUND:
		return ERROR_FORMAT_INVALID;
	case AVERROR_INVALIDDATA:
		return ERROR_DATA_INVALID;
	case AVERROR_BUFFER_TOO_SMALL:
		return ERROR_NOTENOUGH_MEMORY;
	default:
		return ERROR_UNSUPPORTED;
	}
}

int32_t get_samples(char filename[], int32_t argc, _Bool is_first, int32_t *sample_rate, Array *res)
{
	av_log_set_level(AV_LOG_QUIET);
	AVFormatContext *fmt_ctx = NULL;
	AVCodecParameters *codec_params = NULL;
	const AVCodec *codec = NULL;
	AVCodecContext *codec_ctx = NULL;
	AVPacket *packet = NULL;
	AVFrame *frame = NULL;

	int32_t code_error = SUCCESS;

	fmt_ctx = avformat_alloc_context();

	if (fmt_ctx == NULL)
	{
		fprintf(stderr, "Could not allocate format context\n");
		code_error = ERROR_NOTENOUGH_MEMORY;
		goto free;
	}

	if ((code_error = avformat_open_input(&fmt_ctx, filename, NULL, NULL)) < 0)
	{
		fprintf(stderr, "Could not open audio file\n");
		code_error = averror_to_local(code_error);
		goto free;
	}

	if ((code_error = avformat_find_stream_info(fmt_ctx, NULL)) < 0)
	{
		fprintf(stderr, "Could not find stream information\n");
		code_error = averror_to_local(code_error);
		goto free;
	}
	int32_t audio_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

	if (audio_stream_index < 0)
	{
		fprintf(stderr, "Could not find audio stream in the file\n");
		code_error = averror_to_local(audio_stream_index);
		goto free;
	}

	if ((codec_params = fmt_ctx->streams[audio_stream_index]->codecpar) == NULL)
	{
		fprintf(stderr, "Could not find decoder\n");
		code_error = ERROR_DATA_INVALID;
		goto free;
	}

	if ((codec = avcodec_find_decoder(codec_params->codec_id)) == NULL)
	{
		fprintf(stderr, "Could not find codec\n");
		code_error = ERROR_DATA_INVALID;
		goto free;
	}

	if ((codec_ctx = avcodec_alloc_context3(codec)) == NULL)
	{
		fprintf(stderr, "Could not allocate decoder context\n");
		code_error = ERROR_NOTENOUGH_MEMORY;
		goto free;
	}

	if ((code_error = avcodec_parameters_to_context(codec_ctx, codec_params)) < 0)
	{
		fprintf(stderr, "Failed to copy codec parameters to decoder context\n");
		code_error = averror_to_local(code_error);
		goto free;
	}

	if ((code_error = avcodec_open2(codec_ctx, codec, NULL)) < 0)
	{
		fprintf(stderr, "Could not open codec\n");
		code_error = averror_to_local(code_error);
		goto free;
	}

	if ((packet = av_packet_alloc()) == NULL)
	{
		fprintf(stderr, "Could not allocate packet\n");
		code_error = ERROR_NOTENOUGH_MEMORY;
		goto free;
	}

	if ((frame = av_frame_alloc()) == NULL)
	{
		fprintf(stderr, "Could not allocate frame\n");
		code_error = ERROR_NOTENOUGH_MEMORY;
		goto free;
	}

	if (codec->supported_samplerates == NULL)
	{
		*sample_rate = 44100;
	}
	else
	{
		*sample_rate = codec_ctx->sample_rate;
	}

	int32_t code_error_temp;
	_Bool is_planar = av_sample_fmt_is_planar(codec_ctx->sample_fmt);	 // is_planar ? yes : no it's packed

	while (av_read_frame(fmt_ctx, packet) == 0)
	{
		if (packet->stream_index != audio_stream_index)
		{
			continue;
		}
		code_error = avcodec_send_packet(codec_ctx, packet);
		code_error_temp = avcodec_receive_frame(codec_ctx, frame);
		if (code_error == AVERROR(EAGAIN) || code_error_temp == AVERROR(EAGAIN))
		{
			continue;
		}
		else if (code_error == AVERROR(EOF) || code_error_temp == AVERROR(EOF))
		{
			break;
		}
		else if (code_error == AVERROR(EINVAL) || code_error_temp == AVERROR(EINVAL))
		{
			fprintf(stderr, "Codec not opened in the packet\n");
			code_error = ERROR_DATA_INVALID;
			goto free;
		}
		else if (code_error != 0 || code_error_temp != 0)
		{
			fprintf(stderr, "Failed in read packet\n");
			code_error = averror_to_local(code_error);
			goto free;
		}

		if (argc != 3 && ((is_planar && frame->ch_layout.nb_channels != 2) || (!is_planar && frame->ch_layout.nb_channels != 1)))
		{
			fprintf(stderr,
					"Incorrect audio file expected channels %d but found %d\n",
					(av_sample_fmt_is_planar(codec_ctx->sample_fmt) ? 2 : 1),
					frame->ch_layout.nb_channels);
			code_error = ERROR_FORMAT_INVALID;
			goto free;
		}

		sample_types common =
			(argc == 3 ? single : (is_first ? (is_planar ? planar_first : non_planar_first) : (is_planar ? planar_second : non_planar_second)));
		if ((code_error = fill_array(res, common, frame)))
		{
			goto free;
		}

		av_packet_unref(packet);
	}

free:
	if (fmt_ctx)
		avformat_close_input(&fmt_ctx);
	if (codec_ctx)
		avcodec_free_context(&codec_ctx);
	if (frame)
		av_frame_free(&frame);
	if (packet)
		av_packet_free(&packet);

	return code_error;
}
