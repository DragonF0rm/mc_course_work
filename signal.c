#include <assert.h>

#include <avr/interrupt.h>

#include "signal.h"

#define T0_MAX_TACTS 256

#define SIG_PORT	PORTD	/* Register to enable pull up resistors / write data */
#define SIG_DIR		DDRD	/* Register to specify port direction: read/write */
#define SIG_PIN		PIND	/* Register to read data */
#define SIG_OUT		7	/* Offset in port registers for output signal */

static bool sig_initialized = false;
static unsigned long sig_tfreq = 0; // Timer frequency

void
sig_init(unsigned long cpu_freq)
{
	// Disable global interrupts
	cli();

	assert(!sig_initialized);

	// We are using frequency divider
	sig_tfreq = cpu_freq / 8;
	// Check if we have enough tackts for sig_tfreq
	assert ((sig_tfreq / SIG_5KHZ) < T0_MAX_TACTS);

	// Set frequency divider cpu_freq / 8
	TCCR0 = (1 << CS01);
	// Enable T0 interrupts
	TIMSK |= (1 << TOIE0);

	// Set SIG_OUT to output
	SIG_DIR  |= (1 << SIG_OUT);
	// Default output: 0
	SIG_PORT &= (!(1 << SIG_OUT));

	sig_initialized = true;

	// Enable global interrupts
	sei();
}

void
sig_parse_props(byte_t b, struct sig_props *props)
{
	// Bytes are interpreted like: 0000xxyy,
	//   where 0 - garbage, xx - code of frequency, yy - code of duty cycle

	enum sig_frequency freq = SIG_5KHZ;
	byte_t freq_code = (b & ((1 << 3)|(1 << 2))) >> 2;
	switch (freq_code) {
	case 0:
		freq = SIG_5KHZ;
		break;
	case 1:
		freq = SIG_10KHZ;
		break;
	case 2:
		freq = SIG_15KHZ;
		break;
	case 3:
		freq = SIG_20KHZ;
		break;
	}

	enum sig_duty_cycle dc = SIG_2DC;
	byte_t dc_code = (b & ((1 << 1)|(1 << 0)));
	switch (dc_code) {
	case 0:
		dc_code = SIG_2DC;
		break;
	case 1:
		dc_code = SIG_4DC;
		break;
	case 2:
		dc_code = SIG_6DC;
		break;
	case 3:
		dc_code = SIG_8DC;
		break;
	}

	props->freq = freq;
	props->dc = dc;
}

void
sig_get_generator_tasks(struct sig_props *props,
			struct sig_generator_task tasks[SIG_GEN_TASKS_CNT])
{
	assert(props);
	assert(tasks);

	byte_t tacts = (byte_t) sig_tfreq / props->freq;

	tasks[0].tacts = (byte_t) tacts / 10 * props->dc;
	tasks[0].level = true;

	tasks[1].tacts = tacts - tasks[0].tacts;
	tasks[1].level = false;
}

bool *sig_t0_intr_handler_done = NULL;

ISR(TIMER0_OVF_vect)
{
	if (sig_t0_intr_handler_done) {
		*sig_t0_intr_handler_done = true;
		sig_t0_intr_handler_done = NULL;
	}
}

void
sig_generate(struct sig_generator_task *task, bool *done)
{
	assert(sig_t0_intr_handler_done == NULL);
	assert(task);
	assert(done);

	if ((SIG_PIN & (1 << SIG_OUT)) != task->level) {
		SIG_PORT ^= (1 << SIG_OUT);
	}

	TCNT0 = T0_MAX_TACTS - task->tacts - 1;
	sig_t0_intr_handler_done = done;
}
