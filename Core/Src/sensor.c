#include "sensor.h"
#include "main.h"
#include "arm_math.h"
#include "usart.h"
#include <stdio.h>

#define MAX_SAMPLE 1024

extern ADC_HandleTypeDef hadc1;
volatile uint8_t _sensor_sampling = 0;
static uint16_t _samples[MAX_SAMPLE] = {0};
static arm_cfft_radix4_instance_f32 _fft;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    _sensor_sampling = 0;
}

void sensor_init(void)
{
    _sensor_sampling = 1;
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)_samples, MAX_SAMPLE);

    arm_cfft_radix4_init_f32(&_fft, MAX_SAMPLE, 0, 1);

    SEGGER_RTT_printf(0, "sensor initialized\n");
}

void sensor_update(void)
{
    if (0 == _sensor_sampling)
    {
        static float in[MAX_SAMPLE * 2] = {0};
        static float out[MAX_SAMPLE] = {0};
        
        for (int i = 0; i < MAX_SAMPLE; i++)
        {
            out[i] = (float)_samples[i];
        }

        float intensity;
        arm_mean_f32(out, MAX_SAMPLE, &intensity);

        float std;
        arm_std_f32(out, MAX_SAMPLE, &std);

        float upper = intensity + 2.0 * std;
        float lower = intensity - 2.0 * std;

        float min = -1;
        float max = -1;

        for (int i = 0; i < MAX_SAMPLE; i++)
        {
            in[2 * i] = _samples[i];
            in[2 * i + 1] = 0;
            if (_samples[i] <= upper)
            {
                if (max < 0 || max < _samples[i])
                {
                    max = _samples[i];
                }
            }
            if (_samples[i] >= lower)
            {
                if (min < 0 || min > _samples[i])
                {
                    min = _samples[i];
                }
            }
            //SEGGER_RTT_printf(0, "%04x ", _samples[i]);
        }
        //SEGGER_RTT_printf(0, "\n");
        float amplitude = max - min;
        arm_cfft_radix4_f32(&_fft, in);
        arm_cmplx_mag_f32(in, out, MAX_SAMPLE);
        int flicker = 0;
        for (int i = 0; i < MAX_SAMPLE / 2; i++)
        {
            if (i)
            {
                out[i] /= MAX_SAMPLE / 2;
            }
            else
            {
                out[i] /= MAX_SAMPLE;
            }
            if (out[i] > out[flicker])
            {
                flicker = i;
            }
        }
        //SEGGER_RTT_printf(0, "intensity:%4u std:%3u amp:%4u flicker:%4u\n", (uint16_t)floor(intensity), (uint16_t)floor(std), (uint16_t)amplitude, flicker);

        _sensor_sampling = 1;
        HAL_ADC_Start_DMA(&hadc1, (uint32_t *)_samples, MAX_SAMPLE);
    }
}

uint16_t sensor_intensity_get_percent(void)
{
    return 0;
}

uint16_t sensor_frequency_get_percent(void)
{
    return 0;
}

uint16_t sensor_amplitude_get_percent(void)
{
    return 0;
}

uint16_t sensor_quality_get_percent(void)
{
    return 0;
}
