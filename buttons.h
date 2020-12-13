#ifndef BUTTONS_H_
#define BUTTONS_H_ 1

#include <stdbool.h>

#include "byte.h"

void
btns_init(void);

typedef void (*btns_intr_handler_t)(void *args);

void
btns_set_intr_handler(btns_intr_handler_t handler, void *args);

byte_t
btns_read_byte(void);

void
btns_read_byte_async(byte_t *b, bool *ready);

#endif
