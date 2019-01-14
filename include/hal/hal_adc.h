#ifndef HAL_ADC_H_INCLUDED
#define HAL_ADC_H_INCLUDED


#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ADC_RES_8BIT = 0x0UL,
    ADC_RES_10BIT,
    ADC_RES_12BIT,
    ADC_RES_14BIT
}hal_adc_resolution_t;


typedef enum {
    ADC_MODE_SINGLE_END = 0x0UL,
    ADC_MODE_DIFFERENTIAL
}hal_adc_mode_t;


typedef enum {
    ADC_GAIN_1_6 = 0x0UL,
    ADC_GAIN_1_5,
    ADC_GAIN_1_4,
    ADC_GAIN_1_3,
	ADC_GAIN_1_2,
	ADC_GAIN_1,
    ADC_GAIN_2,
    ADC_GAIN_4
}hal_adc_gain_t;


typedef struct {
	// from 0 to 7
    uint8_t channel;
	
	// from 0 to 9; 0 is not connected, 9 is VDD
    uint8_t analog_input;
    uint8_t analog_input_diff;

    hal_adc_mode_t mode;
	hal_adc_gain_t gain;
}hal_adc_channel_config_t;

/**
 * @brief 
 * 
 * @param res 
 * @param config 
 * @param size 
 */
void hal_adc_config(hal_adc_resolution_t res, hal_adc_channel_config_t const * const config, uint8_t size);

/**
 * @brief 
 * 
 * @param data_source 
 * @param size 
 * @param ticks 
 * @return true 
 * @return false 
 */
bool hal_adc_sample(int16_t *data_source, uint8_t size, uint32_t ticks);

#ifdef __cplusplus
}
#endif

#endif /*HAL_ADC_H_INCLUDED*/


