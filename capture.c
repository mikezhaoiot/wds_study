
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <assert.h> 
#include "wav_parser.h" 	//封装解析wav文件


#define DEFAULT_CHANNELS		 (1)//(2) 
#define DEFAULT_SAMPLE_RATE 	 (16000) 
#define DEFAULT_SAMPLE_LENGTH	 (16) 
#define DEFAULT_DURATION_TIME	 (30) 

typedef long long off64_t; 



typedef struct SNDPCMCONTROL  { 
	 snd_pcm_t *handle; 
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
	uint16_t channels 	   = DEFAULT_CHANNELS; 
	uint16_t sample_rate   = DEFAULT_SAMPLE_RATE; 
	uint16_t sample_length = DEFAULT_SAMPLE_LENGTH; 
	/* 总时间 */
	uint32_t duration_time = DEFAULT_DURATION_TIME; 

	wav->header.magic 	 = WAV_RIFF; 
	wav->header.type 	 = WAV_WAVE; 
	wav->format.magic 	 = WAV_FMT; 
	wav->format.fmt_size = LE_INT(16); 
	wav->format.format 	 = LE_SHORT(WAV_FMT_PCM); 
	wav->chunk.type 	 = WAV_DATA; 

	//自定义wav数据
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
	
	snd_pcm_hw_params_alloca(&hwparams); 
	
	if (snd_pcm_hw_params_any(sndpcm->handle, hwparams) < 0) { 
		printf("Line(%d) error \n",__LINE__);
		return -1; 
	} 
 
	if (snd_pcm_hw_params_set_access(sndpcm->handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) { 
		printf("Line(%d) error \n",__LINE__);
		return -1; 
	} 
 
	format = SND_PCM_FORMAT_S16_LE;
	if (snd_pcm_hw_params_set_format(sndpcm->handle, hwparams, format) < 0) { 
		printf("Line(%d) error \n",__LINE__);
		return -1; 
	} 
	sndpcm->format = format; 
 
	/* 设置声道数 */ 
	if (snd_pcm_hw_params_set_channels(sndpcm->handle, hwparams, LE_SHORT(wav->format.channels)) < 0) { 
		printf("Line(%d) error \n",__LINE__);
		return -1; 
	} 
	sndpcm->channels = LE_SHORT(wav->format.channels); 
 
	exact_rate = LE_INT(wav->format.sample_rate); 
 	printf("Line(%d) exact_rate = %d\n",__LINE__,exact_rate);
	/*设置采样率 16k*/
	if (snd_pcm_hw_params_set_rate_near(sndpcm->handle, hwparams, &exact_rate, 0) < 0) { 
		printf("Line(%d) error \n",__LINE__);
		return -1; 
	} 

	if (snd_pcm_hw_params_get_buffer_time_max(hwparams, &buffer_time, 0) < 0) { 
		printf("Line(%d) error \n",__LINE__);
		return -1; 
	} 
	if (buffer_time > 500000) buffer_time = 500000; period_time = buffer_time / 4; 
 
	if (snd_pcm_hw_params_set_buffer_time_near(sndpcm->handle, hwparams, &buffer_time, 0) < 0) { 
		printf("Line(%d) error \n",__LINE__);
		return -1; 
	} 
 
	if (snd_pcm_hw_params_set_period_time_near(sndpcm->handle, hwparams, &period_time, 0) < 0) { 
		printf("Line(%d) error \n",__LINE__);
		return -1; 
	} 
	/* 整理设置好的参数 */ 
	if (snd_pcm_hw_params(sndpcm->handle, hwparams) < 0) { 
		printf("Line(%d) error \n",__LINE__);
		return -1; 
	}
	/* 获取周期大小 */
	snd_pcm_hw_params_get_period_size(hwparams, &sndpcm->chunk_size, 0);	 
	snd_pcm_hw_params_get_buffer_size(hwparams, &sndpcm->buffer_size); 
	if (sndpcm->chunk_size == sndpcm->buffer_size) {		 
		printf("Line(%d) error \n",__LINE__);
		return -1; 
	} 
	sndpcm->bits_per_sample = snd_pcm_format_physical_width(format); 
	sndpcm->bits_per_frame = sndpcm->bits_per_sample * LE_SHORT(wav->format.channels);  
	sndpcm->chunk_bytes = sndpcm->chunk_size * sndpcm->bits_per_frame / 8; 
	
	/*分配缓冲区*/
	sndpcm->data_buf = (uint8_t *)malloc(sndpcm->chunk_bytes); 
	if (!sndpcm->data_buf) { 
		printf("Line(%d) error \n",__LINE__);
		return -1; 
	} 
	return 0; 

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
		printf("Line(%d) r = %d\n",__LINE__,r);

		/*
		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) { 
			snd_pcm_wait(sndpcm->handle, 1000); 
		} 
		else if (r == -EPIPE) { 
			snd_pcm_prepare(sndpcm->handle); 
			printf("Line(%d) r = %d\n",__LINE__,r);
		} 
		else if (r == -ESTRPIPE) { 
			printf("Line(%d) r = %d\n",__LINE__,r);
		} 
		else if (r < 0) { 
			printf("Line(%d) r = %d\n",__LINE__,r);
			exit(-1); 
		} 
		*/
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
	
	//写Wav文件头
	if (WAV_WriteHeader(fd, wav) < 0) { 
		exit(-1); 
	} 
	
	rest = wav->chunk.length; 
	printf("Line(%d) rest = %d\n",__LINE__,rest);
	
	while (rest > 0) { 
		c = (rest <= sndpcm->chunk_bytes) ? (size_t)rest : sndpcm->chunk_bytes; 
		printf("Line(%d) c = %d\n",__LINE__,c);
		frame_size = c * 8 / sndpcm->bits_per_frame; 
		if (WAV_ReadPcm(sndpcm, frame_size) != frame_size) 
			break; 
		 
		if (write(fd, sndpcm->data_buf, c) != c) { 			
			printf("Line(%d) error \n",__LINE__);
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
		fprintf(stderr, "Usage: ./capture <file name>.wav/n"); 
		return -1; 
	}  
	memset(&record, 0, sizeof(record)); 
	filename = argv[1]; 
	remove(filename); 
	if ((fd = open(filename, O_WRONLY | O_CREAT, 0644)) == -1) { 
		fprintf(stderr, "open fail: %s/n", filename); 
		return -1; 
	} 
 
	if (snd_pcm_open(&record.handle, devicename, SND_PCM_STREAM_CAPTURE, 0) < 0) { 
		printf("Line(%d) error \n",__LINE__);
		goto Err; 
	} 
	//配置参数
	if (WAV_Configure_Params(&wav) < 0) { 
		printf("Line(%d) error \n",__LINE__);
		goto Err; 
	} 
 
	if (WAV_Set_Params(&record, &wav) < 0) { 
		printf("Line(%d) error \n",__LINE__);
		goto Err; 
	} 
	/* 开始录音 */
	WAV_Record(&record, &wav, fd); 
	snd_pcm_drain(record.handle); 
	
	close(fd); 
	free(record.data_buf); 
	snd_pcm_close(record.handle); 
	return 0; 
 
Err: 
	close(fd); 
	remove(filename); 
	if (record.data_buf) free(record.data_buf); 
	if (record.handle) snd_pcm_close(record.handle); 
	return -1; 
} 
