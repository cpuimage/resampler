# resampler
A Simple and Efficient Audio Resampler Implementation in C.

Example
=======

```C
uint64_t Resample_f32(const float *input, float *output, int inSampleRate, int outSampleRate, uint64_t inputSize,
                      uint32_t channels
) {
    if (input == NULL)
        return 0;
    uint64_t outputSize = inputSize * outSampleRate / inSampleRate;
    if (output == NULL)
        return outputSize;
    double stepDist = ((double) inSampleRate / (double) outSampleRate);
    const uint64_t fixedFraction = (1LL << 32);
    const double normFixed = (1.0 / (1LL << 32));
    uint64_t step = ((uint64_t) (stepDist * fixedFraction + 0.5));
    uint64_t curOffset = 0;
    for (uint32_t i = 0; i < outputSize; i += 1) {
        for (uint32_t c = 0; c < channels; c += 1) {
            *output++ = (float) (input[c] + (input[c + channels] - input[c]) * (
                    (double) (curOffset >> 32) + ((curOffset & (fixedFraction - 1)) * normFixed)
            )
            );
        }
        curOffset += step;
        input += (curOffset >> 32) * channels;
        curOffset &= (fixedFraction - 1);
    }
    return outputSize;
}


uint64_t Resample_s16(const int16_t *input, int16_t *output, int inSampleRate, int outSampleRate, uint64_t inputSize,
                      uint32_t channels
) {
    if (input == NULL)
        return 0;
    uint64_t outputSize = inputSize * outSampleRate / inSampleRate;
    if (output == NULL)
        return outputSize;
    double stepDist = ((double) inSampleRate / (double) outSampleRate);
    const uint64_t fixedFraction = (1LL << 32);
    const double normFixed = (1.0 / (1LL << 32));
    uint64_t step = ((uint64_t) (stepDist * fixedFraction + 0.5));
    uint64_t curOffset = 0;
    for (uint32_t i = 0; i < outputSize; i += 1) {
        for (uint32_t c = 0; c < channels; c += 1) {
            *output++ = (int16_t) (input[c] + (input[c + channels] - input[c]) * (
                    (double) (curOffset >> 32) + ((curOffset & (fixedFraction - 1)) * normFixed)
            )
            );
        }
        curOffset += step;
        input += (curOffset >> 32) * channels;
        curOffset &= (fixedFraction - 1);
    }
    return outputSize;
}
```



# Donate

If the project is useful to you, and you like it, you can buy me a beer.
===========================================================
 
### Alipay donate
![Scan QRCode donate me via Alipay](https://img2018.cnblogs.com/blog/824862/201809/824862-20180930223557236-1709972421.png)

**Scan QRCode donate me via Alipay**
 
### WeChat donate
![Scan QRCode donate me via WeChat](https://img2018.cnblogs.com/blog/824862/201809/824862-20180930223603138-1708589189.png)

**Scan QRCode donate me via WeChat**