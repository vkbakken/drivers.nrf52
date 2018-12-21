#include "board/pins.h"
#include "cpu/io.h"
#include "hal/hal_spi_master.h"

void hal_spi_init(hal_spi_instance_t *spi_instance) {
	//	The SPI interface of the BMA280 is compatible with two modes, '00' and '11'.
	//	The automatic selection between [CPOL = 0 and CPHA = 0] and
	//	[CPOL = 1 and CPHA = 1] is controlled based on the value of SCK after a falling edge of CSB
	//	We will use '00' mode.
	NRF_P0->OUTCLR = BOARD_ACCEL_CLK_bm;
	NRF_P0->PIN_CNF[BOARD_ACCEL_CLK_bp] = (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
										  (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
										  (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
										  (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
										  (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NRF_P0->OUTCLR = BOARD_ACCEL_MOSI_bm;
	NRF_P0->PIN_CNF[BOARD_ACCEL_MOSI_bp] = (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
										   (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
										   (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
										   (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
										   (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NRF_P0->PIN_CNF[BOARD_ACCEL_MISO_bp] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
										   (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
										   (GPIO_PIN_CNF_PULL_Pulldown << GPIO_PIN_CNF_PULL_Pos) |
										   (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
										   (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NRF_P0->OUTCLR = 1UL << spi_instance->SS_pin;
	NRF_P0->PIN_CNF[spi_instance->SS_pin] = (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
											(GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
											(GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
											(GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
											(GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NRF_SPIM0->PSEL.SCK = BOARD_ACCEL_CLK_bp;
	NRF_SPIM0->PSEL.MOSI = BOARD_ACCEL_MOSI_bp;
	NRF_SPIM0->PSEL.MISO = BOARD_ACCEL_MISO_bp;

	NRF_SPIM0->FREQUENCY = SPIM_FREQUENCY_FREQUENCY_M1;

	uint32_t config = SPIM_CONFIG_ORDER_MsbFirst;
	config |= (SPIM_CONFIG_CPOL_ActiveHigh << SPIM_CONFIG_CPOL_Pos) |
			  (SPIM_CONFIG_CPHA_Leading << SPIM_CONFIG_CPHA_Pos);
	
    NRF_SPIM0->ENABLE = (SPIM_ENABLE_ENABLE_Enabled << SPIM_ENABLE_ENABLE_Pos);
}

void hal_spi_deinit(hal_spi_instance_t *spi_instance) {
}

bool hal_spi_write(hal_spi_instance_t *spi_instance, uint8_t *data, uint8_t size) {
}

bool hal_spi_read(hal_spi_instance_t *spi_instance, uint8_t *data, uint8_t size) {
}

bool hal_spi_rw(hal_spi_instance_t *spi_instance, uint8_t *data_w, uint8_t size_w, uint8_t *data_r, uint8_t size_r) {
}