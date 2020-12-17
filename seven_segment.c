#include <assert.h>
#include <stddef.h>

#include <avr/io.h>

#include "byte.h"
#include "max7219.h"

#include "seven_segment.h"

#define SEG_ADDR_FREQ_HI	MAX7219_ADDR_DIG0
#define SEG_ADDR_FREQ_LO	MAX7219_ADDR_DIG1
#define SEG_ADDR_DC_HI		MAX7219_ADDR_DIG2
#define SEG_ADDR_DC_LO		MAX7219_ADDR_DIG3

#define SEG_DATA_TO_DISPLAY_ARR_SIZE 8 /* 4 indicators + 4 addresses */

static byte_t seg_data_to_display[SEG_DATA_TO_DISPLAY_ARR_SIZE] =
	{SEG_ADDR_FREQ_HI, 0, SEG_ADDR_FREQ_LO, 0, SEG_ADDR_DC_HI, 0, SEG_ADDR_DC_LO, 0};
static size_t seg_data_to_display_i = 0;
static size_t seg_data_len = SEG_DATA_TO_DISPLAY_ARR_SIZE;
static bool *seg_display_done = NULL;

bool seg_initialized = false;

void
seg_init(void)
{
	assert(!seg_initialized);

	struct max7219_config max7219_cfg = (struct max7219_config) {
		MAX7219_ADDR_DIG3,
		MAX7219_DECODE_BITS_7_0,
		MAX7219_INTENSITY_31,
		MAX7219_DISPLAY_TEST_MODE_OFF,
	};
	max7219_init(&max7219_cfg);

	// Display initial value
	for (size_t i = 0; i < SEG_DATA_TO_DISPLAY_ARR_SIZE; i+=2) {
		max7219_write(seg_data_to_display[i], seg_data_to_display[i + 1]);
	}

	seg_initialized = true;
}

#define seg_digit_lo(_x) ((_x % 100) % 10)
#define seg_digit_hi(_x) (((_x % 100) / 10) % 10)

void
seg_display_sig_props_async(struct sig_props *props, bool *done)
{
	// Check if nodisplaying in progress
	assert(seg_display_done == NULL);

	byte_t freq_khz = byte_lo(props->freq / 1000);
	byte_t dc = byte_lo(props->dc);

	seg_data_to_display[0] = SEG_ADDR_FREQ_HI;
	seg_data_to_display[1] = seg_digit_hi(freq_khz);
	seg_data_to_display[2] = SEG_ADDR_FREQ_LO;
	seg_data_to_display[3] = seg_digit_lo(freq_khz);
	seg_data_to_display[4] = SEG_ADDR_DC_HI;
	seg_data_to_display[5] = seg_digit_hi(dc);
	seg_data_to_display[6] = SEG_ADDR_DC_LO;
	seg_data_to_display[7] = seg_digit_lo(dc);

	seg_display_done = done;
	seg_data_len = SEG_DATA_TO_DISPLAY_ARR_SIZE;
	seg_data_to_display_i = 0;

	if (props->freq == SIG_FR_UNDEF)
		seg_data_to_display_i = 4;
	if (props->dc == SIG_DC_UNDEF)
		seg_data_len = SEG_DATA_TO_DISPLAY_ARR_SIZE / 2;
}

void
seg_display_sig_props_async_continue()
{
	assert(seg_display_done);
	assert(seg_data_len <= SEG_DATA_TO_DISPLAY_ARR_SIZE);

	if (seg_data_to_display_i >= seg_data_len) {
		*seg_display_done = true;
		seg_display_done = NULL;
		return;
	}

	max7219_write(seg_data_to_display[seg_data_to_display_i],
			seg_data_to_display[seg_data_to_display_i + 1]);
	seg_data_to_display_i += 2;
}
