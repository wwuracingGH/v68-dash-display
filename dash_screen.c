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

#include <string.h>
#include "screens.h"
#include "car_interface.h"
#include "canDefinitions.h"
#include "stdlib.h"
#include "stdio.h"

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------
static int framesCounter = 0;
static int finishScreen = 0;
static int battery = 99;
static char *modes[5] = {"VCU IDLE", "WAITING", "VCU RTD", "RESETING", "NO VCU"};
static Color modecols[5] = {WHITE, YELLOW, GREEN, RED, GRAY};
static int mode = 0;
static double lapstarttime = 0;
static int laps = 0;
static double bestlap = 3599.99f;
static uint64_t mc_errors = 0;
static int vcu_timeout = 0;
static double last_delta;
static int delta_disp_timer;
static double pred_delta;

void vcu_state_handler(struct can_frame);
void mc_error_handler(struct can_frame);
void battery_handler(struct can_frame);
void time_cmd_handler(struct can_frame);
void lap(uint16_t);

void printTime(double, Vector2, int, int);
void printErrors(uint64_t);

void vcu_state_handler(struct can_frame frame) {
    VCU_VCUState state;
    memcpy(&state, &frame.data, sizeof(state));
    mode = state.state;
    vcu_timeout = 120;
}

void mc_error_handler(struct can_frame frame) {
    MC_FaultCodes faults;
    memcpy(&faults, &frame.data, sizeof(faults));
    mc_errors = (uint64_t)faults.postErrors + ((uint64_t)faults.runtimeErrors << 32);
    printf("%lx\n", mc_errors);
}

void battery_handler(struct can_frame frame) {
    battery = frame.data[0] >> 1;
}

void lap (uint16_t end_time) {
    last_delta = end_time - bestlap;
    delta_disp_timer = 60;
}

void time_cmd_handler(struct can_frame frame) {
    DASH_TimeCommand dc;
    memcpy(&dc, &frame.data, sizeof(dc));

    double os_time = GetTime();
    double can_time = (0.01f * dc.current_time);
    
    pred_delta = 0.01f * dc.pred_delta;
            
    if (frame.can_dlc == 8) {
        bestlap = 0.01f * dc.best_lap_time;
        lap(0.01f * dc.last_lap_time);
    }
    
    lapstarttime = os_time - can_time;
}

// Title Screen Initialization logic
void InitDashScreen(void)
{
    // TODO: Initialize TITLE screen variables here!
    framesCounter = 0;
    lapstarttime = GetTime();
    SetupCAN();
    register_message(VCU_CANID_VCU_STATE, vcu_state_handler);
    register_message(MC_CANID_FAULTCODES, mc_error_handler);
    register_message(0x205, battery_handler);
    register_message(DL_CANID_DASH_COMMAND, time_cmd_handler);
}

// Title Screen Update logic
void UpdateDashScreen(void)
{
    // TODO: Update TITLE screen variables here!

    // Press enter or tap to change to GAMEPLAY screen
    //if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
    //{
        //finishScreen = 1;   // OPTIONS
    //    finishScreen = 2;   // GAMEPLAY
    //    PlaySound(fxCoin);
    //}

    if (IsKeyPressed(KEY_UP)) {
        battery += 3;
        if (battery > 99) battery = 99;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        battery -= 3;
        if (battery < 0) battery = 0;
    }

    if (IsKeyPressed(KEY_W)) {
        mode++;
        if (mode > 4) mode = 0;
    }

    if (IsKeyPressed(KEY_S)) {
        mode--;
        if (mode < 0) mode = 4;
    }

    if (IsKeyPressed(KEY_L)) {
        laps++;
        double elapsedtime = GetTime() - lapstarttime;
	    
        if (elapsedtime < bestlap)
		    bestlap = elapsedtime;
	    
        lapstarttime = GetTime();
        if (laps > 23) laps = 0;
    }
}

