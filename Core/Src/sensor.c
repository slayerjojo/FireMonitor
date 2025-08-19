#include "sensor.h"
#include "timer.h"
#include "function_set.h"
#include "main.h"
#include "arm_math.h"
#include "usart.h"
#include <stdio.h>

#define SAMPLE_FREQ 250
#define MAX_SAMPLE 250
#define MAX_ADC_SAMPLE 1500

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim3;

static uint8_t _sampling = 0;
static float _samples_fft[MAX_SAMPLE * 2] = {0};
static float _fft_result[MAX_SAMPLE] = {0};
static float _samples_raw[MAX_SAMPLE * 2] = {0};
static float _samples_filter[MAX_SAMPLE] = {0};
static uint16_t _filter_smoothing = MAX_SAMPLE;
static uint16_t _fft_smoothing = MAX_SAMPLE;
static uint16_t _sample_tail = MAX_SAMPLE - 1;
static int16_t _sample_total = 0;
static uint32_t _pause = 0;

static arm_cfft_instance_f32 _fft;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
        _sampling = 1;
    }
}

void sensor_init(void)
{
    HAL_TIM_Base_Start_IT(&htim3);

    SEGGER_RTT_printf(0, "sensor initialized\n");
}

void sensor_update(void)
{
    if (_sampling && (!_pause || timer_diff(_pause) > 10000))
    {
        _pause = 0;

        HAL_ADC_Start(&hadc1);
        HAL_StatusTypeDef ret = HAL_ADC_PollForConversion(&hadc1, 50);
        if (HAL_OK != ret)
        {
            SEGGER_RTT_printf(0, "HAL_ADC_PollForConversion ret:%u\n", ret);
            return;
        }
        if (HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC))
        {
            _sampling = 0;
            _sample_tail = (_sample_tail + 1) % MAX_SAMPLE;
            _samples_raw[_sample_tail] = _samples_raw[MAX_SAMPLE + _sample_tail] = HAL_ADC_GetValue(&hadc1);
            /*
            SEGGER_RTT_printf(0, "%.0f ", _samples_raw[_sample_tail]);
            if (1 == _sample_tail % 10)
                SEGGER_RTT_printf(0, "\n");
            */
            _fft_smoothing = _filter_smoothing = MAX_SAMPLE;
            if (_sample_total < MAX_SAMPLE)
                _sample_total++;
        }
        HAL_ADC_Stop(&hadc1);
    }
}

uint8_t sensor_type_get(void)
{
    return 2;
}

uint16_t sensor_intensity_raw_get(uint8_t sensor)
{
    if (sensor)
        return 0;
    return _samples_raw[_sample_tail];
}

static float *sample_smoothing(uint16_t smoothing)
{
    if (_sample_total < smoothing)
        return 0;
    float *samples = &_samples_raw[MAX_SAMPLE + _sample_tail - _sample_total];
    if (smoothing)
    {
        samples = _samples_filter;
        if (_filter_smoothing != smoothing)
        {
            for (int i = 0; i < _sample_total - smoothing; i++)
            {
                arm_mean_f32(&_samples_raw[MAX_SAMPLE + _sample_tail - _sample_total + i], smoothing, &samples[i]);
            }
            _filter_smoothing = smoothing;
        }
    }
    return samples;
}

uint16_t sensor_intensity_get(uint8_t sensor, uint16_t smoothing)
{
    if (sensor)
        return 0;

    float *samples = sample_smoothing(smoothing);
    if (!samples)
        return 0;

    float intensity = 0;
    arm_mean_f32(samples, _sample_total - smoothing, &intensity);

    if (intensity > MAX_ADC_SAMPLE)
    {
        intensity = MAX_ADC_SAMPLE;
    }
    return (uint16_t)ceil(100 * intensity / MAX_ADC_SAMPLE);
}

