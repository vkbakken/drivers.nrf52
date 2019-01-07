#include "board/pins.h"
#include "cpu/io.h"
#include "hal/hal_i2c_master.h"

static SemaphoreHandle_t xSemaphoreI2C = NULL;
static StaticSemaphore_t xSemaphoreBufferI2C;

__STATIC_INLINE uint32_t hal_i2c_nrfx_freq_convert(i2c_frequency_t freq) {
	switch (freq) {
	case I2C_FREQ_100K:
		return TWIM_FREQUENCY_FREQUENCY_K100;
	case I2C_FREQ_250K:
		return TWIM_FREQUENCY_FREQUENCY_K250;
	case I2C_FREQ_400K:
		return TWIM_FREQUENCY_FREQUENCY_K400;
	}
}


void hal_i2c_init(hal_i2c_instance_t *i2c_instance) {
	NRF_P0->PIN_CNF[BOARD_MIC_CLK_bp] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
										(GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
										(GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos) |
										(GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos) |
										(GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NRF_P0->PIN_CNF[BOARD_MIC_DATA_bp] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
										 (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
										 (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos) |
										 (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos) |
										 (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NRF_TWIM0->PSEL.SCL = BOARD_MIC_CLK_bp;
	NRF_TWIM0->PSEL.SDA = BOARD_MIC_DATA_bp;
	NRF_TWIM0->FREQUENCY = hal_i2c_nrfx_freq_convert(i2c_instance->config.frequency);

	NVIC_ClearPendingIRQ(TWIM0_TWIS0_IRQn);
	NVIC_EnableIRQ(TWIM0_TWIS0_IRQn);

	NRF_TWIM0->ENABLE = (TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos);

	xSemaphoreI2C = xSemaphoreCreateBinaryStatic(&xSemaphoreBufferI2C);
	xSemaphoreGive(xSemaphoreI2C);
}


void hal_i2c_deinit(void) {
	NRF_TWIM0->ENABLE = (TWIM_ENABLE_ENABLE_Disabled << TWIM_ENABLE_ENABLE_Pos);
	NVIC_DisableIRQ(TWIM0_TWIS0_IRQn);
	vSemaphoreDelete(xSemaphoreI2C);
}


bool hal_i2c_write(hal_i2c_instance_t *i2c_instance, uint8_t *data, uint8_t size) {
	bool ret = false;

	if (xSemaphoreTake(xSemaphoreI2C, 0) == pdTRUE) {
		NRF_TWIM0->INTENCLR = TWIM_INTENSET_STOPPED_Msk | TWIM_INTENSET_ERROR_Msk |
							  TWIM_INTENSET_SUSPENDED_Msk | TWIM_INTENSET_RXSTARTED_Msk |
							  TWIM_INTENSET_TXSTARTED_Msk | TWIM_INTENSET_LASTRX_Msk |
							  TWIM_INTENSET_LASTTX_Msk;

		NRF_TWIM0->ADDRESS = i2c_instance->slave_address;
		NRF_TWIM0->TXD.PTR = (uint32_t)data;
		NRF_TWIM0->TXD.MAXCNT = size;

		NRF_TWIM0->EVENTS_ERROR = 0x0UL;
		NRF_TWIM0->EVENTS_STOPPED = 0x0UL;
		NRF_TWIM0->EVENTS_LASTTX = 0X0UL;
		NRF_TWIM0->EVENTS_LASTRX = 0X0UL;

		NRF_TWIM0->INTENSET = TWIM_INTENSET_STOPPED_Msk | TWIM_INTENSET_LASTTX_Msk;
		NRF_TWIM0->TASKS_STARTTX = 0x1UL;

		ret = xSemaphoreTake(xSemaphoreI2C, i2c_instance->timeout);
		xSemaphoreGive(xSemaphoreI2C);
	}

	return ret;
}


bool hal_i2c_read(hal_i2c_instance_t *i2c_instance, uint8_t *data, uint8_t size) {
	bool ret = false;

	if (xSemaphoreTake(xSemaphoreI2C, 0) == pdTRUE) {
		NRF_TWIM0->INTENCLR = TWIM_INTENSET_STOPPED_Msk | TWIM_INTENSET_ERROR_Msk |
							  TWIM_INTENSET_SUSPENDED_Msk | TWIM_INTENSET_RXSTARTED_Msk |
							  TWIM_INTENSET_TXSTARTED_Msk | TWIM_INTENSET_LASTRX_Msk |
							  TWIM_INTENSET_LASTTX_Msk;

		NRF_TWIM0->ADDRESS = i2c_instance->slave_address;
		NRF_TWIM0->RXD.PTR = (uint32_t)data;
		NRF_TWIM0->RXD.MAXCNT = size;

		NRF_TWIM0->EVENTS_ERROR = 0x0UL;
		NRF_TWIM0->EVENTS_STOPPED = 0x0UL;
		NRF_TWIM0->EVENTS_LASTTX = 0X0UL;
		NRF_TWIM0->EVENTS_LASTRX = 0X0UL;

		NRF_TWIM0->INTENSET = TWIM_INTENSET_STOPPED_Msk | TWIM_INTENSET_LASTRX_Msk;
		NRF_TWIM0->TASKS_STARTRX = 0x1UL;

		ret = xSemaphoreTake(xSemaphoreI2C, i2c_instance->timeout);
		xSemaphoreGive(xSemaphoreI2C);
	}

	return ret;
}


void serialbox0_handler(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (NRF_TWIM0->EVENTS_LASTTX) {
		NRF_TWIM0->TASKS_STOP = 0x1UL;
	} else if (NRF_TWIM0->EVENTS_LASTRX) {
		NRF_TWIM0->TASKS_STOP = 0x1UL;
	} else if (NRF_TWIM0->EVENTS_STOPPED) {
		NRF_TWIM0->EVENTS_STOPPED = 0x0UL;
		xSemaphoreGiveFromISR(xSemaphoreI2C, &xHigherPriorityTaskWoken);
	} else if (NRF_TWIM0->EVENTS_ERROR) {
		
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}