
#ifdef __cplusplus
extern "C" {
#endif

#define  _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//采用https://github.com/mackron/dr_libs/blob/master/dr_wav.h 解码
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
void resampler(char *in_file, char *out_file);

//写wav文件
void wavWrite_int16(char *filename, int16_t *buffer, int sampleRate, uint32_t totalSampleCount) {
    drwav_data_format format;
    format.container = drwav_container_riff;     // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
    format.format = DR_WAVE_FORMAT_PCM;          // <-- Any of the DR_WAVE_FORMAT_* codes.
    format.channels = 1;
    format.sampleRate = (drwav_uint32) sampleRate;
    format.bitsPerSample = 16;
    drwav *pWav = drwav_open_file_write(filename, &format);
    if (pWav) {
        drwav_uint64 samplesWritten = drwav_write(pWav, totalSampleCount, buffer);
        drwav_uninit(pWav);
        if (samplesWritten != totalSampleCount) {
            fprintf(stderr, "ERROR\n");
            exit(1);
        }
    }
}
//读取wav文件
int16_t *wavRead_int16(char *filename, uint32_t *sampleRate, uint64_t *totalSampleCount) {
    unsigned int channels;
    int16_t *buffer = drwav_open_and_read_file_s16(filename, &channels, sampleRate, totalSampleCount);
    if (buffer == NULL) {
        printf("读取wav文件失败.");
    }
    //仅仅处理单通道音频
    if (channels != 1) {
        drwav_free(buffer);
        buffer = NULL;
        *sampleRate = 0;
        *totalSampleCount = 0;
    }
    return buffer;
}

//分割路径函数
void splitpath(const char *path, char *drv, char *dir, char *name, char *ext) {
    const char *end;
    const char *p;
    const char *s;
    if (path[0] && path[1] == ':') {
        if (drv) {
            *drv++ = *path++;
            *drv++ = *path++;
            *drv = '\0';
        }
    } else if (drv)
        *drv = '\0';
    for (end = path; *end && *end != ':';)
        end++;
    for (p = end; p > path && *--p != '\\' && *p != '/';)
        if (*p == '.') {
            end = p;
            break;
        }
    if (ext)
        for (s = end; (*ext = *s++);)
            ext++;
    for (p = end; p > path;)
        if (*--p == '\\' || *p == '/') {
            p++;
            break;
        }
    if (name) {
        for (s = p; s < end;)
            *name++ = *s++;
        *name = '\0';
    }
    if (dir) {
        for (s = path; s < p;)
            *dir++ = *s++;
        *dir = '\0';
    }
}

void resampleData(const int16_t *sourceData, int32_t sampleRate, uint32_t srcSize, int16_t *destinationData,
                  int32_t newSampleRate) {
    if (sampleRate == newSampleRate) {
        memcpy(destinationData, sourceData, srcSize * sizeof(int16_t));
        return;
    }
    uint32_t last_pos = srcSize - 1;
    uint32_t dstSize = (uint32_t) (srcSize * ((float) newSampleRate / sampleRate));
    for (uint32_t idx = 0; idx < dstSize; idx++) {
        float index = ((float) idx * sampleRate) / (newSampleRate);
        uint32_t p1 = (uint32_t) index;
        float coef = index - p1;
        uint32_t p2 = (p1 == last_pos) ? last_pos : p1 + 1;
        destinationData[idx] = (int16_t) ((1.0f - coef) * sourceData[p1] + coef * sourceData[p2]);
    }
}


void resampler(char *in_file, char *out_file) {
    //音频采样率
    uint32_t sampleRate = 0;
    //总音频采样数
    uint64_t totalSampleCount = 0;
    int16_t *data_in = wavRead_int16(in_file, &sampleRate, &totalSampleCount);
    int16_t *data_out = (int16_t *) malloc(totalSampleCount * 2 * sizeof(int16_t));
    //如果加载成功
    if (data_in != NULL && data_out != NULL) {
        resampleData(data_in, sampleRate, (uint32_t) totalSampleCount, data_out, sampleRate * 2);
        wavWrite_int16(out_file, data_out,sampleRate * 2, (uint32_t) totalSampleCount * 2);
        free(data_in);
        free(data_out);
    }
  if(data_in) free(data_in);
  if(data_out) free(data_out);
}

int main(int argc, char *argv[]) {
	printf("Audio Processing\n");
	printf("博客:http://tntmonks.cnblogs.com/\n");
	printf("音频插值重采样\n");
     if (argc < 2)
         return -1;
         
    char *in_file =  argv[1];
    char drive[3];
    char dir[256];
    char fname[256];
    char ext[256];
    char out_file[1024];
    splitpath(in_file, drive, dir, fname, ext);
    sprintf(out_file, "%s%s%s_out%s", drive, dir, fname, ext);
    resampler(in_file, out_file);
    getchar();
    printf("按任意键退出程序 \n");
    return 0;
}

#ifdef __cplusplus
}
#endif