#ifndef PROTO2_PINS_H_INCLUDED
#define PROTO2_PINS_H_INCLUDED

#define BOARD_BUTTON1_bp (13)
#define BOARD_BUTTON2_bp (14)
#define BOARD_BUTTON3_bp (15)
#define BOARD_BUTTON4_bp (16)

#define BOARD_LED1_bp (17)
#define BOARD_LED1_bm (1 << BOARD_LED1_bp)

#define BOARD_LED2_bp (18)
#define BOARD_LED2_bm (1 << BOARD_LED2_bp)

#define BOARD_LED3_bp (19)
#define BOARD_LED3_bm (1 << BOARD_LED3_bp)

#define BOARD_LED4_bp (20)
#define BOARD_LED4_bm (1 << BOARD_LED4_bp)

#define BOARD_NTC_PWR_bp (1)
#define BOARD_NTC_PWR_bm (1 << BOARD_NTC_PWR_bp)
#define BOARD_NTC0_SENSE_bp	(28)
#define BOARD_NTC0_SENSE_ai	(4)
#define BOARD_NTC0_SENSE_bm	(1 << BOARD_NTC0_SENSE_bp)

#define BOARD_NTC1_SENSE_bp	(4)
#define BOARD_NTC1_SENSE_ai	(2)
#define BOARD_NTC1_SENSE_bm	(1 << BOARD_NTC1_SENSE_bp)

#define BOARD_NTC2_SENSE_bp	(5)
#define BOARD_NTC2_SENSE_ai	(3)
#define BOARD_NTC2_SENSE_bm	(1 << BOARD_NTC2_SENSE_bp)

#define BOARD_ACCEL_PWR_bp (10)
#define BOARD_ACCEL_PWR_bm (1 << BOARD_ACCEL_PWR_bp)
#define BOARD_ACCEL_CS_bp (15)
#define BOARD_ACCEL_CS_bm (1 << BOARD_ACCEL_CS_bp)
#define BOARD_ACCEL_MISO_bp (13)
#define BOARD_ACCEL_MISO_bm (1 << BOARD_ACCEL_MISO_bp)
#define BOARD_ACCEL_MOSI_bp (12)
#define BOARD_ACCEL_MOSI_bm (1 << BOARD_ACCEL_MOSI_bp)
#define BOARD_ACCEL_CLK_bp (14)
#define BOARD_ACCEL_CLK_bm (1 << BOARD_ACCEL_CLK_bp)
#define BOARD_ACCEL_INT_bp (11)
#define BOARD_ACCEL_INT_bm (1 << BOARD_ACCEL_INT_bp)
#endif /*PROTO2_PINS_H_INCLUDED*/