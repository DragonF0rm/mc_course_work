#include <avr/io.h>

void
uart_interrupt_handler(void)
{
	//TODO update settings
}

void
btns_interrupt_handler(void)
{
	//TODO update settings
}

void
make_signal(void)
{
	//TODO produce half of a signal
	//TODO delay
	//TODO produce other holf of a signal
	//TODO delay
}

int
main(void)
{
	//TODO init UART
	//TODO setup UART interrupt handler
	//TODO setup buttons interrupt handler

	int uart_ev_cnt = 0;
	int btn_ev_cnt = 0;

	//TODO start signal

	for (;;) {
		if (
	}

	return 0;
}
