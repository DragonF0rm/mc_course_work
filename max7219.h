#ifndef MAX7219_H_
#define MAX7219_H_ 1

#include <stdbool.h>

#include "byte.h"

enum max7219_addr {
	MAX7219_ADDR_NOOP		= 0x00,
	MAX7219_ADDR_DIG0		= 0x01,
	MAX7219_ADDR_DIG1		= 0x02,
	MAX7219_ADDR_DIG2		= 0x03,
	MAX7219_ADDR_DIG3		= 0x04,
	MAX7219_ADDR_DIG4		= 0x05,
	MAX7219_ADDR_DIG5		= 0x06,
	MAX7219_ADDR_DIG6		= 0x07,
	MAX7219_ADDR_DIG7		= 0x08,
	MAX7219_ADDR_DECODE_MODE	= 0x09,
	MAX7219_ADDR_INTENSITY		= 0x0A,
	MAX7219_ADDR_SCAN_LIMIT		= 0x0B,
	MAX7219_ADDR_SHUTDOWN		= 0x0C,
	MAX7219_ADDR_DISPLAY_TEST	= 0x0F
};

enum max7219_decode_mode {
	MAX7219_DECODE_NOTHING	= 0x00,
	MAX7219_DECODE_BITS_0_0	= 0x01,
	MAX7219_DECODE_BITS_3_0	= 0x0F,
	MAX7219_DECODE_BITS_7_0	= 0xFF
};

enum max7219_intensity {
	/* duty cycle <intensity level>/32 */
	MAX7219_INTENSITY_01	= 0x00,
	MAX7219_INTENSITY_03	= 0x01,
	MAX7219_INTENSITY_05	= 0x02,
	MAX7219_INTENSITY_07	= 0x03,
	MAX7219_INTENSITY_09	= 0x04,
	MAX7219_INTENSITY_11	= 0x05,
	MAX7219_INTENSITY_13	= 0x06,
	MAX7219_INTENSITY_15	= 0x07,
	MAX7219_INTENSITY_17	= 0x08,
	MAX7219_INTENSITY_19	= 0x09,
	MAX7219_INTENSITY_21	= 0x0A,
	MAX7219_INTENSITY_23	= 0x0B,
	MAX7219_INTENSITY_25	= 0x0C,
	MAX7219_INTENSITY_27	= 0x0D,
	MAX7219_INTENSITY_29	= 0x0E,
	MAX7219_INTENSITY_31	= 0x0F
};

enum max7219_display_test {
	MAX7219_DISPLAY_TEST_MODE_OFF	= 0x00,
	MAX7219_DISPLAY_TEST_MODE_ON	= 0x01
};

enum max7219_shutdown {
	MAX7219_DISPLAY_SHUTDOWN_MODE_ON	= 0x00,
	MAX7219_DISPLAY_SHUTDOWN_MODE_OFF	= 0x01
};

struct max7219_config {
	enum max7219_addr scan_limit;
	enum max7219_decode_mode decode_mode;
	enum max7219_intensity intensity;
	enum max7219_display_test display_test;
};

void
max7219_init(struct max7219_config *cfg);

void
max7219_write(byte_t addr, byte_t data);

#endif