uint16_t sensor_amplitude_get(uint8_t sensor, uint16_t smoothing)
{
    if (sensor)
        return 0;
    
    float *samples = sample_smoothing(smoothing);
    if (!samples)
        return 0;

    float max = 0;
    arm_max_no_idx_f32(samples, _sample_total - smoothing, &max);
    float min = 0;
    arm_min_no_idx_f32(samples, _sample_total - smoothing, &min);
    
    //SEGGER_RTT_printf(0, "amplitude:%f - %f\n", min, max);

    max -= min;
    if (max > MAX_ADC_SAMPLE)
    {
        max = MAX_ADC_SAMPLE;
    }
    return (uint16_t)ceil(100 * max / MAX_ADC_SAMPLE);
}

uint16_t sensor_flicker_frequency_get(uint8_t sensor, uint16_t smoothing, uint16_t sensitivity, uint16_t max)
{
    if (sensor)
        return 0;
    if (_sample_total < smoothing)
        return 0;
    if (_fft_smoothing != smoothing)
    {
        arm_cfft_init_f32(&_fft, _sample_total - smoothing);
        for (int i = 0; i < _sample_total - smoothing; i++)
        {
            if (smoothing)
            {
                arm_mean_f32(&_samples_raw[MAX_SAMPLE + _sample_tail - _sample_total + i], smoothing, &_samples_fft[i * 2]);
            }
            else
            {
                _samples_fft[i * 2] = _samples_raw[MAX_SAMPLE + _sample_tail - _sample_total + i];
            }
            _samples_fft[i * 2 + 1] = 0;
        }
        arm_cfft_f32(&_fft, _samples_fft, 0, 1);
        arm_cmplx_mag_f32(_samples_fft, _fft_result, _sample_total - smoothing);

        for (int i = 0; i < (_sample_total - smoothing) / 2; i++)
        {
            _fft_result[i] /= (_sample_total - smoothing);
            if (i)
            {
                _fft_result[i] /= 2;
            }
            //SEGGER_RTT_printf(0, "%f, ", _fft_result[i]);
        }
        //SEGGER_RTT_printf(0, "\n");
        _fft_smoothing = smoothing;
    }
    uint16_t flicker = 0;
    for (int i = 1; i < (_sample_total - smoothing) / 2; i++)
    {
        float freq = (float)i * SAMPLE_FREQ / (_sample_total - smoothing);
        
        if (freq > max)
        {
            break;
        }

        if (flicker)
        {
            float freq_flicker = (float)flicker * SAMPLE_FREQ / (_sample_total - smoothing);

            if (_fft_result[i] * (1 - (freq - max * sensitivity / 100) / max) > _fft_result[flicker] * (1 - (freq_flicker - max * sensitivity / 100) / max))
            {
                flicker = i;
            }
        }
        else
        {
            flicker = i;
        }
    }
    if (_fft_result[flicker] < 10)
    {
        flicker = 0;
    }
    //SEGGER_RTT_printf(0, "freq:%f\n", (float)flicker * SAMPLE_FREQ / (_sample_total - smoothing));
    return (uint16_t)ceil((float)flicker * SAMPLE_FREQ / (_sample_total - smoothing) * 100 / max);
}

uint16_t sensor_quality_get(uint8_t sensor, struct function_set *f)
{
    uint16_t intensity = sensor_intensity_get(sensor, f->intensity.filter);
    uint16_t frequency = sensor_flicker_frequency_get(sensor, f->frequency.filter, f->frequency.sensitivity, f->frequency.max);
    uint16_t amplitude = sensor_amplitude_get(sensor, f->amplitude.filter);

    int quality = 100;
    quality *= MAX(intensity - f->intensity.trip.drop_out, 0);
    quality *= MAX(frequency - f->frequency.trip.drop_out, 0);
    quality *= MAX(amplitude - f->amplitude.trip.drop_out, 0);
    quality /= MAX(f->intensity.normalization.value, f->intensity.normalization.high);
    quality /= MAX(f->frequency.normalization.value, f->frequency.normalization.high);
    quality /= MAX(f->amplitude.normalization.value, f->amplitude.normalization.high);

    SEGGER_RTT_printf(0, "intensity:%u frequency:%u amplitude:%u quality:%u\n", intensity, frequency, amplitude, quality);
    return quality;
}

void sensor_sample_pause(uint8_t action)
{
    SEGGER_RTT_printf(0, "sensor_sample_pause action:%u\n", action);
    _pause = action ? timer_start() : 0;
}
