
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <assert.h> 
#include "wav_parser.h" 	//封装解析wav文件


#define DEFAULT_CHANNELS		 (1)//(2) 
#define DEFAULT_SAMPLE_RATE 	 (16000) 
#define DEFAULT_SAMPLE_LENGTH	 (16) 
#define DEFAULT_DURATION_TIME	 (10) 

typedef long long off64_t; 



typedef struct SNDPCMCONTROL  { 
	 snd_pcm_t *handle; 
	 snd_output_t *log; 
	 snd_pcm_uframes_t chunk_size; 
	 snd_pcm_uframes_t buffer_size; 
	 snd_pcm_format_t format; 
	 uint16_t channels; 
	 size_t chunk_bytes; 
	 size_t bits_per_sample; 
	 size_t bits_per_frame; 
  
	 uint8_t *data_buf; 
 } SNDControl_t; 





//配置参数
int WAV_Configure_Params(WAVContainer_t *wav)
{ 
	assert(wav); 
	uint16_t channels 	   = DEFAULT_CHANNELS; 
	uint16_t sample_rate   = DEFAULT_SAMPLE_RATE; 
	uint16_t sample_length = DEFAULT_SAMPLE_LENGTH; 
	uint32_t duration_time = DEFAULT_DURATION_TIME; 

	wav->header.magic 	 = WAV_RIFF; 
	wav->header.type 	 = WAV_WAVE; 
	wav->format.magic 	 = WAV_FMT; 
	wav->format.fmt_size = LE_INT(16); 
	wav->format.format 	 = LE_SHORT(WAV_FMT_PCM); 
	wav->chunk.type 	 = WAV_DATA; 

	//自定义 
	wav->format.channels 	  =	LE_SHORT(channels); 
	wav->format.sample_rate   = LE_INT(sample_rate); 
	wav->format.sample_length = LE_SHORT(sample_length); 

	wav->format.blocks_align   = LE_SHORT(channels * sample_length / 8); 
	wav->format.bytes_p_second = LE_INT((uint16_t)(wav->format.blocks_align) * sample_rate); 

	wav->chunk.length  = LE_INT(duration_time * (uint32_t)(wav->format.bytes_p_second)); 
	wav->header.length = LE_INT((uint32_t)(wav->chunk.length) + sizeof(wav->chunk) + sizeof(wav->format) + sizeof(wav->header) - 8); 
	return 0; 
}

