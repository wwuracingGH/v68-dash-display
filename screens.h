/*
 * Based loosely on raylib template
 */
#ifndef SCREENS_H
#define SCREENS_H

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

#define SET_SCREEN_WIDTH 1280
#define SET_SCREEN_HEIGHT 480

extern Font font;
extern Font errfont;

void DrawDefaultScreen();
void DrawBrakeTestScreen();

#ifdef __cplusplus
}
#endif

#endif // SCREENS_H