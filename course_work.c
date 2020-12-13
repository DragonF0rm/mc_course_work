#include <stddef.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "buttons.h"
#include "seven_segment.h"
#include "spi.h"
#include "signal.h"
#include "uart.h"

#define CW_CPU_FREQ	8000000L
#define CW_BAUD_RATE	9600L

int
main(void)
{
	uart_init(CW_CPU_FREQ, CW_BAUD_RATE);
	btns_init();
	sig_init(CW_CPU_FREQ);
	spi_init();

	// Enable global interrupts
	sei();

	// Enable change of interrupt vectors
	GICR = (1 << IVCE);
	// Move interrupts to boot Flash section
	GICR = (1 << IVSEL);

	byte_t props_byte = 0;
	struct sig_props props;
	struct sig_generator_task tasks[SIG_GEN_TASKS_CNT];
	size_t task_i = SIG_GEN_TASKS_CNT;
	bool   task_done = true;

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
		if (task_done && task_i == SIG_GEN_TASKS_CNT) {
			// Period done, should update signal props and restart
			sig_parse_props(props_byte, &props);
			sig_get_generator_tasks(&props, tasks);
			task_i = 0;
			task_done = false;
			sig_generate(&(tasks[task_i]), &task_done);
			task_i++;
		} else if (task_done) {
			// Task done, should start next
			task_done = false;
			sig_generate(&(tasks[task_i]), &task_done);
			task_i++;
		} else if (uart_ready) {
			// Got uart data, should save
			props_byte = uart_b;
			uart_ready = false;
			uart_must_read_byte_async(&uart_b, &uart_ready);
		} else if (btns_ready) {
			// Got buttons data, should save
			props_byte = btns_b;
			btns_ready = false;
			btns_read_byte_async(&btns_b, &btns_ready);
		} else if (props_displayed_byte != props_byte && display_done) {
			// Real props differs from displayed, should display new one
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
