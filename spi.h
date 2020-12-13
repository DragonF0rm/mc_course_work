#ifndef SPI_H_
#define SPI_H_ 1

#include <stdbool.h>

#include "byte.h"

void
spi_init(void);

void
spi_write_byte(byte_t b);

void
spi_write_byte_async(byte_t b);

bool
spi_write_byte_async_done();

#endif
