#include <assert.h>

#include <avr/io.h>

#include "spi.h"

#include "max7219.h"

#define MAX7219_PORT	PORTB	/* Register to enable pull up resistors / write data */
#define MAX7219_DIR	DDRB	/* Register to specify port direction: read/write */
#define MAX7219_PIN	PINB	/* Register to read data */
#define MAX7219_WE	1	/* Pin to enable write of 7-segment indicator data */

bool max7219_initialized = false;

void
max7219_init(struct max7219_config *cfg)
{
	assert(!max7219_initialized);
	assert(cfg);

	// Configuring MAX7219_WE as output
	MAX7219_DIR  |= (1 << MAX7219_WE);
	// Set MAX7219_WE to 0
	MAX7219_PORT &= (!(1 << MAX7219_WE));

	max7219_write(MAX7219_ADDR_SCAN_LIMIT, cfg->scan_limit);
	max7219_write(MAX7219_ADDR_DECODE_MODE, cfg->decode_mode);
	max7219_write(MAX7219_ADDR_INTENSITY, cfg->intensity);
	max7219_write(MAX7219_ADDR_DISPLAY_TEST, cfg->display_test);

	max7219_write(MAX7219_ADDR_SHUTDOWN, MAX7219_DISPLAY_SHUTDOWN_MODE_OFF);

	// Set MAX7219_WE to 1
	MAX7219_PORT |= (1 << MAX7219_WE);

	max7219_initialized = true;
}

void
max7219_write(byte_t addr, byte_t data)
{
	MAX7219_PORT &= (!(1 << MAX7219_WE));

	byte_t word[2] = {addr, data};
	spi_write(word, 2);

	MAX7219_PORT |= (1 << MAX7219_WE);
}
