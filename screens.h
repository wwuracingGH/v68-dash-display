/*
 * Based loosely on raylib template
 */
#ifndef SCREENS_H
#define SCREENS_H

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

extern Font font;
extern Font errfont;

void DrawDefaultScreen(void);

#ifdef __cplusplus
}
#endif

#endif // SCREENS_H