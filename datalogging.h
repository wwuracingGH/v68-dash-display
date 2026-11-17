#ifndef _DATALOGGING_H_
#define _DATALOGGING_H_
#include <stdint.h>

void settimeandstartlogging(struct can_frame);
void store_frame(struct can_frame, uint64_t);

#endif