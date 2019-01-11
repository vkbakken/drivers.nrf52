#ifndef HAL_SPI_H_INCLUDED
#define HAL_SPI_H_INCLUDED

#include "FreeRTOS.h"
#include "semphr.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SPI_FREQ_125K = 0,
	SPI_FREQ_250K,
	SPI_FREQ_500K,
	SPI_FREQ_1M,
	SPI_FREQ_2M,
	SPI_FREQ_4M,
	SPI_FREQ_8M
} spi_frequency_t;

typedef enum {
	SPI_CLOCK_POLARITY_ActiveHigh = 0UL,
	SPI_CLOCK_POLARITY_ActiveLow
} spi_cpol_t;

typedef enum {
	SPI_CLOCK_PHASE_Leading = 0UL,
	SPI_CLOCK_PHASE_Trailing
} spi_cpha_t;

typedef enum {
	SPI_BIT_ORDER_MsbFirst = 0UL,
	SPI_BIT_ORDER_LsbFirst
} spi_bit_order_t;

typedef struct {
	const struct spi_config {
		spi_frequency_t frequency;
		spi_cpol_t clock_polarity;
		spi_cpha_t clock_phase;
		spi_bit_order_t bit_order;
	} config;
	const uint8_t SS_pin;
	uint32_t timeout;
} hal_spi_instance_t;

/**
 * @brief API to initialize SPI interface.
 * 
 */
void hal_spi_init(hal_spi_instance_t const *const spi_instance);

/**
 * @brief API to deinitialize SPI interface.
 * 
 */
void hal_spi_deinit(hal_spi_instance_t const *const spi_instance);

/**
 * @brief 
 * 
 * @param spi_instance : pointer of insctance to manage SPI interface.
 * @param data_w : pointer of data to write.
 * @param size_w : number of byte write to.
 * @param data_r : pointer of data read to.
 * @param size_r : number of byte to read.
 * @return true : successed.
 * @return false : timeout.
 */
bool hal_spi_wr(hal_spi_instance_t const *const spi_instance, uint8_t *data_w, uint8_t size_w, uint8_t *data_r, uint8_t size_r);

/**
 * @brief API to write data via SPI interface.
 * 
 * @param spi_instance : pointer of insctance to manage SPI interface.
 * @param data : pointer of data to write.
 * @param size : number of byte write to.
 * @return true : write successed.
 * @return false : write timeout.
 */
bool hal_spi_write(hal_spi_instance_t const *const spi_instance, uint8_t *data, uint8_t size);

/**
 * @brief API to read data via SPI interface.
 * 
 * @param spi_instance : pointer of insctance to manage SPI interface.
 * @param data : pointer of data read to.
 * @param size : number of byte to read.
 * @return true : read successed.
 * @return false : read timeout.
 */
bool hal_spi_read(hal_spi_instance_t const *const spi_instance, uint8_t *data, uint8_t size);

#ifdef __cplusplus
}
#endif
#endif /*HAL_SPI_H_INCLUDED*/