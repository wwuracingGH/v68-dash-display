/*
 * Based loosely on raylib template
 */
#ifndef SCREENS_H
#define SCREENS_H

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 300

extern Font font;
extern Font errfont;

void DrawDefaultScreen();
void DrawBrakeTestScreen();

#ifdef __cplusplus
}
#endif

#endif // SCREENS_H