#include "cpu/io.h"
#include "hal/hal_spi_master.h"

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
	NRF_SPIM1->PSEL.SCK = spi_instance->config.pin_CLK;
	NRF_SPIM1->PSEL.MOSI = spi_instance->config.pin_MOSI;
	NRF_SPIM1->PSEL.MISO = spi_instance->config.pin_MISO;

	/* Config SPI peripheral */
	NRF_SPIM1->FREQUENCY = hal_spi_nrfx_freq_convert(spi_instance->config.frequency);
	NRF_SPIM1->CONFIG = (spi_instance->config.bit_order << SPIM_CONFIG_ORDER_Pos) |
						(spi_instance->config.clock_polarity << SPIM_CONFIG_CPOL_Pos) |
						(spi_instance->config.clock_phase << SPIM_CONFIG_CPHA_Pos);

	NRF_SPIM1->ENABLE = (SPIM_ENABLE_ENABLE_Enabled << SPIM_ENABLE_ENABLE_Pos);
	NRF_SPIM1->INTENSET = SPIM_INTENSET_END_Msk;

	NVIC_SetPriority(SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
	NVIC_ClearPendingIRQ(SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn);
	NVIC_EnableIRQ(SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn);

	xSemaphoreSPI = xSemaphoreCreateBinaryStatic(&xSemaphoreBufferSPI);
	xSemaphoreGive(xSemaphoreSPI);
}

void hal_spi_deinit(hal_spi_instance_t const *const spi_instance) {
	NRF_SPIM1->ENABLE = (SPIM_ENABLE_ENABLE_Disabled << SPIM_ENABLE_ENABLE_Pos);

	NVIC_DisableIRQ(SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn);
	vSemaphoreDelete(xSemaphoreSPI);
}

bool hal_spi_wr(hal_spi_instance_t const *const spi_instance, uint8_t *data_w, uint8_t size_w, uint8_t *data_r, uint8_t size_r) {
	bool ret = false;

	if (xSemaphoreTake(xSemaphoreSPI, 0) == pdTRUE) {
		NRF_SPIM1->INTENCLR = SPIM_INTENSET_STOPPED_Msk | SPIM_INTENSET_ENDRX_Msk |
							  SPIM_INTENSET_END_Msk | SPIM_INTENSET_ENDTX_Msk |
							  SPIM_INTENSET_STARTED_Msk;
		/* select a slave and start spi transaction */
		NRF_P0->OUTCLR = 0x1UL << spi_instance->SS_pin;

		NRF_SPIM1->TXD.PTR = (uint32_t)data_w;
		NRF_SPIM1->TXD.MAXCNT = size_w;
		NRF_SPIM1->RXD.PTR = (uint32_t)data_r;
		NRF_SPIM1->RXD.MAXCNT = size_r;

		NRF_SPIM1->EVENTS_END = 0x0UL;
		NRF_SPIM1->INTENSET = SPIM_INTENSET_END_Msk;
		NRF_SPIM1->TASKS_START = 0x1UL;

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

void serialbox1_handler(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (NRF_SPIM1->EVENTS_END) {
		NRF_SPIM1->EVENTS_END = 0x0UL;
		xSemaphoreGiveFromISR(xSemaphoreSPI, &xHigherPriorityTaskWoken);
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}