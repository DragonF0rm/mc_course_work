#ifndef SEVEN_SEGMENT_H_
#define SEVEN_SEGMENT_H_ 1

#include <stdbool.h>

#include "signal.h"

void
seg_display_sig_props_async(struct sig_props *props, bool *done);

void
seg_display_sig_props_async_continue();

#endif
