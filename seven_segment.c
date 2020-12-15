#include <assert.h>
#include <stddef.h>

#include "byte.h"
#include "spi.h"

#include "seven_segment.h"

#define SEG_ADDR_FREQ_HI	0x01
#define SEG_ADDR_FREQ_LO	0x02
#define SEG_ADDR_DC_HI		0x03
#define SEG_ADDR_DC_LO		0x04

#define SEG_DATA_TO_DISPLAY_ARR_SIZE 8 /* 4 indicators + 4 addresses */

static byte_t seg_data_to_display[SEG_DATA_TO_DISPLAY_ARR_SIZE] =
	{SEG_ADDR_FREQ_HI, 0, SEG_ADDR_FREQ_LO, 0, SEG_ADDR_DC_HI, 0, SEG_ADDR_DC_LO, 0};
static size_t seg_data_to_display_i = 0;
static bool *seg_display_done = NULL;

void
seg_display_sig_props_async(struct sig_props *props, bool *done)
{
	// Check if nodisplaying in progress
	assert(seg_display_done == NULL);

	int freq_khz = props->freq / 1000;
	int dc = props->dc;

	seg_data_to_display[0] = SEG_ADDR_FREQ_HI;
	seg_data_to_display[1] = byte_hi(freq_khz);
	seg_data_to_display[2] = SEG_ADDR_FREQ_LO;
	seg_data_to_display[3] = byte_lo(freq_khz);
	seg_data_to_display[4] = SEG_ADDR_DC_HI;
	seg_data_to_display[5] = byte_hi(dc);
	seg_data_to_display[6] = SEG_ADDR_DC_LO;
	seg_data_to_display[7] = byte_lo(dc);

	seg_display_done = done;

	spi_write_byte_async(seg_data_to_display[0]);
	seg_data_to_display_i = 1;
}

void
seg_display_sig_props_async_continue()
{
	assert(seg_display_done);

	if (!spi_write_byte_async_done()) {
		return; // yield
	}

	if (seg_data_to_display_i >= SEG_DATA_TO_DISPLAY_ARR_SIZE) {
		*seg_display_done = true;
		seg_display_done = NULL;
		return;
	}

	spi_write_byte_async(seg_data_to_display[seg_data_to_display_i]);
	seg_data_to_display_i++;
}
