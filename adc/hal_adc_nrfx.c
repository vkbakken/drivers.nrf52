#include "cpu/io.h"
#include "hal/hal_adc.h"


#define __HAL_DISABLE_INTERRUPTS(x) \
	do {                            \
		x = __get_PRIMASK();        \
		__disable_irq();            \
	} while (0);

#define __HAL_ENABLE_INTERRUPTS(x) \
	do {                           \
		if (!x) {                  \
			__enable_irq();        \
		}                          \
	} while (0);


static TaskHandle_t xTaskToNotify = NULL;
static SemaphoreHandle_t xSemaphoreADC = NULL;
static StaticSemaphore_t xSemaphoreBufferADC;

void hal_adc_config(hal_adc_resolution_t res, hal_adc_channel_config_t const *const config, uint8_t size) {
	NRF_SAADC->RESOLUTION = res;
	NRF_SAADC->OVERSAMPLE = 0;
	// Normal samplerate control from SAMPLE task
	NRF_SAADC->SAMPLERATE = SAADC_SAMPLERATE_MODE_Task << SAADC_SAMPLERATE_MODE_Pos;

	for (uint8_t i = 0; i < size; i++) {
		/*Configure ADC*/
		NRF_SAADC->CH[config[i].channel].CONFIG = (SAADC_CH_CONFIG_GAIN_Gain1_4 << SAADC_CH_CONFIG_GAIN_Pos) |
												  (SAADC_CH_CONFIG_MODE_SE << SAADC_CH_CONFIG_MODE_Pos) |
												  (SAADC_CH_CONFIG_REFSEL_VDD1_4 << SAADC_CH_CONFIG_REFSEL_Pos) |
												  (SAADC_CH_CONFIG_RESN_Bypass << SAADC_CH_CONFIG_RESN_Pos) |
												  (SAADC_CH_CONFIG_RESP_Bypass << SAADC_CH_CONFIG_RESP_Pos) |
												  (SAADC_CH_CONFIG_TACQ_10us << SAADC_CH_CONFIG_TACQ_Pos);

		// Configure the SAADC channel with VDD as positive input, no negative input(single ended).
		NRF_SAADC->CH[config[i].channel].PSELP = (uint32_t)config[i].analog_input << SAADC_CH_PSELP_PSELP_Pos;
		NRF_SAADC->CH[config[i].channel].PSELN = SAADC_CH_PSELN_PSELN_NC << SAADC_CH_PSELN_PSELN_Pos;
	}

	NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos);

	NRF_SAADC->TASKS_CALIBRATEOFFSET = 1;
	while (NRF_SAADC->EVENTS_CALIBRATEDONE == 0);
	NRF_SAADC->EVENTS_CALIBRATEDONE = 0;
	while (NRF_SAADC->STATUS == (SAADC_STATUS_STATUS_Busy << SAADC_STATUS_STATUS_Pos));

	NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos;

    NVIC_SetPriority(SAADC_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    
	xSemaphoreADC = xSemaphoreCreateBinaryStatic(&xSemaphoreBufferADC);
    xSemaphoreGive(xSemaphoreADC);

//    xTaskToNotify = xTaskGetCurrentTaskHandle();
//	xTaskNotifyGive(xTaskToNotify);
}


bool hal_adc_sample(int16_t *data_source, uint8_t size, uint32_t ticks) {
	bool ret = false;
	uint32_t ctx;

//    if(ulTaskNotifyTake(pdTRUE, 0) != 0){
//		__HAL_DISABLE_INTERRUPTS(ctx);
//
//		NRF_SAADC->RESULT.MAXCNT = size;
//		NRF_SAADC->RESULT.PTR = (uint32_t)data_source;    
//		NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos);
//		NRF_SAADC->INTEN = SAADC_INTEN_END_Msk;
//		NRF_SAADC->INTENSET = SAADC_INTENSET_END_Msk;    
//
//		NVIC_ClearPendingIRQ(SAADC_IRQn);
//		NVIC_EnableIRQ(SAADC_IRQn);
//
//
//		__HAL_ENABLE_INTERRUPTS(ctx);
//
//		NRF_SAADC->TASKS_START = 1;
//		while (NRF_SAADC->EVENTS_STARTED == 0);
//		NRF_SAADC->EVENTS_STARTED = 0;
//		NRF_SAADC->EVENTS_END = 0;
//		NRF_SAADC->TASKS_SAMPLE = 1;
//
//		if(ulTaskNotifyTake(pdTRUE, ticks) != 0){
//			ret = true;
//			xTaskNotifyGive(xTaskToNotify);
//		}
//
//
//		NRF_SAADC->TASKS_STOP = 1;
//		while (NRF_SAADC->EVENTS_STOPPED == 0);
//		NRF_SAADC->EVENTS_STOPPED = 0;
//
//		NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos;
//		NVIC_DisableIRQ(SAADC_IRQn);
//	}
	

	
    if(xSemaphoreTake(xSemaphoreADC, 1) == pdTRUE){
		__HAL_DISABLE_INTERRUPTS(ctx);

		NRF_SAADC->RESULT.MAXCNT = size;
		NRF_SAADC->RESULT.PTR = (uint32_t)data_source;    
		NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos);
		NRF_SAADC->INTEN = SAADC_INTEN_END_Msk;
		NRF_SAADC->INTENSET = SAADC_INTENSET_END_Msk;    
    
		NVIC_ClearPendingIRQ(SAADC_IRQn);
		NVIC_EnableIRQ(SAADC_IRQn);
    

		__HAL_ENABLE_INTERRUPTS(ctx);

		NRF_SAADC->TASKS_START = 1;
		while (NRF_SAADC->EVENTS_STARTED == 0);
		NRF_SAADC->EVENTS_STARTED = 0;

		NRF_SAADC->EVENTS_END = 0;
 		NRF_SAADC->TASKS_SAMPLE = 1;
 		ret = xSemaphoreTake(xSemaphoreADC, ticks);
		xSemaphoreGive(xSemaphoreADC);

        NRF_SAADC->TASKS_STOP = 1;
		while (NRF_SAADC->EVENTS_STOPPED == 0);
		NRF_SAADC->EVENTS_STOPPED = 0;
	
		NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos;
		NVIC_DisableIRQ(SAADC_IRQn);
 	}	

	return ret;
}


uint32_t counter = 0;
void saadc_handler(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE, xResult;
	++counter;
	if (NRF_SAADC->EVENTS_END == 1) {
		NRF_SAADC->EVENTS_END = 0;
//        vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);

        xSemaphoreGiveFromISR(xSemaphoreADC, &xHigherPriorityTaskWoken);
	}	

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}