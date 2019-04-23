#include "cpu/io.h"
#include "hal/hal_i2c_master.h"


#if	defined(NRF52810_H)
	#define I2C_REG				NRF_TWIM0
    #define I2C_IRQ				TWIM0_TWIS0_IRQn
    #define I2C_IRQ_HANDLER		twi0_handler
#elif defined(NRF52_H)
	#define I2C_REG				NRF_TWIM0
    #define I2C_IRQ				SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn
    #define I2C_IRQ_HANDLER		serialbox0_handler
#else
	#error "This driver only support NRF52 and NRF52810 CPU"
#endif


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


void hal_i2c_init(hal_i2c_instance_t const *const i2c_instance) {
	I2C_REG->PSEL.SCL = i2c_instance->config.pin_SCL;
	I2C_REG->PSEL.SDA = i2c_instance->config.pin_SDA;
	I2C_REG->FREQUENCY = hal_i2c_nrfx_freq_convert(i2c_instance->config.frequency);

	NVIC_ClearPendingIRQ(I2C_IRQ);
	NVIC_EnableIRQ(I2C_IRQ);

	I2C_REG->ENABLE = (TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos);

	xSemaphoreI2C = xSemaphoreCreateBinaryStatic(&xSemaphoreBufferI2C);
	xSemaphoreGive(xSemaphoreI2C);
}


void hal_i2c_deinit(void) {
	I2C_REG->ENABLE = (TWIM_ENABLE_ENABLE_Disabled << TWIM_ENABLE_ENABLE_Pos);
	NVIC_DisableIRQ(I2C_IRQ);
	vSemaphoreDelete(xSemaphoreI2C);
}


bool hal_i2c_write(hal_i2c_instance_t const *const i2c_instance, uint8_t *data, uint8_t size) {
	bool ret = false;

	if (xSemaphoreTake(xSemaphoreI2C, 0) == pdTRUE) {
		I2C_REG->INTENCLR = TWIM_INTENSET_STOPPED_Msk | TWIM_INTENSET_ERROR_Msk |
							  TWIM_INTENSET_SUSPENDED_Msk | TWIM_INTENSET_RXSTARTED_Msk |
							  TWIM_INTENSET_TXSTARTED_Msk | TWIM_INTENSET_LASTRX_Msk |
							  TWIM_INTENSET_LASTTX_Msk;

		I2C_REG->ADDRESS = i2c_instance->slave_address;
		I2C_REG->TXD.PTR = (uint32_t)data;
		I2C_REG->TXD.MAXCNT = size;

		I2C_REG->EVENTS_ERROR = 0x0UL;
		I2C_REG->EVENTS_STOPPED = 0x0UL;
		I2C_REG->EVENTS_LASTTX = 0X0UL;
		I2C_REG->EVENTS_LASTRX = 0X0UL;

		I2C_REG->INTENSET = TWIM_INTENSET_STOPPED_Msk | TWIM_INTENSET_LASTTX_Msk;
		I2C_REG->TASKS_STARTTX = 0x1UL;

		ret = xSemaphoreTake(xSemaphoreI2C, i2c_instance->timeout);
		xSemaphoreGive(xSemaphoreI2C);
	}

	return ret;
}


bool hal_i2c_read(hal_i2c_instance_t const *const i2c_instance, uint8_t *data, uint8_t size) {
	bool ret = false;

	if (xSemaphoreTake(xSemaphoreI2C, 0) == pdTRUE) {
		I2C_REG->INTENCLR = TWIM_INTENSET_STOPPED_Msk | TWIM_INTENSET_ERROR_Msk |
							  TWIM_INTENSET_SUSPENDED_Msk | TWIM_INTENSET_RXSTARTED_Msk |
							  TWIM_INTENSET_TXSTARTED_Msk | TWIM_INTENSET_LASTRX_Msk |
							  TWIM_INTENSET_LASTTX_Msk;

		I2C_REG->ADDRESS = i2c_instance->slave_address;
		I2C_REG->RXD.PTR = (uint32_t)data;
		I2C_REG->RXD.MAXCNT = size;

		I2C_REG->EVENTS_ERROR = 0x0UL;
		I2C_REG->EVENTS_STOPPED = 0x0UL;
		I2C_REG->EVENTS_LASTTX = 0X0UL;
		I2C_REG->EVENTS_LASTRX = 0X0UL;

		I2C_REG->INTENSET = TWIM_INTENSET_STOPPED_Msk | TWIM_INTENSET_LASTRX_Msk;
		I2C_REG->TASKS_STARTRX = 0x1UL;

		ret = xSemaphoreTake(xSemaphoreI2C, i2c_instance->timeout);
		xSemaphoreGive(xSemaphoreI2C);
	}

	return ret;
}


void I2C_IRQ_HANDLER(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (I2C_REG->EVENTS_LASTTX) {
		I2C_REG->TASKS_STOP = 0x1UL;
	} else if (I2C_REG->EVENTS_LASTRX) {
		I2C_REG->TASKS_STOP = 0x1UL;
	} else if (I2C_REG->EVENTS_STOPPED) {
		I2C_REG->EVENTS_STOPPED = 0x0UL;
		xSemaphoreGiveFromISR(xSemaphoreI2C, &xHigherPriorityTaskWoken);
	} else if (I2C_REG->EVENTS_ERROR) {
		
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}