#include "sensor.h"
#include "timer.h"
#include "function_set.h"
#include "main.h"
#include "arm_math.h"
#include "usart.h"
#include <stdio.h>

static const uint8_t _log_level = 0;

#define CUTOFF_FREQ_HIGH 5ul
#define SAMPLE_FREQ 2000ul
#define MAX_ADC_SAMPLE 3080

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern TIM_HandleTypeDef htim3;

static struct {
    struct {
        float slope;
        float current;
        float intensity;
        float amplitude;
        float freq;
        uint16_t cross;
        uint8_t pos;
        struct {
            uint16_t t;
            float min;
            float max;
        }periods[8];
    }function_set[2];
}_sensors[2] = {
    {
        .function_set[0].periods[0].min = 3080,
        .function_set[1].periods[0].min = 3080,
    },
    {
        .function_set[0].periods[0].min = 3080,
        .function_set[1].periods[0].min = 3080,
    }
};

static const uint8_t _alpha_lp[] = {0x00, 0x19, 0x33, 0x4c, 0x66, 0x7f, 0x99, 0xb2, 0xcc, 0xe5, 0xfe};
static const float _alpha_hp = ((2 * 3.1416f * CUTOFF_FREQ_HIGH / SAMPLE_FREQ) / (1 + 2 * 3.1416f * CUTOFF_FREQ_HIGH / SAMPLE_FREQ));

static uint8_t _sampling = 0;
static uint32_t _pause = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
        _sampling = 0x01;
    }
}

void sensor_init(void)
{
    HAL_TIM_Base_Start_IT(&htim3);

    LOG_INF("sensor initialized");
}

void sensor_update(void)
{
    if (_sampling && (!_pause || timer_diff(_pause) > 10000))
    {
        _pause = 0;

        uint8_t adc = 0;
        float sample = 0;
        if (_sampling & 0x01)
        {
            HAL_ADC_Start(&hadc1);
            HAL_StatusTypeDef ret = HAL_ADC_PollForConversion(&hadc1, 50);
            if (HAL_OK == ret)
            {
                if (HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC))
                {
                    sample = HAL_ADC_GetValue(&hadc1);
                    _sampling &= ~0x01;
                    if (sample < 255)
                    {
                        _sampling |= 0x02;
                    }
                }
            }
            else
            {
                LOG_ERR("HAL_ADC_PollForConversion hadc1 ret:%u", ret);
            }
            HAL_ADC_Stop(&hadc1);
        }

        if (_sampling & 0x02)
        {
            HAL_ADC_Start(&hadc2);
            HAL_StatusTypeDef ret = HAL_ADC_PollForConversion(&hadc2, 50);
            if (HAL_OK == ret)
            {
                if (HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc2), HAL_ADC_STATE_REG_EOC))
                {
                    adc = 1;
                    sample = HAL_ADC_GetValue(&hadc2);
                    sample /= 11;
                    _sampling &= ~0x02;
                }
            }
            else
            {
                LOG_ERR("HAL_ADC_PollForConversion hadc2 ret:%u", ret);
            }
            HAL_ADC_Stop(&hadc2);
        }

        if (!_sampling)
        {
            //LOG_DBG_SEQ(50, "%.02f ", sample);

            for (uint8_t fs = 0; fs < MAX_FUNCTION_SET; fs++)
            {
                struct function_set *f = function_set_get(fs);
          
                float current = (2 * 3.1416f * f->frequency.max / SAMPLE_FREQ) / (1 + 2 * 3.1416f * f->frequency.max / SAMPLE_FREQ);
                current = current * sample + (1 - current) * _sensors[0].function_set[fs].current;

                //LOG_DBG_SEQ(50, "%.2f ", current);
           
                _sensors[0].function_set[fs].intensity = ((255 - _alpha_lp[f->intensity.filter]) * current + _alpha_lp[f->intensity.filter] * _sensors[0].function_set[fs].intensity) / 255;
                
                //LOG_DBG_SEQ(20, "%6.2f ", _sensors[0].function_set[fs].intensity);

                uint8_t pos = _sensors[0].function_set[fs].pos;
                if (current < _sensors[0].function_set[fs].periods[pos].min)
                {
                    _sensors[0].function_set[fs].periods[pos].min = current;
                }
                if (current > _sensors[0].function_set[fs].periods[pos].max)
                {
                    _sensors[0].function_set[fs].periods[pos].max = current;
                }

                float slope = _alpha_hp * (current - _sensors[0].function_set[fs].current) + (1 - _alpha_hp) * _sensors[0].function_set[fs].slope;
                if (slope <= 0 && _sensors[0].function_set[fs].slope >= 0 && _sensors[0].function_set[fs].cross >= SAMPLE_FREQ / 125 || _sensors[0].function_set[fs].cross >= SAMPLE_FREQ / 5)
                {
                    uint8_t smoothing = f->amplitude.filter;
                    _sensors[0].function_set[fs].amplitude = ((255 - _alpha_lp[smoothing]) * (_sensors[0].function_set[fs].periods[pos].max - _sensors[0].function_set[fs].periods[pos].min) + _alpha_lp[smoothing] * _sensors[0].function_set[fs].amplitude) / 255;

                    _sensors[0].function_set[fs].periods[pos].t = _sensors[0].function_set[fs].cross;

                    float delta = (f->frequency.sensitivity * 30 / 1000.0) / 14.0;
                    arm_vexp_f32(&delta, &delta, 1);
                    delta = (500 * current + 1) * (delta - 1) / 500;
                    delta = (delta < 1) ? 1 : delta;
                    //LOG_DBG_SEQ(10, "%4u-%6.3f(%6.3f) ", (SAMPLE_FREQ / _sensors[0].function_set[fs].cross), _sensors[0].function_set[fs].periods[pos].max - _sensors[0].function_set[fs].periods[pos].min, delta);

                    uint8_t count = 0;
                    uint32_t t = 0;
                    for (uint8_t i = 0; i < 8; i++)
                    {
                        if (_sensors[0].function_set[fs].periods[i].max - _sensors[0].function_set[fs].periods[i].min > delta)
                        {
                            count++;
                            t += _sensors[0].function_set[fs].periods[i].t;
                        }
                    }

                    uint32_t freq = 0;
                    if (count && t)
                    {
                        freq = SAMPLE_FREQ * count / t;
                    }
                    uint8_t smoothing = f->frequency.filter;
                    _sensors[0].function_set[fs].freq = ((255 - _alpha_lp[smoothing]) * freq + _alpha_lp[smoothing] * _sensors[0].function_set[fs].freq) / 255;
                   
                    //LOG_DBG_SEQ(20, "%.2f ", _sensors[0].function_set[fs].freq);

                    pos = (pos + 1) % 8;
                    _sensors[0].function_set[fs].pos = pos;

                    _sensors[0].function_set[fs].cross = 0;
                    _sensors[0].function_set[fs].periods[pos].min = 3080;
                    _sensors[0].function_set[fs].periods[pos].max = 0;
                }
                _sensors[0].function_set[fs].cross++;
                _sensors[0].function_set[fs].current = current;
                _sensors[0].function_set[fs].slope = slope;
            }
        }
    }
}

