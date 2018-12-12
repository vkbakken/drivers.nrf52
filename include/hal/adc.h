#ifndef HAL_ADC_H_INCLUDED
#define HAL_ADC_H_INCLUDED


#include <stdint.h>


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
    uint8_t channel;
    uint32_t analog_input;
}
typedef struct hal_adc_channel_config hal_adc_channel_config_t;

void hal_adc_init(hal_adc_resolution_t res);
void hal_adc_channel_config(hal_adc_channel_config_t *config);
void hal_adc_sample(uint8_t count, uint16_t data_source);

#ifdef __cplusplus
}
#endif
#endif /*HAL_ADC_H_INCLUDED*/


