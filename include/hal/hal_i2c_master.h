#ifndef HAL_I2C_H_INCLUDED
#define HAL_I2C_H_INCLUDED


#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    I2C_FREQ_100K = 0,
    I2C_FREQ_250K,
    I2C_FREQ_400K
} i2c_frequency_t;


typedef struct hal_i2c_instance{
    const struct i2c_config{
		i2c_frequency_t frequency;
		uint8_t pin_SCL;
		uint8_t pin_SDA;
	}config;
	const uint8_t slave_address;
    uint32_t timeout;
}hal_i2c_instance_t;


/**
 * @brief API to initialize i2c interface.
 * 
 */
void hal_i2c_init(hal_i2c_instance_t const *const i2c_instance);

/**
 * @brief API to deinitialize i2c interface.
 * 
 */
void hal_i2c_deinit(void);

/**
 * @brief API to write data via i2c interface.
 * 
 * @param i2c_instance : pointer of insctance to manage i2c interface.
 * @param data : pointer of data to write.
 * @param size : number of byte write to.
 * @return true : write successed.
 * @return false : write timeout.
 */
bool hal_i2c_write(hal_i2c_instance_t const *const i2c_instance, uint8_t *data, uint8_t size);

/**
 * @brief API to read data via i2c interface.
 * 
 * @param i2c_instance : pointer of insctance to manage i2c interface.
 * @param data : pointer of data read to.
 * @param size : number of byte to read.
 * @return true : read successed.
 * @return false : read timeout.
 */
bool hal_i2c_read(hal_i2c_instance_t const *const i2c_instance, uint8_t *data, uint8_t size);

#ifdef __cplusplus
}
#endif
#endif /*HAL_I2C_H_INCLUDED*/


