#ifndef SIGNAL_H_
#define SIGNAL_H_ 1

#include <stdbool.h>
#include <stddef.h>

#include "byte.h"

void
sig_init(unsigned long cpu_freq);

enum sig_frequency {
	SIG_FR_UNDEF,
	SIG_5KHZ  = 5000,
	SIG_10KHZ = 10000,
	SIG_15KHZ = 15000,
	SIG_20KHZ = 20000
};

enum sig_duty_cycle {
	SIG_DC_UNDEF,
	SIG_2DC = 2,
	SIG_4DC = 4,
	SIG_6DC = 6,
	SIG_8DC = 8
};

struct sig_props {
	enum sig_frequency freq;
	enum sig_duty_cycle dc;
};

#define SIG_FR_ENCODING_MASK		0x30
#define SIG_DC_ENCODING_MASK		0x03
#define SIG_DC_ENCODING_START_BIT	0
#define SIG_FR_ENCODING_START_BIT	4
#define SIG_DC_ENCODING_UNDEF_BIT	3
#define SIG_FR_ENCODING_UNDEF_BIT	7

void
sig_parse_props(byte_t b, struct sig_props *props);

struct sig_generator_task {
	byte_t tacts;
	bool level;
};

#define SIG_GEN_TASK_ARR_MAX_LEN 256

size_t
sig_get_meander_generator_tasks(struct sig_props *props,
				struct sig_generator_task task_arr[SIG_GEN_TASK_ARR_MAX_LEN]);

void
sig_generate(struct sig_generator_task *task, size_t task_arr_len);

#endif
