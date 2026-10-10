#ifndef CAR_INTERFACE_H
#define CAR_INTERFACE_H

#include <linux/can.h>

void SetupCAN();
void register_message(int id, void (*func)(struct can_frame));

#endif