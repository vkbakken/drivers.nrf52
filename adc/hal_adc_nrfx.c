#include "hal/hal_adc.h"
#include "cpu/io.h"

void hal_adc_config(hal_adc_resolution_t res, hal_adc_channel_config_t const * const config, uint8_t size){
    NRF_SAADC->RESOLUTION = res;
    NRF_SAADC->OVERSAMPLE = 0;
    // Normal samplerate control from SAMPLE task
	NRF_SAADC->SAMPLERATE = SAADC_SAMPLERATE_MODE_Task << SAADC_SAMPLERATE_MODE_Pos;

    for(uint8_t i = 0; i < size; i++)
    {
        /*Configure ADC*/
        NRF_SAADC->CH[config[i].channel].CONFIG = (SAADC_CH_CONFIG_GAIN_Gain1_4    << SAADC_CH_CONFIG_GAIN_Pos) |
                                                (SAADC_CH_CONFIG_MODE_SE         << SAADC_CH_CONFIG_MODE_Pos) |
                                                (SAADC_CH_CONFIG_REFSEL_VDD1_4 << SAADC_CH_CONFIG_REFSEL_Pos) |
                                                (SAADC_CH_CONFIG_RESN_Bypass     << SAADC_CH_CONFIG_RESN_Pos) |
                                                (SAADC_CH_CONFIG_RESP_Bypass     << SAADC_CH_CONFIG_RESP_Pos) |
                                                (SAADC_CH_CONFIG_TACQ_10us        << SAADC_CH_CONFIG_TACQ_Pos);

        // Configure the SAADC channel with VDD as positive input, no negative input(single ended).
        NRF_SAADC->CH[config[i].channel].PSELP = config[i].analog_input << SAADC_CH_PSELP_PSELP_Pos;
        NRF_SAADC->CH[config[i].channel].PSELN = SAADC_CH_PSELN_PSELN_NC << SAADC_CH_PSELN_PSELN_Pos;
    }

    NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos);

    NRF_SAADC->TASKS_CALIBRATEOFFSET = 1;
    while (NRF_SAADC->EVENTS_CALIBRATEDONE == 0);
    NRF_SAADC->EVENTS_CALIBRATEDONE = 0;
    while (NRF_SAADC->STATUS == (SAADC_STATUS_STATUS_Busy << SAADC_STATUS_STATUS_Pos));

    NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos;
}

void hal_adc_sample(int16_t *data_source, uint8_t size){
    NRF_SAADC->RESULT.MAXCNT = size;
	NRF_SAADC->RESULT.PTR = (uint32_t)data_source;
    NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos);

    NRF_SAADC->TASKS_START = 1;
	while (NRF_SAADC->EVENTS_STARTED == 0);
	NRF_SAADC->EVENTS_STARTED = 0;

    NRF_SAADC->TASKS_SAMPLE = 1;
	while (NRF_SAADC->EVENTS_END == 0);
	NRF_SAADC->EVENTS_END = 0;

    NRF_SAADC->TASKS_STOP = 1;
	while (NRF_SAADC->EVENTS_STOPPED == 0);
	NRF_SAADC->EVENTS_STOPPED = 0;

    NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos;
}