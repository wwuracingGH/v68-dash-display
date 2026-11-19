/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Title Screen Functions Definitions (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014-2022 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "raylib.h"

#include "stdio.h"
#include "../screens.h"
#include "../car_state.h"
#include "common.h"

// Title Screen Draw logic
void DrawDefaultScreen()
{
    static unsigned char scroll = 0;
    if (IsKeyDown(KEY_DOWN)) scroll++;

    int vcu_state = CST_VCUState();
    int battery = CST_BatteryPercentage();
    double elapsedlaptime = CST_GetElapsedLapTime(GetTime());
    double bestlap = CST_GetBestLapTime();

    /* how long to blink the delta */
    const double time_to_blink_delta = 2.0f;
    const double delta_blink_period = 0.25f;

    const double xstart1 = 0.0125 * SCREEN_WIDTH;
    const double xstart2 = 0.5 * SCREEN_WIDTH;
    const double xstart3 = (0.6875 * SCREEN_WIDTH);

    const double ystart1 = SCREEN_HEIGHT * ( 4.0 / 15.0);
    const double ystart2 = SCREEN_HEIGHT * ( 7.0 / 60.0);
    const double ystart3 = SCREEN_HEIGHT * ( 7.0 / 15.0);
    const double ystart4 = SCREEN_HEIGHT * ( 1.0 / 10.0);
    const double ystart5 = SCREEN_HEIGHT * (37.0 / 60.0);
    const double ystart6 = SCREEN_HEIGHT * (11.0 / 15.0);

    Color bg = (Color){0, 0, 0, 255};
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), bg);
    const Vector2 pos = { xstart3, ystart1 };
    DrawTextEx(font, "Battery", pos, font.baseSize*0.2f, 4, WHITE);
    const Vector2 pos2 = { xstart3, ystart3 };
    char batstr[10];
    sprintf(batstr, "%02d", battery);

    DrawTextEx(font, batstr, pos2, font.baseSize * 1.0f, 4, battery > 15 ? WHITE : RED);

    Vector2 pos3 = { xstart3, ystart4 };
    DrawTextEx(font, modes[vcu_state], pos3, font.baseSize*0.2f, 4, modecols[vcu_state]);

    DrawTextEx(font, "Best Time", (Vector2){ xstart1, ystart2 }, font.baseSize*0.2f, 4, WHITE);
    printTime(bestlap, (Vector2){ xstart1, ystart1 }, font.baseSize*0.5f, 0);
    
    if (elapsedlaptime < time_to_blink_delta) {
        DrawTextEx(font, "Last Delta", (Vector2){ xstart1, ystart5 }, font.baseSize*0.2f, 4, WHITE);
        if ((int)(elapsedlaptime / delta_blink_period) % 2)
            printTime(CST_GetDeltaTime(), (Vector2){ xstart1, ystart6 }, font.baseSize*0.5f, 1);
    } else {
        DrawTextEx(font, "Current Time", (Vector2){ xstart1, ystart5}, font.baseSize*0.2f, 4, WHITE);
        printTime(elapsedlaptime, (Vector2){ xstart1, 352}, font.baseSize*0.5f, 0);
    }

    DrawTextEx(font, "Lap", (Vector2){xstart2, ystart2}, font.baseSize*0.2f, 4, WHITE);
    char lapstr[10];
    sprintf(lapstr, "%02d", CST_GetLaps());
    DrawTextEx(font, lapstr, (Vector2){xstart2, ystart1}, font.baseSize*0.5f, 4, WHITE);

    printErrors(CST_GetMCFaults(), CST_GetBMSFaults(), CST_GetVCUFaults(), CST_GetVCUPlausabilityLatch(), scroll);
}
