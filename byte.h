#ifndef BYTE_H_
#define BYTE_H_ 1

#include <stdbool.h>
#include <inttypes.h>

#define NULL ((void *)0)

typedef volatile uint8_t byte_t;

#define byte_hi(_x)	((byte_t)(_x >> 8))
#define byte_lo(_x)	((byte_t)(_x & 0xFF))

bool
bit_get(const byte_t *src, byte_t bit);

void
bit_set(byte_t *dest, byte_t bit);

void
bit_clr(byte_t *dest, byte_t bit);

void
bit_invert(byte_t *dest, byte_t bit);

void
bit_def(byte_t *dest, byte_t bit, bool val);

void
bit_copy(const byte_t *src, byte_t *dest, byte_t src_bit, byte_t dest_bit);

#endif