//设置硬件的参数
int WAV_Set_Params(SNDControl_t *sndpcm, WAVContainer_t *wav) 
{ 
	snd_pcm_hw_params_t *hwparams; 
	snd_pcm_format_t format; 
	uint32_t exact_rate; 
	uint32_t buffer_time, period_time; 

	/* 在堆栈上分配snd_pcm_hw_params_t结构 */ 
	snd_pcm_hw_params_alloca(&hwparams); 
	
	/* 初始化hwparams全配置空间 */ 
	if (snd_pcm_hw_params_any(sndpcm->handle, hwparams) < 0) { 
		fprintf(stderr, "Error snd_pcm_hw_params_any/n"); 
		goto ERR_SET_PARAMS; 
	} 
 
	if (snd_pcm_hw_params_set_access(sndpcm->handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) { 
		fprintf(stderr, "Error snd_pcm_hw_params_set_access/n"); 
		goto ERR_SET_PARAMS; 
	} 
 
	/* 设置样品格式 */ 
	/*if (SNDWAV_P_GetFormat(wav, &format) < 0) { 
		fprintf(stderr, "Error get_snd_pcm_format/n"); 
		goto ERR_SET_PARAMS; 
	} */
	
	format = SND_PCM_FORMAT_S16_LE;
	if (snd_pcm_hw_params_set_format(sndpcm->handle, hwparams, format) < 0) { 
		fprintf(stderr, "Error snd_pcm_hw_params_set_format/n"); 
		goto ERR_SET_PARAMS; 
	} 
	sndpcm->format = format; 
 
	/* 设置声道数 */ 
	if (snd_pcm_hw_params_set_channels(sndpcm->handle, hwparams, LE_SHORT(wav->format.channels)) < 0) { 
		fprintf(stderr, "Error snd_pcm_hw_params_set_channels/n"); 
		goto ERR_SET_PARAMS; 
	} 
	sndpcm->channels = LE_SHORT(wav->format.channels); 
 
	/* 设定采样率。如果不支持，设置最接近制定的采样率*/  
	exact_rate = LE_INT(wav->format.sample_rate); 

 	printf("Line(%d) exact_rate = %d\n",__LINE__,exact_rate);	
	if (snd_pcm_hw_params_set_rate_near(sndpcm->handle, hwparams, &exact_rate, 0) < 0) { 
		fprintf(stderr, "Error snd_pcm_hw_params_set_rate_near/n"); 
		goto ERR_SET_PARAMS; 
	} 
	if (LE_INT(wav->format.sample_rate) != exact_rate) { 
		fprintf(stderr, "The rate %d Hz is not supported by your hardware./n ==> Using %d Hz instead./n", LE_INT(wav->format.sample_rate), exact_rate); 
	} 
 
	if (snd_pcm_hw_params_get_buffer_time_max(hwparams, &buffer_time, 0) < 0) { 
		fprintf(stderr, "Error snd_pcm_hw_params_get_buffer_time_max/n"); 
		goto ERR_SET_PARAMS; 
	} 
	if (buffer_time > 500000) buffer_time = 500000; period_time = buffer_time / 4; 
 
	if (snd_pcm_hw_params_set_buffer_time_near(sndpcm->handle, hwparams
, &buffer_time, 0) < 0) { 
		fprintf(stderr, "Error snd_pcm_hw_params_set_buffer_time_near/n"); 
		goto ERR_SET_PARAMS; 
	} 
 
	if (snd_pcm_hw_params_set_period_time_near(sndpcm->handle, hwparams, &period_time, 0) < 0) { 
		fprintf(stderr, "Error snd_pcm_hw_params_set_period_time_near/n"); 
		goto ERR_SET_PARAMS; 
	} 
	/* 整理设置好的参数 */ 
	if (snd_pcm_hw_params(sndpcm->handle, hwparams) < 0) { 
		fprintf(stderr, "Error snd_pcm_hw_params(handle, params)/n"); 
		goto ERR_SET_PARAMS; 
	} 
	//获取周期大小
	snd_pcm_hw_params_get_period_size(hwparams, &sndpcm->chunk_size, 0);	 
	//获取足够大的数组，来储存数据块
	snd_pcm_hw_params_get_buffer_size(hwparams, &sndpcm->buffer_size); 
	if (sndpcm->chunk_size == sndpcm->buffer_size) {		 
		fprintf(stderr, ("Can't use period equal to buffer size (%lu == %lu)/n"), sndpcm->chunk_size, sndpcm->buffer_size); 	 
		goto ERR_SET_PARAMS; 
	} 
	sndpcm->bits_per_sample = snd_pcm_format_physical_width(format); 
	sndpcm->bits_per_frame = sndpcm->bits_per_sample * LE_SHORT(wav->format.channels);  
	sndpcm->chunk_bytes = sndpcm->chunk_size * sndpcm->bits_per_frame / 8; 
	/* 分配音频数据缓冲区 */ 
	sndpcm->data_buf = (uint8_t *)malloc(sndpcm->chunk_bytes); 
	if (!sndpcm->data_buf) { 
		fprintf(stderr, "Error malloc: [data_buf]/n"); 
		goto ERR_SET_PARAMS; 
	} 
 
	return 0; 
 
ERR_SET_PARAMS: 
	return -1; 
} 


//读硬件的配置参数（录音） 
ssize_t WAV_ReadPcm(SNDControl_t *sndpcm, size_t rcount) 
{ 
	ssize_t r; 
	size_t result = 0; 
	size_t count = rcount; 
	uint8_t *data = sndpcm->data_buf; 
 
	if (count != sndpcm->chunk_size) { 
		count = sndpcm->chunk_size; 
	} 
 
	while (count > 0) { 
		r = snd_pcm_readi(sndpcm->handle, data, count); 
		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) { 
			snd_pcm_wait(sndpcm->handle, 1000); 
		} else if (r == -EPIPE) { 
			snd_pcm_prepare(sndpcm->handle); 
			fprintf(stderr, "<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>/n"); 
		} else if (r == -ESTRPIPE) { 
			fprintf(stderr, "<<<<<<<<<<<<<<< Need suspend >>>>>>>>>>>>>>>/n"); 
		} else if (r < 0) { 
			fprintf(stderr, "Error snd_pcm_writei: [%s]", snd_strerror(r)); 
			exit(-1); 
		} 	 
		if (r > 0) { 
			result += r; 
			count -= r; 
			data += r * sndpcm->bits_per_frame / 8; 
		} 
	} 
	return rcount; 
} 

