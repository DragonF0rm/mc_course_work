#define F_CPU		4000000L
#define BAUD_RATE	9600L

#include <stddef.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "buttons.h"
#include "seven_segment.h"
#include "spi.h"
#include "signal.h"
#include "uart.h"

int
main(void)
{
	// Internal initialization
	uart_init(F_CPU, BAUD_RATE);
	btns_init();
	sig_init(F_CPU);
	spi_init();

	// Enable global interrupts
	sei();

	// External initialization
	seg_init();

	byte_t props_byte = -1;
	byte_t new_props_byte = 0;
	struct sig_props props;
	struct sig_generator_task task_arr[SIG_GEN_TASK_ARR_MAX_LEN];
	size_t task_arr_used_len = 0;

	// Better store it in separete variables to prevent override in a middle of io
	byte_t props_displayed_byte = -1;
	struct sig_props props_displayed;
	bool display_done = true;

	byte_t uart_b = 0;
	bool   uart_ready = false;
	uart_must_read_byte_async(&uart_b, &uart_ready);

	byte_t btns_b = 0;
	bool   btns_ready = false;
	btns_read_byte_async(&btns_b, &btns_ready);

	// Event loop
	for (;;) {
		if (new_props_byte != props_byte) {
			// Period done, should update signal props and restart
			props_byte = new_props_byte;
			struct sig_props new_props;
			sig_parse_props(props_byte, &new_props);
			if (new_props.freq != SIG_FR_UNDEF)
				props.freq = new_props.freq;
			if (new_props.dc != SIG_DC_UNDEF)
				props.dc = new_props.dc;
			task_arr_used_len = sig_get_meander_generator_tasks(&props, task_arr);
			sig_generate(task_arr, task_arr_used_len);
		} else if (uart_ready) {
			// Got uart data, should save
			new_props_byte = uart_b;
			uart_ready = false;
			uart_must_read_byte_async(&uart_b, &uart_ready);
		} else if (btns_ready) {
			// Got buttons data, should save
			new_props_byte = btns_b;
			btns_ready = false;
			btns_read_byte_async(&btns_b, &btns_ready);
		} else if (props_displayed_byte != props_byte && display_done) {
			// Real props differs from displayed, should display new one
			display_done = false;
			props_displayed_byte = props_byte;
			sig_parse_props(props_displayed_byte, &props_displayed);
			seg_display_sig_props_async(&props_displayed, &display_done);
		} else if (!display_done) {
			// Display in progress, should try to continue
			seg_display_sig_props_async_continue();
		}
	}

	return 0;
}
