/*
* @Author: bjarneyang
* @Date:   2016-11-01 10:31:48
* @Last Modified by:   bjarneyang
* @Last Modified time: 2016-11-02 20:42:11
* @FileName: chinese2speech.cpp
*/

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "qtts.h"
#include "msp_cmn.h"
#include "msp_errors.h"

using namespace std;

/* wav音频头部格式 */
typedef struct _wave_pcm_hdr
{
	char    		riff[4];                // = "RIFF"
	int				size_8;                 // = FileSize - 8
	char    		wave[4];                // = "WAVE"
	char    		fmt[4];                 // = "fmt "
	int				fmt_size;		        // = 下一个结构体的大小 : 16

	short int       format_tag;             // = PCM : 1
	short int       channels;               // = 通道数 : 1
	int				samples_per_sec;        // = 采样率 : 8000 | 6000 | 11025 | 16000
	int				avg_bytes_per_sec;      // = 每秒字节数 : samples_per_sec * bits_per_sample / 8
	short int       block_align;            // = 每采样点字节数 : wBitsPerSample / 8
	short int       bits_per_sample;        // = 量化比特数: 8 | 16

	char            data[4];                // = "data";
	int				data_size;              // = 纯数据长度 : FileSize - 44 
} wave_pcm_hdr;

/* 默认wav音频头部数据 */
wave_pcm_hdr default_wav_hdr = 
{
	{ 'R', 'I', 'F', 'F' },
	0,
	{'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '},
	16,
	1,
	1,
	16000,
	32000,
	2,
	16,
	{'d', 'a', 't', 'a'},
	0  
};



int text_to_speech(const std::string &src_text, const std::string& des_path, const std::string & params)
{
	int          ret          = -1;
	FILE*        fp           = NULL;
	const char*  sessionID    = NULL;
	unsigned int audio_len    = 0;
	wave_pcm_hdr wav_hdr      = default_wav_hdr;
	int          synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;

	//printf("text_to_speech text %s\n", src_text.c_str());
	//printf("text_to_speech path %s\n", des_path.c_str());

	fp = fopen(des_path.c_str(), "wb");
	if (NULL == fp)
	{
		printf("open %s error.\n", des_path.c_str());
		return ret;
	}
	/* 开始合成 */
	sessionID = QTTSSessionBegin(params.c_str(), &ret);
	if (MSP_SUCCESS != ret)
	{
		printf("session begin failed, error code: %d.\n", ret);
		fclose(fp);
		return ret;
	}
	ret = QTTSTextPut(sessionID, src_text.c_str(), src_text.size(), NULL);
	if (MSP_SUCCESS != ret)
	{
		printf("text put failed, error code: %d.\n",ret);
		QTTSSessionEnd(sessionID, "TextPutError");
		fclose(fp);
		return ret;
	}
	fwrite(&wav_hdr, sizeof(wav_hdr) ,1, fp); //添加wav音频头，使用采样率为16000
	while (1) 
	{
		/* 获取合成音频 */
		const void* data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
		if (MSP_SUCCESS != ret)
			break;
		if (NULL != data)
		{
			fwrite(data, audio_len, 1, fp);
		    wav_hdr.data_size += audio_len; //计算data_size大小
		}
		if (MSP_TTS_FLAG_DATA_END == synth_status)
			break;
		usleep(100*1000); //防止频繁占用CPU
	}//合成状态synth_status取值请参阅《讯飞语音云API文档》
	if (MSP_SUCCESS != ret)
	{
		printf("audio get failed, error code: %d.\n",ret);
		QTTSSessionEnd(sessionID, "AudioGetError");
		fclose(fp);
		return ret;
	}
	/* 修正wav文件头数据的大小 */
	wav_hdr.size_8 += wav_hdr.data_size + (sizeof(wav_hdr) - 8);
	
	/* 将修正过的数据写回文件头部,音频文件为wav格式 */
	fseek(fp, 4, 0);
	fwrite(&wav_hdr.size_8,sizeof(wav_hdr.size_8), 1, fp); //写入size_8的值
	fseek(fp, 40, 0); //将文件指针偏移到存储data_size值的位置
	fwrite(&wav_hdr.data_size,sizeof(wav_hdr.data_size), 1, fp); //写入data_size的值
	fclose(fp);
	ret = QTTSSessionEnd(sessionID, "Normal");
	if (MSP_SUCCESS != ret)
	{
		printf("session end failed, error code: %d.\n",ret);
	}
	return ret;
}

#ifdef __cplusplus
extern "C" {
#endif
/*
*登录 这里需要写入科大讯飞帐号 在 http://www.iflytek.com/ 申请
*/
int login()
{
	int ret= MSPLogin("帐号","密码", "appid = 58088cdc, work_dir = .");
	if (MSP_SUCCESS != ret)
	{
		printf("login failed, error code: %d.\n", ret);
		return -1;
	}
	return 0;
}

/*
*退出登录
*/
void logout()
{
	MSPLogout();//退出登录
}

/*
*转换
*/
int speech(const char* text,const char* path)
{
	std::string textstr(text);
	std::string filepath(path);
	std::string params("voice_name = xiaoyan, text_encoding = utf8, sample_rate = 16000, speed = 50, volume = 50, pitch = 50, rdn = 2");
	int ret = text_to_speech(textstr, filepath, params);
	if(MSP_SUCCESS!=ret)
	{
		return -1;
	}
	return 0;
}

#ifdef __cplusplus
}
#endif


