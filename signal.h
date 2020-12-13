#ifndef SIGNAL_H_
#define SIGNAL_H_ 1

#include <stdbool.h>

#include "byte.h"

void
sig_init(unsigned long cpu_freq);

enum sig_frequency {
	SIG_5KHZ  = 5000,
	SIG_10KHZ = 10000,
	SIG_15KHZ = 15000,
	SIG_20KHZ = 20000
};

enum sig_duty_cycle {
	SIG_2DC = 2,
	SIG_4DC = 4,
	SIG_6DC = 6,
	SIG_8DC = 8
};

struct sig_props {
	enum sig_frequency freq;
	enum sig_duty_cycle dc;
};

void
sig_parse_props(byte_t b, struct sig_props *props);

struct sig_generator_task {
	byte_t tacts;
	bool level;
};

#define SIG_GEN_TASKS_CNT 2

void
sig_get_generator_tasks(struct sig_props *props,
			struct sig_generator_task tasks[SIG_GEN_TASKS_CNT]);

bool *
sig_generate(struct sig_generator_task *task);

#endif
