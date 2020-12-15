#include <assert.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "spi.h"

#define SPI_PORT	PORTB	/* Register to enable pull up resistors / write data */
#define SPI_DIR		DDRB	/* Register to specify port direction: read/write */
#define SPI_PIN		PINB	/* Register to read data */
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
	SPI_PORT &= (!(1 << SPI_SS));
	// Endble SPI, Master, frequency divider F_CPU/16
	SPCR = (1 << SPE)|(1 << MSTR)|(1 << SPR0);

	spi_initialized = true;

	// Enable global interrupts
	sei();
}

void
spi_write(const byte_t *b, size_t len)
{
	assert(b);

	for (size_t i = 0; i < len; i++) {
		SPDR = b[i];
		while (!SPI_WRITE_DONE); // Wait for SPI write to complete
	}
}
