#include <assert.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "spi.h"

#define SPI_PORT	PORTB	/* Register to enable pull up resistors / write data */
#define SPI_DIR		DDRB	/* Register to specify port direction: read/write */
#define SPI_PIN		PIND	/* Register to read data */
#define SPI_SS		2	/* Pin of SPI Slave Select */
#define SPI_MOSI	3	/* Pin of SPI Master Output Slave Input */
#define SPI_MISO	4	/* Pin of SPI Master Input Slave Output */
#define SPI_SCK		5	/* Pin of SPI Serial Clock */

#define SPI_WRITE_DONE (SPSR & (1 << SPIF))

static bool spi_initialized = false;

void
spi_init(void)
{
	// Disable global interrupts
	cli();

	assert(!spi_initialized);

	// Configuring SPI_SS, SPI_MOSI and SPI_SCK as output
	SPI_DIR  |= (1 << SPI_SS)|(1 << SPI_MOSI)|(1 << SPI_SCK);
	// SPI_SS behaves as general io port, set signal to 0
	SPI_PORT &= (!(1 <<SPI_SS));
	// Endble SPI, Master
	SPCR = (1 << SPE)|(1 << MSTR);
	// Set SPI_SCK as cpu_freq/2 (fastest available)
	SPSR |= (1 << SPI2X);

	spi_initialized = true;

	// Enable global interrupts
	sei();
}

void
spi_write_byte(byte_t b)
{
	// Check if write already in progress
	assert(SPI_WRITE_DONE);

	SPDR = b;

	// Wait for SPI write to complete
	while (!SPI_WRITE_DONE);
}

void
spi_write_byte_async(byte_t b)
{
	// Check if write already in progress
	assert(SPI_WRITE_DONE);

	SPDR = b;
}

bool
spi_write_byte_async_done()
{
	return SPI_WRITE_DONE;
}
