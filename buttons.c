#include <assert.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "signal.h"

#include "buttons.h"

#define BTNS_PORT	PORTD		/* Register to enable pull up resistors / write data */
#define BTNS_DIR	DDRD		/* Register to specify port direction: read/write */
#define BTNS_PIN	PIND		/* Register to read data */
#define BTNS_EN		(byte_t)2	/* INT0 - any button pressed */
#define BTNS_FR_LO	(byte_t)3
#define BTNS_FR_HI	(byte_t)4
#define BTNS_DC_LO	(byte_t)5
#define BTNS_DC_HI	(byte_t)6

#define BTNS_EV_PORT	PORTB		/* Register to enable pull up resistors / write data */
#define BTNS_EV_DIR	DDRB		/* Register to specify port direction: read/write */
#define BTNS_EV_PIN	PINB		/* Register to read data */
#define BTNS_EV_FR	(byte_t)0	/* Frequency button pressed */
#define BTNS_EV_DC	(byte_t)7	/* Duty cycle button pressed */

#define BTNS_READY (!(BTNS_PIN & (1 << BTNS_EN))) /* Triggered by falling edge */

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

	// Set all BTNS and BTNS_EV pins to input
	bit_clr(&BTNS_DIR, BTNS_EN);
	bit_clr(&BTNS_DIR, BTNS_FR_LO);
	bit_clr(&BTNS_DIR, BTNS_FR_HI);
	bit_clr(&BTNS_DIR, BTNS_DC_LO);
	bit_clr(&BTNS_DIR, BTNS_DC_HI);
	bit_clr(&BTNS_EV_DIR, BTNS_EV_DC);
	bit_clr(&BTNS_EV_DIR, BTNS_EV_FR);
	// Enable pull ups
	bit_set(&BTNS_PORT, BTNS_EN);
	bit_set(&BTNS_PORT, BTNS_FR_LO);
	bit_set(&BTNS_PORT, BTNS_FR_HI);
	bit_set(&BTNS_PORT, BTNS_DC_LO);
	bit_set(&BTNS_PORT, BTNS_DC_HI);
	bit_set(&BTNS_EV_PORT, BTNS_EV_DC);
	bit_set(&BTNS_EV_PORT, BTNS_EV_FR);

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

	// Buttons are 1 by default and 0 when pressed, inverting
	byte_t btns_port_snap = ~BTNS_PIN;
	byte_t btns_ev_port_snap = ~BTNS_EV_PIN;
	byte_t sig_props_code = 0;

	bit_copy(&btns_port_snap, &sig_props_code, BTNS_DC_LO, SIG_DC_ENCODING_START_BIT);
	bit_copy(&btns_port_snap, &sig_props_code, BTNS_DC_HI, SIG_DC_ENCODING_START_BIT + 1);
	bit_copy(&btns_port_snap, &sig_props_code, BTNS_FR_LO, SIG_FR_ENCODING_START_BIT);
	bit_copy(&btns_port_snap, &sig_props_code, BTNS_FR_HI, SIG_FR_ENCODING_START_BIT + 1);

	bool btns_ev_dc = bit_get(&btns_ev_port_snap, BTNS_EV_DC);
	bool btns_ev_fr = bit_get(&btns_ev_port_snap, BTNS_EV_FR);

	// Invalidating part of byte if no event got
	if (!btns_ev_dc)
		bit_set(&sig_props_code, SIG_DC_ENCODING_UNDEF_BIT);
	if (!btns_ev_fr)
		bit_set(&sig_props_code, SIG_FR_ENCODING_UNDEF_BIT);

	return sig_props_code;
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
