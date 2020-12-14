#include <assert.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "buttons.h"

#define BTNS_PORT	PORTD	/* Register to enable pull up resistors / write data */
#define BTNS_DIR	DDRD	/* Register to specify port direction: read/write */
#define BTNS_PIN	PIND	/* Register to read data */
#define BTNS_EN		2	/* INT0 */
#define BTNS_0		3
#define BTNS_1		4
#define BTNS_2		5
#define BTNS_3		6

#define BTNS_READY (BTNS_PIN & (1 << BTNS_EN))

static bool btns_initialized = false;

void
btns_init(void)
{
	// Disable global interrupts
	cli();

	assert(!btns_initialized);

	// Enable interrupts by INT0
	GICR |= (1 << 6);
	// INT0 interrupt is triggerred by falling edge
	MCUCR |= (1 << 1);

	// Set all BTNS pins to input
	BTNS_DIR  &= (!(1 << BTNS_EN)) &
		(!(1 << BTNS_0)) &
		(!(1 << BTNS_1)) &
		(!(1 << BTNS_2)) &
		(!(1 << BTNS_3));
	BTNS_PORT |= (1 << BTNS_EN) |
		(1 << BTNS_0) |
		(1 << BTNS_1) |
		(1 << BTNS_2) |
		(1 << BTNS_3);

	btns_initialized = true;

	// Enable global interrupts
	sei();
}

static btns_intr_handler_t btns_intr_handler = NULL;
static void *btns_intr_handler_args = NULL;

ISR(INT0_vect)
{
	if (btns_intr_handler) {
		btns_intr_handler(btns_intr_handler_args);
	}
}

void
btns_set_intr_handler(btns_intr_handler_t handler, void *args)
{
	// Disable global interrupts
	cli();

	btns_intr_handler = handler;
	btns_intr_handler_args = args;

	// Enable global interrupts
	sei();
}

byte_t
btns_read_byte(void)
{
	// Wait for user to press button
	while (!BTNS_READY);

	byte_t b = 0;
	b |= ((BTNS_PIN & (1 << BTNS_0)) << 0) |
		((BTNS_PIN & (1 << BTNS_1)) << 1) |
		((BTNS_PIN & (1 << BTNS_2)) << 2) |
		((BTNS_PIN & (1 << BTNS_3)) << 3);

	return b;
}

struct btns_read_byte_async_intr_handler_args {
	byte_t *b;
	bool *ready;
};

void
btns_read_byte_async_intr_handler(void *raw_args)
{
	assert(raw_args);

	struct btns_read_byte_async_intr_handler_args *args =
		(struct btns_read_byte_async_intr_handler_args *) raw_args;

	byte_t b = btns_read_byte();

	*(args->b) = b;
	*(args->ready) = true;

	// Clean up
	btns_intr_handler = NULL;
	btns_intr_handler_args = NULL;
}

void
btns_read_byte_async(byte_t *b, bool *ready)
{
	assert(b);
	assert(ready);

	static struct btns_read_byte_async_intr_handler_args args;
	assert(btns_intr_handler_args == NULL); // Check if async read in progress

	args = (struct btns_read_byte_async_intr_handler_args) {
		b = b,
		ready = ready,
	};

	btns_set_intr_handler(btns_read_byte_async_intr_handler, &args);
}
