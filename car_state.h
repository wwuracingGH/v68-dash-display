#ifndef _LAPTIMING_H_
#define _LAPTIMING_H_
#include <stdint.h>

void CST_Setup(double firstlapstarttime);

void CST_Update(void);
int CST_VCUState(void);
int CST_BatteryPercentage(void);

int CST_GetLaps(void);
double CST_GetElapsedLapTime(double time);
double CST_GetBestLapTime(void);
double CST_GetDeltaTime(void);

uint64_t CST_GetMCFaults(void);
uint16_t CST_GetVCUFaults(void);
uint32_t CST_GetBMSFaults(void);
int CST_GetVCUPlausabilityLatch(void);

double CST_GetSpeed(void);
double CST_GetAccelerationX(void);
double CST_GetAccelerationY(void);

#endif