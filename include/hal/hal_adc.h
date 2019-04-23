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
	/* 
		Map with the ADC channel
	*/
    uint8_t channel;
	
	/* 
		Map with the ADC input channel (not the pin number)
	*/
    uint8_t analog_input;
    uint8_t analog_input_diff;

    hal_adc_mode_t mode;
	hal_adc_gain_t gain;
}hal_adc_channel_config_t;

/**
 * @brief The interface use for configuring the channel of ADC peripheral
 * 
 * @param res: ADC resolution
 * @param config: reference to configuration array
 * @param size: number of configuration channel
 */
void hal_adc_config(hal_adc_resolution_t res, hal_adc_channel_config_t const * const config, uint8_t size);

/**
 * @brief The interface use for sampling the configured ADC channel
 * 
 * @param data_source: reference to the buffer use for storing data
 * @param size: number of conversion
 * @param ticks: number of tick wait for timeout
 * @return true: successful conversion
 * @return false: timeout
 */
bool hal_adc_sample(int16_t *data_source, uint8_t size, uint32_t ticks);

#ifdef __cplusplus
}
#endif

#endif /*HAL_ADC_H_INCLUDED*/


