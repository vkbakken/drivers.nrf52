#include "cpu/io.h"
#include "hal/hal_adc.h"

void hal_adc_config(hal_adc_resolution_t res, hal_adc_channel_config_t const *const config, uint8_t size)
{
	NRF_SAADC->RESOLUTION = res;
	NRF_SAADC->OVERSAMPLE = 0;

	// Normal samplerate control from SAMPLE task
	NRF_SAADC->SAMPLERATE = SAADC_SAMPLERATE_MODE_Task << SAADC_SAMPLERATE_MODE_Pos;

	for (uint8_t i = 0; i < size; i++) {
		/*Configure ADC*/
		NRF_SAADC->CH[config[i].channel].CONFIG = (config->gain << SAADC_CH_CONFIG_GAIN_Pos) |
												  (config->mode << SAADC_CH_CONFIG_MODE_Pos) |
												  (SAADC_CH_CONFIG_REFSEL_VDD1_4 << SAADC_CH_CONFIG_REFSEL_Pos) |
												  (SAADC_CH_CONFIG_RESN_Bypass << SAADC_CH_CONFIG_RESN_Pos) |
												  (SAADC_CH_CONFIG_RESP_Bypass << SAADC_CH_CONFIG_RESP_Pos) |
												  (SAADC_CH_CONFIG_TACQ_10us << SAADC_CH_CONFIG_TACQ_Pos);

		// Configure the SAADC channel with VDD as positive input, no negative input(single ended).
		NRF_SAADC->CH[config[i].channel].PSELP = (uint32_t)config[i].analog_input + 1;
		if (config->mode == ADC_MODE_SINGLE_END) {
			NRF_SAADC->CH[config[i].channel].PSELN = SAADC_CH_PSELN_PSELN_NC;
		} else {
			NRF_SAADC->CH[config[i].channel].PSELN = (uint32_t)config[i].analog_input_diff;
		}
	}

	NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos);

	NRF_SAADC->TASKS_CALIBRATEOFFSET = 1;
	while (NRF_SAADC->EVENTS_CALIBRATEDONE == 0);
	NRF_SAADC->EVENTS_CALIBRATEDONE = 0;
	while (NRF_SAADC->STATUS == (SAADC_STATUS_STATUS_Busy << SAADC_STATUS_STATUS_Pos));

	NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos;

	NVIC_SetPriority(SAADC_IRQn, 0);
}

bool hal_adc_sample(int16_t *data_source, uint8_t size, uint32_t ticks)
{
	__disable_irq();

	NRF_SAADC->RESULT.MAXCNT = size;
	NRF_SAADC->RESULT.PTR = (uint32_t)data_source;
	NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos);
	NRF_SAADC->INTEN = SAADC_INTEN_END_Msk;
	NRF_SAADC->INTENSET = SAADC_INTENSET_END_Msk;

	NVIC_ClearPendingIRQ(SAADC_IRQn);
	NVIC_EnableIRQ(SAADC_IRQn);

	__enable_irq();

	NRF_SAADC->TASKS_START = 1;
	while (NRF_SAADC->EVENTS_STARTED == 0);
	NRF_SAADC->EVENTS_STARTED = 0;

	NRF_SAADC->EVENTS_END = 0;
	NRF_SAADC->TASKS_SAMPLE = 1;

	return true;
}

void saadc_handler(void)
{
	if (NRF_SAADC->EVENTS_END == 1) {
		NRF_SAADC->EVENTS_END = 0;
	}
}