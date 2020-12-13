#ifndef UART_H_
#define UART_H_ 1

#include <stdbool.h>

#include "byte.h"

void
uart_init(unsigned long cpu_freq, unsigned long baud_rate);

typedef void (*uart_intr_handler_t)(void *args);

void
uart_set_rx_intr_handler(uart_intr_handler_t handler, void *args);

int
uart_read_byte(byte_t *b);

byte_t
uart_must_read_byte();

void
uart_read_byte_async(byte_t *b, bool *ready, int *err);

void
uart_must_read_byte_async(byte_t *b, bool *ready);

#endif