uint8_t sensor_type_get(void)
{
    return 2;
}

uint16_t sensor_intensity_get(uint8_t sensor, uint8_t fs)
{
    float intensity = 500 * 3.3 * _sensors[sensor].function_set[fs].intensity / 4095 + 1;
    arm_vlog_f32(&intensity, &intensity, 1);
    return floor(intensity * 14);
}

uint16_t sensor_amplitude_get(uint8_t sensor, uint8_t fs)
{
    return floor(_sensors[sensor].function_set[fs].amplitude);
}

uint16_t sensor_flicker_frequency_get(uint8_t sensor, uint8_t fs)
{
    return floor(_sensors[sensor].function_set[fs].freq);
}

uint16_t sensor_quality_get(uint8_t sensor, uint8_t fs_id)
{
    uint16_t intensity = sensor_intensity_get(sensor, fs_id);
    uint16_t frequency = sensor_flicker_frequency_get(sensor, fs_id);
    uint16_t amplitude = sensor_amplitude_get(sensor, fs_id);

    struct function_set *f = function_set_get(fs_id);
    int quality = 100;
    quality *= MAX(intensity - f->intensity.trip.drop_out, 0);
    quality *= MAX(frequency - f->frequency.trip.drop_out, 0);
    quality *= MAX(amplitude - f->amplitude.trip.drop_out, 0);
    quality /= MAX(f->intensity.normalization.value, f->intensity.normalization.high);
    quality /= MAX(f->frequency.normalization.value, f->frequency.normalization.high);
    quality /= MAX(f->amplitude.normalization.value, f->amplitude.normalization.high);

    //LOG_DBG("intensity:%u frequency:%u amplitude:%u quality:%u", intensity, frequency, amplitude, quality);
    return quality;
}

void sensor_sample_pause(uint8_t action)
{
    LOG_WRN("action:%u", action);
    _pause = action ? timer_start() : 0;
}
