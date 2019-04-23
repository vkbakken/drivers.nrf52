#include "cpu/io.h"
#include "hal/hal_spi_master.h"

#if	defined(NRF52810_H)
	#define SPI_REG				NRF_SPIM0
    #define SPI_IRQ				SPIM0_SPIS0_IRQn
    #define SPI_IRQ_HANDLER		spi0_handler
#elif defined(NRF52_H)
	#define SPI_REG				NRF_SPIM1
    #define SPI_IRQ				SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn
    #define SPI_IRQ_HANDLER		serialbox1_handler
#else
	#error "This driver only support NRF52 and NRF52810 CPU"
#endif


static SemaphoreHandle_t xSemaphoreSPI = NULL;
static StaticSemaphore_t xSemaphoreBufferSPI;

__STATIC_INLINE uint32_t hal_spi_nrfx_freq_convert(spi_frequency_t freq) {
	switch (freq) {
	case SPI_FREQ_125K:
		return SPIM_FREQUENCY_FREQUENCY_K125;
	case SPI_FREQ_250K:
		return SPIM_FREQUENCY_FREQUENCY_K250;
	case SPI_FREQ_500K:
		return SPIM_FREQUENCY_FREQUENCY_K500;
	case SPI_FREQ_1M:
		return SPIM_FREQUENCY_FREQUENCY_M1;
	case SPI_FREQ_2M:
		return SPIM_FREQUENCY_FREQUENCY_M2;
	case SPI_FREQ_4M:
		return SPIM_FREQUENCY_FREQUENCY_M4;
	case SPI_FREQ_8M:
		return SPIM_FREQUENCY_FREQUENCY_M8;
	}
}

void hal_spi_init(hal_spi_instance_t const *const spi_instance) {
	/* config pin IO used for SPI */

	/* Bind IO pin to SPI peripheral */
	SPI_REG->PSEL.SCK = spi_instance->config.pin_CLK;
	SPI_REG->PSEL.MOSI = spi_instance->config.pin_MOSI;
	SPI_REG->PSEL.MISO = spi_instance->config.pin_MISO;

	/* Config SPI peripheral */
	SPI_REG->FREQUENCY = hal_spi_nrfx_freq_convert(spi_instance->config.frequency);
	SPI_REG->CONFIG = (spi_instance->config.bit_order << SPIM_CONFIG_ORDER_Pos) |
						(spi_instance->config.clock_polarity << SPIM_CONFIG_CPOL_Pos) |
						(spi_instance->config.clock_phase << SPIM_CONFIG_CPHA_Pos);

	SPI_REG->ENABLE = (SPIM_ENABLE_ENABLE_Enabled << SPIM_ENABLE_ENABLE_Pos);
	SPI_REG->INTENSET = SPIM_INTENSET_END_Msk;

	NVIC_SetPriority(SPI_IRQ, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
	NVIC_ClearPendingIRQ(SPI_IRQ);
	NVIC_EnableIRQ(SPI_IRQ);

	xSemaphoreSPI = xSemaphoreCreateBinaryStatic(&xSemaphoreBufferSPI);
	xSemaphoreGive(xSemaphoreSPI);
}

void hal_spi_deinit(hal_spi_instance_t const *const spi_instance) {
	SPI_REG->ENABLE = (SPIM_ENABLE_ENABLE_Disabled << SPIM_ENABLE_ENABLE_Pos);

	NVIC_DisableIRQ(SPI_IRQ);
	vSemaphoreDelete(xSemaphoreSPI);
}

bool hal_spi_wr(hal_spi_instance_t const *const spi_instance, uint8_t *data_w, uint8_t size_w, uint8_t *data_r, uint8_t size_r) {
	bool ret = false;

	if (xSemaphoreTake(xSemaphoreSPI, 0) == pdTRUE) {
		SPI_REG->INTENCLR = SPIM_INTENSET_STOPPED_Msk | SPIM_INTENSET_ENDRX_Msk |
							  SPIM_INTENSET_END_Msk | SPIM_INTENSET_ENDTX_Msk |
							  SPIM_INTENSET_STARTED_Msk;
		/* select a slave and start spi transaction */
		NRF_P0->OUTCLR = 0x1UL << spi_instance->SS_pin;

		SPI_REG->TXD.PTR = (uint32_t)data_w;
		SPI_REG->TXD.MAXCNT = size_w;
		SPI_REG->RXD.PTR = (uint32_t)data_r;
		SPI_REG->RXD.MAXCNT = size_r;

		SPI_REG->EVENTS_END = 0x0UL;
		SPI_REG->INTENSET = SPIM_INTENSET_END_Msk;
		SPI_REG->TASKS_START = 0x1UL;

		ret = xSemaphoreTake(xSemaphoreSPI, spi_instance->timeout);
		xSemaphoreGive(xSemaphoreSPI);

		NRF_P0->OUTSET = 0x1UL << spi_instance->SS_pin;
	}

	return ret;
}

bool hal_spi_write(hal_spi_instance_t const *const spi_instance, uint8_t *data, uint8_t size) {
	return hal_spi_wr(spi_instance, data, size, NULL, 0);
}

bool hal_spi_read(hal_spi_instance_t const *const spi_instance, uint8_t *data, uint8_t size) {
	return hal_spi_wr(spi_instance, NULL, 0, data, size);
}

void SPI_IRQ_HANDLER(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (SPI_REG->EVENTS_END) {
		SPI_REG->EVENTS_END = 0x0UL;
		xSemaphoreGiveFromISR(xSemaphoreSPI, &xHigherPriorityTaskWoken);
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}