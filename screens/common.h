#ifndef _SCREENS_COMMON_H_
#define _SCREENS_COMMON_H_

#include "raylib.h"
#include <stdint.h>

extern char* modes[];
extern Color modecols[];

void printTime(double, Vector2, int, int);
void printErrors(uint64_t, uint16_t, int);
void printErrorAtPos(int idx, int scrollpixels, const char* string, Color rectColor, Color outlineColor, Color textColor);

#endif
