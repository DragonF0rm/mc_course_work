#ifndef SPI_H_
#define SPI_H_ 1

#include <stdbool.h>
#include <stddef.h>

#include "byte.h"

void
spi_init(void);

void
spi_write(const byte_t *b, size_t len);

#endif
