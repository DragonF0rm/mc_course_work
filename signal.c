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
	TCCR0 = 0;
	bit_set(&TCCR0, CS01);
	// Enable T0 interrupts
	bit_set(&TIMSK, TOIE0);

	// Set SIG_OUT to output
	bit_set(&SIG_DIR, SIG_OUT);
	// Default output: 0
	bit_clr(&SIG_PORT, SIG_OUT);

	sig_initialized = true;

	// Enable global interrupts
	sei();
}

void
sig_parse_props(byte_t b, struct sig_props *props)
{
	// Bytes are interpreted like: 0xxx0yyy,
	//   where 0 - undef flag, xxxx - code of frequency, yyyy - code of duty cycle

	enum sig_frequency freq = SIG_FR_UNDEF;
	if (!bit_get(&b, SIG_FR_ENCODING_UNDEF_BIT)) {
		switch (b & SIG_FR_ENCODING_MASK) {
		case 0x00:
			freq = SIG_5KHZ;
			break;
		case 0x10:
			freq = SIG_10KHZ;
			break;
		case 0x20:
			freq = SIG_15KHZ;
			break;
		case 0x30:
			freq = SIG_20KHZ;
			break;
		}
	}

	enum sig_duty_cycle dc = SIG_DC_UNDEF;
	if (!bit_get(&b, SIG_DC_ENCODING_UNDEF_BIT)) {
		switch (b & SIG_DC_ENCODING_MASK) {
		case 0x00:
			dc = SIG_2DC;
			break;
		case 0x01:
			dc = SIG_4DC;
			break;
		case 0x02:
			dc = SIG_6DC;
			break;
		case 0x03:
			dc = SIG_8DC;
			break;
		}
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

static bool *sig_t0_intr_handler_done = NULL;

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

	bit_def(&SIG_PORT, SIG_OUT, task->level);

	TCNT0 = T0_MAX_TACTS - task->tacts - 1;
	sig_t0_intr_handler_done = done;
}
