#include <assert.h>

#include "byte.h"

bool
bit_get(const byte_t *src, byte_t bit)
{
	assert(src);
	assert(bit <= 7);

	return ((*src) & (1 << bit));
}

void
bit_set(byte_t *dest, byte_t bit)
{
	assert(dest);
	assert(bit <= 7);

	*dest |= (1 << bit);
}

void
bit_clr(byte_t *dest, byte_t bit)
{
	assert(dest);
	assert(bit <= 7);

	*dest &= (!(1 << bit));
}

void
bit_invert(byte_t *dest, byte_t bit)
{
	assert(dest);
	assert(bit <= 7);

	if (bit_get(dest, bit)) {
		bit_clr(dest, bit);
	} else {
		bit_set(dest, bit);
	}
}

void
bit_def(byte_t *dest, byte_t bit, bool val)
{
	assert(dest);
	assert(bit <= 7);

	if (val) {
		bit_set(dest, bit);
	} else {
		bit_clr(dest, bit);
	}
}

void
bit_copy(const byte_t *src, byte_t *dest, byte_t src_bit, byte_t dest_bit)
{
	assert(src);
	assert(dest);
	assert(src_bit <= 7);
	assert(dest_bit <= 7);

	bool val = bit_get(src, src_bit);
	bit_def(dest, dest_bit, val);
}
