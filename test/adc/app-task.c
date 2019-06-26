#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <FreeRTOS.h>
#include <task.h>

#include "board/bsp.h"
#include "board/pins.h"
#include "cpu/io.h"
#include "hal/hal_adc.h"
#include "led.h"

#define ADC_HIGHEST_TEMP (400)
#define ADC_LOWEST_TEMP (710)

#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) ((((ADC_VALUE)*600) / 1024) * 6)

static TaskHandle_t xHandle;
static uint8_t cal_counter;
static int16_t raw_sample[2];

static const hal_adc_channel_config_t adc_config[] = {
	{
		.channel = 0,
		.analog_input = BOARD_NTC0_SENSE_ai,
		.mode = ADC_MODE_SINGLE_END,
		.gain = ADC_GAIN_1_4
	},
	{
		.channel = 1,
		.analog_input = BOARD_NTC2_SENSE_ai,
		.mode = ADC_MODE_SINGLE_END,
		.gain = ADC_GAIN_1_4
	}
};


static void task_testcase(void *pvParameters) {
	for (;;) {
		board_led_on(LED0);
		/*Perform measurement*/
		board_ntc_on();
		/*Wait 1usec or less*/
		if (hal_adc_sample(raw_sample, 2, 10)) {
			
		}
		board_ntc_off();

		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}


void tasks_init(void) {
	hal_adc_config(ADC_RES_10BIT, adc_config, 2);

	if (xTaskCreate(task_testcase, "task_tc", configMINIMAL_STACK_SIZE, (void *)NULL,
			tskIDLE_PRIORITY, &xHandle) != pdPASS) {
		while (true) {;}
	}
}