#ifndef HAL_ADC_H_INCLUDED
#define HAL_ADC_H_INCLUDED


#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

enum hal_adc_resolution{
    ADC_RES_8BIT = 0,
    ADC_RES_10BIT,
    ADC_RES_12BIT,
    ADC_RES_14BIT
};
typedef enum hal_adc_resolution hal_adc_resolution_t;

struct hal_adc_channel_config{
	// from 0 to 7
    uint8_t channel;
	// from 0 to 9; 0 is not connected, 9 is VDD
    uint8_t analog_input;
};
typedef struct hal_adc_channel_config hal_adc_channel_config_t;

void hal_adc_config(hal_adc_resolution_t res, hal_adc_channel_config_t const * const config, uint8_t size);

bool hal_adc_sample(int16_t *data_source, uint8_t size, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /*HAL_ADC_H_INCLUDED*/


