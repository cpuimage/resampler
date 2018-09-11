
#ifdef __cplusplus
extern "C" {
#endif
#define  _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//ref: https://github.com/mackron/dr_libs/blob/master/dr_wav.h
#define DR_WAV_IMPLEMENTATION

#include "timing.h"
#include "dr_wav.h"


void wavWrite_int16(char *filename, int16_t *buffer, int sampleRate, uint32_t totalSampleCount, uint32_t channels) {
    drwav_data_format format;
    format.container = drwav_container_riff;     // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
    format.format = DR_WAVE_FORMAT_PCM;          // <-- Any of the DR_WAVE_FORMAT_* codes.
    format.channels = channels;
    format.sampleRate = (drwav_uint32) sampleRate;
    format.bitsPerSample = 16;
    drwav *pWav = drwav_open_file_write(filename, &format);
    if (pWav) {
        drwav_uint64 samplesWritten = drwav_write(pWav, totalSampleCount, buffer);
        drwav_uninit(pWav);
        if (samplesWritten != totalSampleCount) {
            fprintf(stderr, "write file error.\n");
            exit(1);
        }
    }
}

int16_t *wavRead_int16(char *filename, uint32_t *sampleRate, uint64_t *totalSampleCount, uint32_t *channels) {
    int16_t *buffer = drwav_open_and_read_file_s16(filename, channels, sampleRate, totalSampleCount);
    if (buffer == NULL) {
        fprintf(stderr, "read file error.\n");
        exit(1);
    }
    return buffer;
}


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

//easy f32 version
void simple_resample_f32(const float *input, float *output, uint32_t in_frames, int32_t out_frames, int channels) {
    if (in_frames == out_frames) {
        memcpy(output, input, in_frames * sizeof(float));
        return;
    }
    float pos = 0;
    uint32_t last_pos = in_frames - 1;
    float scale = (float) (1.0 * in_frames) / out_frames;
    for (uint32_t idx = 0; idx < out_frames; idx++) {
        uint32_t p1 = (uint32_t) pos;
        float coef = pos - p1;
        uint32_t p2 = (p1 == last_pos) ? last_pos : p1 + 1;
        for (int c = 0; c < channels; c++) {
            output[idx * channels + c] = (float) ((1.0f - coef) * input[p1 * channels + c] +
                                                  coef * input[p2 * channels + c]);
        }
        pos += scale;
    }
}

//easy s16 version
void simple_resample_s16(const int16_t *input, int16_t *output, uint32_t in_frames, int32_t out_frames, int channels) {
    if (in_frames == out_frames) {
        memcpy(output, input, in_frames * sizeof(int16_t));
        return;
    }
    float pos = 0;
    uint32_t last_pos = in_frames - 1;
    float scale = (float) (1.0 * in_frames) / out_frames;
    for (uint32_t idx = 0; idx < out_frames; idx++) {
        uint32_t p1 = (uint32_t) pos;
        float coef = pos - p1;
        uint32_t p2 = (p1 == last_pos) ? last_pos : p1 + 1;
        for (int c = 0; c < channels; c++) {
            output[idx * channels + c] = (int16_t) ((1.0f - coef) * input[p1 * channels + c] +
                                                    coef * input[p2 * channels + c]);
        }
        pos += scale;
    }
}

//poly f32 version
void poly_resample_f32(const float *input, float *output, int in_frames, int out_frames, int channels) {
    float scale = (float) (1.0 * in_frames) / out_frames;
    int head = (int) (1.0f / scale);
    float pos = 0;
    for (int i = 0; i < head; i++) {
        for (int c = 0; c < channels; c++) {
            float sample_1 = input[0 + c];
            float sample_2 = input[channels + c];
            float sample_3 = input[(channels << 1) + c];
            float poly_3 = sample_1 + sample_3 - sample_2 * 2;
            float poly_2 = sample_2 * 4 - (sample_1 * 3) - sample_3;
            float poly_1 = sample_1;
            output[i * channels + c] = (poly_3 * pos * pos + poly_2 * pos) * 0.5f + poly_1;
        }
        pos += scale;
    }
    float in_pos = head * scale;
    for (int n = head; n < out_frames; n++) {
        int npos = (int) in_pos;
        pos = in_pos - npos + 1;
        for (int c = 0; c < channels; c++) {
            float sample_1 = input[(npos - 1) * channels + c];
            float sample_2 = input[(npos + 0) * channels + c];
            float sample_3 = input[(npos + 1) * channels + c];
            float poly_3 = sample_1 + sample_3 - sample_2 * 2;
            float poly_2 = sample_2 * 4 - (sample_1 * 3) - sample_3;
            float poly_1 = sample_1;
            output[n * channels + c] = (poly_3 * pos * pos + poly_2 * pos) * 0.5f + poly_1;
        }
        in_pos += scale;
    }
}

//poly s16 version
void poly_resample_s16(const int16_t *input, int16_t *output, int in_frames, int out_frames, int channels) {
    float scale = (float) (1.0 * in_frames) / out_frames;
    int head = (int) (1.0f / scale);
    float pos = 0;
    for (int i = 0; i < head; i++) {
        for (int c = 0; c < channels; c++) {
            int sample_1 = input[0 + c];
            int sample_2 = input[channels + c];
            int sample_3 = input[(channels << 1) + c];
            int poly_3 = sample_1 + sample_3 - (sample_2 << 1);
            int poly_2 = (sample_2 << 2) + sample_1 - (sample_1 << 2) - sample_3;
            int poly_1 = sample_1;
            output[i * channels + c] = (int16_t) ((poly_3 * pos * pos + poly_2 * pos) * 0.5f + poly_1);
        }
        pos += scale;
    }
    float in_pos = head * scale;
    for (int n = head; n < out_frames; n++) {
        int npos = (int) in_pos;
        pos = in_pos - npos + 1;
        for (int c = 0; c < channels; c++) {
            int sample_1 = input[(npos - 1) * channels + c];
            int sample_2 = input[(npos + 0) * channels + c];
            int sample_3 = input[(npos + 1) * channels + c];
            int poly_3 = sample_1 + sample_3 - (sample_2 << 1);
            int poly_2 = (sample_2 << 2) + sample_1 - (sample_1 << 2) - sample_3;
            int poly_1 = sample_1;
            output[n * channels + c] = (int16_t) ((poly_3 * pos * pos + poly_2 * pos) * 0.5f + poly_1);
        }
        in_pos += scale;
    }
}


void resampler(char *in_file, char *out_file, uint32_t out_sampleRate) {
    uint32_t in_sampleRate = 0;
    uint64_t totalSampleCount = 0;
    uint32_t channels = 0;
    int16_t *data_in = wavRead_int16(in_file, &in_sampleRate, &totalSampleCount, &channels);
    uint32_t out_size = (uint32_t) (totalSampleCount * ((float) out_sampleRate / in_sampleRate));
    if (data_in) {
        int in_frames = totalSampleCount / channels;
        int out_frames = out_size / channels;
        int16_t *data_out = (int16_t *) malloc(out_size * sizeof(int16_t));
        if (data_out) {
            double startTime = now();
            simple_resample_s16(data_in, data_out, in_frames, out_frames, channels);
            poly_resample_s16(data_in, data_out, in_frames, out_frames, channels);
            double time_interval = calcElapsed(startTime, now());
            printf("time interval: %f ms\n ", (time_interval * 1000));
            wavWrite_int16(out_file, data_out, out_sampleRate, (uint32_t) out_size, channels);
            free(data_out);
        }
        free(data_in);
    }
}

int main(int argc, char *argv[]) {
    printf("Audio Processing\n");
    printf("blog:http://cpuimage.cnblogs.com/\n");
    printf("Audio Resampler\n");
    if (argc < 2)
        return -1;

    char *in_file = argv[1];
    char drive[3];
    char dir[256];
    char fname[256];
    char ext[256];
    char out_file[1024];
    splitpath(in_file, drive, dir, fname, ext);
    sprintf(out_file, "%s%s%s_out%s", drive, dir, fname, ext);
    uint32_t out_sampleRate = 48000;
    resampler(in_file, out_file, out_sampleRate);
    printf("press any key to exit.\n");
    getchar();
    return 0;
}

#ifdef __cplusplus
}
#endif