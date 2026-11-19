/*
 * Based loosely on raylib template
 */
#include "raylib.h"
#include "screens.h"
#include "can_interface.h"
#include "car_state.h"
#include "datalogging.h"

#define NUM_SCREENS 2

Font font = { 0 };
Font errfont = { 0 };

static void UpdateDrawFrame(void);

static const int screenWidth = 1280;
static const int screenHeight = 480;

int current_screen = 0;
void (*screendrawfunc[NUM_SCREENS])() = {DrawDefaultScreen, DrawBrakeTestScreen};

int main(void)
{
    InitWindow(screenWidth, screenHeight, "v68-dash-display");

    SetupCAN();
    CST_Setup(GetTime());
    construct_blacklist();
    InitAudioDevice();      /* maybe if we need audio ? unlikely tho */

    /* these fonts come with the raspberry pi */
    font = LoadFontEx("fonts/NimbusSans-Bold.otf", 300, 0, 250);
    errfont = LoadFontEx("fonts/NimbusSansNarrow-Bold.otf", 48, 0, 250);

    SetTargetFPS(60); /* the dash display doesn't run over 60 fps so it's kinda decided for us */ 

    /* set to escape key by default. */
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }

    UnloadFont(font);
    UnloadFont(errfont);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

// Update and draw game frame
static void UpdateDrawFrame(void)
{
    CST_Update(); /* update carstate timers */
    if (IsKeyDown(KEY_RIGHT)) current_screen++;
    
    if (current_screen == -1) current_screen = NUM_SCREENS - 1;
    if (current_screen == NUM_SCREENS) current_screen = 0;

    BeginDrawing();
        ClearBackground(RAYWHITE);
        screendrawfunc[current_screen]();    
    EndDrawing();
    //----------------------------------------------------------------------------------
}