void printTime(double elapsedtime, Vector2 pos, int size, int has_sign) {
    char timestr[20];
    int elapsedsecs = (int)elapsedtime;
    int elapsedcsec = (int)(elapsedtime * 100) % 100;

    sprintf(timestr, has_sign ? "%+d:%02d" : "%02d:%02d", (int)(elapsedsecs/60), abs((int)(elapsedsecs%60)));
    DrawTextEx(font, timestr, pos, size, 4, WHITE);
    Vector2 txtwidth = MeasureTextEx(font, timestr, size, 4);
    Vector2 pos2 = { pos.x + 4 + txtwidth.x, pos.y };

    sprintf(timestr, "%02d", abs(elapsedcsec));
    DrawTextEx(font, timestr, pos2, size * 0.6f, 4, GRAY);
}

void printErrors(uint64_t errors) {
    int box_start = 20;
    for (int i = 0; i < 32; i++) {
        if (errors & (1UL << i)) {    
            DrawRectangle(8, box_start-2, 534, 34, BROWN);
            DrawRectangle(10, box_start, 530, 30, YELLOW);
            DrawTextEx(errfont, MC_Post_Errors[i], (Vector2){12, box_start+3}, 30, 1, BLACK);

            box_start += 35;
        }
        
        if (errors & (1ULL << (i + 32))) {    
            DrawRectangle(8, box_start-2, 534, 34, BROWN);
            DrawRectangle(10, box_start, 530, 30, YELLOW);
            DrawTextEx(errfont, MC_Run_Errors[i], (Vector2){12, box_start+3}, 30, 1, BLACK);

            box_start += 35;
        }
    }
}

// Title Screen Draw logic
void DrawDashScreen(void)
{
    // TODO: Draw TITLE screen here!
    Color bg = (Color){0, 0, 0, 255};
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), bg);
    Vector2 pos = { 550, 80 };
    DrawTextEx(font, "Battery", pos, font.baseSize*0.2f, 4, WHITE);
    Vector2 pos2 = {550, 140};
    char batstr[10];
    sprintf(batstr, "%02d", battery);

    DrawTextEx(font, batstr, pos2, font.baseSize * 1.0f, 4, battery > 15 ? WHITE : RED);

    if (vcu_timeout > 0) {
        vcu_timeout--;
    } else {
        mode = 4;
    }

    Vector2 pos3 = {550, 30};
    DrawTextEx(font, modes[mode], pos3, font.baseSize*0.2f, 4, modecols[mode]);

    double elapsedtime = GetTime() - lapstarttime;
    DrawTextEx(font, "Best Time", (Vector2){10, 35}, font.baseSize*0.2f, 4, WHITE);
    printTime(bestlap,    (Vector2){10, 80}, font.baseSize*0.5f, 0);
    
    if (delta_disp_timer > 0) {
        DrawTextEx(font, "Last Delta", (Vector2){10, 185}, font.baseSize*0.2f, 4, WHITE);
        if (delta_disp_timer % 15 > 6)
            printTime(last_delta, (Vector2){10, 220}, font.baseSize*0.5f, 1);
        delta_disp_timer--;
    } else {
        DrawTextEx(font, "Current Time", (Vector2){10, 185}, font.baseSize*0.2f, 4, WHITE);
        printTime(elapsedtime, (Vector2){10, 220}, font.baseSize*0.5f, 0);
    }

    DrawTextEx(font, "Lap", (Vector2){400, 35}, font.baseSize*0.2f, 4, WHITE);
    char lapstr[10];
    sprintf(lapstr, "%02d", laps);
    DrawTextEx(font, lapstr, (Vector2){400, 80}, font.baseSize*0.5f, 4, WHITE);

    printErrors(mc_errors);
}

// Title Screen Unload logic
void UnloadDashScreen(void)
{
    // TODO: Unload TITLE screen variables here!
}

// Title Screen should finish?
int FinishDashScreen(void)
{
    return finishScreen;
}
