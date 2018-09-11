# resampler
A Simple and Efficient Audio Resampler Implementation in C.

Example
=======

```C
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
        //  simple version
            simple_resample_s16(data_in, data_out, in_frames, out_frames, channels);
        //  poly version
        //  poly_resample_s16(data_in, data_out, in_frames, out_frames, channels);
            wavWrite_int16(out_file, data_out, out_sampleRate, (uint32_t) out_size, channels);
            free(data_out);
        }
        free(data_in);
    }
}
```
