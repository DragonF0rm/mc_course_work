#include <assert.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "uart.h"

#define UART_RX_READY (UCSRA & (1 << RXC))
#define UART_TX_READY (UCSRA & (1 << UDRE))

static uart_intr_handler_t uart_rx_intr_handler = NULL;
static void **uart_rx_intr_handler_args = NULL;

ISR(USART_RXC_vect)
{
	if (uart_rx_intr_handler) {
		uart_rx_intr_handler(*uart_rx_intr_handler_args);
	}
}

static bool uart_initialized = false;

void
uart_init(unsigned long cpu_freq, unsigned long baud_rate)
{
	// Disable global interrupts
	cli();

	assert(!uart_initialized);
	assert(cpu_freq > 0);
	assert(baud_rate > 0);

	*uart_rx_intr_handler_args = NULL;

	// Set baud rate
	unsigned long baud_driver = cpu_freq / (16 * baud_rate) - 1;
	UBRRL = byte_lo(baud_driver);
	UBRRH = byte_hi(baud_driver);

	// Enable receiver (we don't need transmitter for this project)
	UCSRB = (1 << RXEN);

	// Set frame format: 8data, 2 stop bit
	UCSRC = (1 << URSEL)|(1 << USBS)|(1 << UCSZ0)|(1 << UCSZ1);

	// UART Receive Complete interrupt enabled
	UCSRB |= (1 << RXCIE);

	uart_initialized = true;

	// Enable global interrupts
	sei();
}

void
uart_set_rx_intr_handler(uart_intr_handler_t handler, void *args)
{
	// Disable global interrupts
	cli();

	uart_rx_intr_handler = handler;
	*uart_rx_intr_handler_args = args;

	// Enable global interrupts
	sei();
}

int
uart_read_byte(byte_t *b)
{
	assert(b);

	// Wait for register to be ready
	while (!UART_RX_READY);

	if ((UCSRA & (1 << FE)) || (UCSRA & (1 << DOR)) || (UCSRA & (1 << PE))) {
		*b = UDR; // Read the UDR to clean it up
		return -1;
	}

	// Read and return received data from buffer
	*b = UDR;
	return 0;
}

byte_t
uart_must_read_byte()
{
	byte_t b = 0;
	while (uart_read_byte(&b)); // Trying to read until success
	return b;
}

struct uart_read_byte_async_intr_handler_args {
	byte_t *b;
	bool *ready;
	int *err;
};

static
void
uart_read_byte_async_intr_handler(void *raw_args)
{
	assert(raw_args);

	struct uart_read_byte_async_intr_handler_args *args =
		(struct uart_read_byte_async_intr_handler_args *) raw_args;
	bool must_read = (args->err == NULL);

	byte_t b = 0;
	int err = uart_read_byte(&b);

	*(args->b) = b;
	if (must_read) {
		*(args->err) = err;
		*(args->ready) = true;
	} else {
		*(args->ready) = (err == 0);
	}

	// Clean up
	uart_rx_intr_handler = NULL;
	*uart_rx_intr_handler_args = NULL;
}

void
uart_read_byte_async(byte_t *b, bool *ready, int *err)
{
	assert(b);
	assert(ready);

	static struct uart_read_byte_async_intr_handler_args args;
	assert(*uart_rx_intr_handler_args == NULL); // Check if async read in progress

	args = (struct uart_read_byte_async_intr_handler_args) {
		b = b,
		ready = ready,
		err = err,
	};

	uart_set_rx_intr_handler(uart_read_byte_async_intr_handler, &args);
}

void
uart_must_read_byte_async(byte_t *b, bool *ready)
{
	uart_read_byte_async(b, ready, NULL);
}