void WAV_Record(SNDControl_t *sndpcm, WAVContainer_t *wav, int fd) 
{ 
	//off64_t rest; 
	uint32_t rest;
	size_t c, frame_size; 
	
	//写文件头
	if (WAV_WriteHeader(fd, wav) < 0) { 
		exit(-1); 
	} 
	
	rest = wav->chunk.length; 
	printf("Line(%d) rest = %d\n",__LINE__,rest);
	
	while (rest > 0) { 
		c = (rest <= (off64_t)sndpcm->chunk_bytes) ? (size_t)rest : sndpcm->chunk_bytes; 
		printf("Line(%d) c = %d\n",__LINE__,c);
		frame_size = c * 8 / sndpcm->bits_per_frame; 
		if (WAV_ReadPcm(sndpcm, frame_size) != frame_size) 
			break; 
		 
		if (write(fd, sndpcm->data_buf, c) != c) { 
			fprintf(stderr, "Error SNDWAV_Record[write]/n"); 
			exit(-1); 
		} 
 
		rest -= c; 
		printf("Line(%d) rest = %d\n",__LINE__,rest);
	} 
}



int main(int argc, char *argv[]) 
{ 
	char *filename; 
	char *devicename = "default"; 
	int fd; 
	WAVContainer_t wav; 
	SNDControl_t record;  
	if (argc != 2) { 
		fprintf(stderr, "Usage: ./lrecord <file name>/n"); 
		return -1; 
	}  
	memset(&record, 0, sizeof(record)); 
	filename = argv[1]; 
	remove(filename); 
	if ((fd = open(filename, O_WRONLY | O_CREAT, 0644)) == -1) { 
		fprintf(stderr, "Error open: [%s]/n", filename); 
		return -1; 
	} 
	
 	/*
	if (snd_output_stdio_attach(&record.log, stderr, 0) < 0) { 
		fprintf(stderr, "Error snd_output_stdio_attach/n"); 
		goto Err; 
	} */
 
	if (snd_pcm_open(&record.handle, devicename, SND_PCM_STREAM_CAPTURE, 0) < 0) { 
		fprintf(stderr, "Error snd_pcm_open [ %s]/n", devicename); 
		goto Err; 
	} 
	//配置参数
	if (WAV_Configure_Params(&wav) < 0) { 
		fprintf(stderr, "Error WAV_Configure_Para/n"); 
		goto Err; 
	} 
 
	if (WAV_Set_Params(&record, &wav) < 0) { 
		fprintf(stderr, "Error set_snd_pcm_params/n"); 
		goto Err; 
	} 
	//snd_pcm_dump(record.handle, record.log); 
 	printf("Line(%d) -- here --\n",__LINE__);
	WAV_Record(&record, &wav, fd); 
 	printf("Line(%d) -- here --\n",__LINE__);
	snd_pcm_drain(record.handle); 
 	printf("Line(%d) -- here --\n",__LINE__);
	
	close(fd); 
	printf("Line(%d) -- here --\n",__LINE__);
	free(record.data_buf); 
	printf("Line(%d) -- here --\n",__LINE__);
	//snd_output_close(record.log); 
	printf("Line(%d) -- here --\n",__LINE__);
	snd_pcm_close(record.handle); 
	printf("Line(%d) -- here --\n",__LINE__);
	return 0; 
 
Err: 
	close(fd); 
	remove(filename); 
	if (record.data_buf) free(record.data_buf); 
	if (record.log) snd_output_close(record.log); 
	if (record.handle) snd_pcm_close(record.handle); 
	return -1; 
} 
