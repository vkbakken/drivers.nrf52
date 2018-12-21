#ifndef HAL_SPI_H_INCLUDED
#define HAL_SPI_H_INCLUDED


#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hal_spi_instance{
    uint8_t SS_pin;
    uint32_t timeout;
};
typedef struct hal_spi_instance hal_spi_instance_t;


/**
 * @brief API to initialize SPI interface.
 * 
 */
void hal_spi_init(hal_spi_instance_t *spi_instance);

/**
 * @brief API to deinitialize SPI interface.
 * 
 */
void hal_spi_deinit(hal_spi_instance_t *spi_instance);

/**
 * @brief API to write data via SPI interface.
 * 
 * @param spi_instance : pointer of insctance to manage SPI interface.
 * @param data : pointer of data to write.
 * @param size : number of byte write to.
 * @return true : write successed.
 * @return false : write timeout.
 */
bool hal_spi_write(hal_spi_instance_t *spi_instance, uint8_t *data, uint8_t size);

/**
 * @brief API to read data via SPI interface.
 * 
 * @param spi_instance : pointer of insctance to manage SPI interface.
 * @param data : pointer of data read to.
 * @param size : number of byte to read.
 * @return true : read successed.
 * @return false : read timeout.
 */
bool hal_spi_read(hal_spi_instance_t *spi_instance, uint8_t *data, uint8_t size);

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
bool hal_spi_rw(hal_spi_instance_t *spi_instance, uint8_t *data_w, uint8_t size_w, uint8_t *data_r, uint8_t size_r);

#ifdef __cplusplus
}
#endif
#endif /*HAL_SPI_H_INCLUDED*/


