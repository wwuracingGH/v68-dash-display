#include "common.h"
#include "../screens.h"
#include "v68-vsw-definitions/errorDefinitions.h"
#include <stdlib.h>
#include <stdio.h>

char *modes[5] = {"VCU IDLE", "WAITING", "VCU RTD", "RESETING", "NO VCU"};
Color modecols[5] = {WHITE, YELLOW, RED, RED, GRAY};

Color Color_BMS_Error[3] = {     BLACK,    RED, WHITE };
Color Color_VCU_Error[3] = {  DARKBLUE,   BLUE, WHITE };
Color Color_MC_Error[3]  = {    YELLOW,  BROWN, BLACK };

#define CIRCULAR_INC(x, y) (x)++; (x) %= (y);

void printTime(double, Vector2, int, int);
void printErrors(uint64_t, uint32_t, uint16_t, int, int);
void printErrorAtPos(int idx, int scrollpixels, const char* string, Color rectColor, Color outlineColor, Color textColor);
int counterrors(uint64_t mc_errors, uint32_t bms_errors, uint16_t vcu_errors, int platch);

void printTime(double elapsedtime, Vector2 pos, int size, int has_sign) {
    char timestr[20];
    int elapsedsecs = (int)elapsedtime;
    int elapsedcsec = (int)(elapsedtime * 100) % 100;

    sprintf(timestr, has_sign ? "%+d:%02d" : "%02d:%02d", (int)(elapsedsecs/60), abs((int)(elapsedsecs%60)));
    DrawTextEx(font, timestr, pos, size, 4, WHITE);
    Vector2 txtwidth = MeasureTextEx(font, timestr, size, 4);
    Vector2 pos2 = { pos.x + 3 + txtwidth.x, pos.y };

    sprintf(timestr, "%02d", abs(elapsedcsec));
    DrawTextEx(font, timestr, pos2, size * 0.6f, 4, GRAY);
}

void printErrorAtPos(int idx, int scrollpixels, const char* string, Color rectColor, Color outlineColor, Color textColor) {
    const int box_ystart = 28;
    const int box_width = 850;
    const int box_height = 50;
    const int outline_thickness = 2;
    const int margin = 5;
    const int text_x_start = 25;
    const int text_y_offset = 6;
    const int box_x_start = 16;
    
    int box_start = (box_height + (2 * outline_thickness) + (margin)) * idx + box_ystart - scrollpixels;


    DrawRectangle(box_x_start - outline_thickness, box_start - outline_thickness, 
        box_width + (outline_thickness * 2), box_height + (outline_thickness * 2), outlineColor);
    DrawRectangle(box_x_start, box_start, box_width, 50, rectColor);
    DrawTextEx(errfont, string, (Vector2){text_x_start, box_start + text_y_offset}, 50, 1, textColor);
}

int counterrors(uint64_t mc_errors, uint32_t bms_errors, uint16_t vcu_errors, int platch) {
    return __builtin_popcount(mc_errors) + __builtin_popcount(bms_errors) + __builtin_popcount(vcu_errors) + !!platch;
}

void printErrors(uint64_t mc_errors, uint32_t bms_errors, uint16_t vcu_errors, int plausability_latch, int start_error) {
    int max_errors = counterrors(mc_errors, bms_errors, vcu_errors, plausability_latch) - 1;
    int n_error = start_error;
    for (int i = 0; i < 32; i++) {
        if (mc_errors & (1UL << i)) {
            printErrorAtPos(n_error, 0, MC_Post_Errors[i], Color_MC_Error[0], Color_MC_Error[1], Color_MC_Error[2]);
            CIRCULAR_INC(n_error, max_errors);
        }
    }

    for (int i = 0; i < 32; i++) {
        if (mc_errors & (1ULL << (i + 32))) {    
            printErrorAtPos(n_error, 0, MC_Run_Errors[i], Color_MC_Error[0], Color_MC_Error[1], Color_MC_Error[2]);
            CIRCULAR_INC(n_error, max_errors);
        }
    }

    for (int i = 0; i < 16; i++) {
        if (bms_errors & (1ULL << i)) {    
            printErrorAtPos(n_error, 0, BMS_DTC_Strings_1[i], Color_BMS_Error[0], Color_BMS_Error[1], Color_BMS_Error[2]);
            CIRCULAR_INC(n_error, max_errors);
        }
    }

    for (int i = 0; i < 16; i++) {
        if (bms_errors & (1ULL << (i + 16))) {    
            printErrorAtPos(n_error, 0, BMS_DTC_Strings_2[i], Color_BMS_Error[0], Color_BMS_Error[1], Color_BMS_Error[2]);
            CIRCULAR_INC(n_error, max_errors);
        }
    }

    for (int i = 0; i < 16; i++) {
        if (vcu_errors & APPS_FAULT_MASK & (1ULL << i)) {    
            printErrorAtPos(n_error, 0, VCU_Faults_String[i], Color_VCU_Error[0], Color_VCU_Error[1], Color_VCU_Error[2]);

            CIRCULAR_INC(n_error, max_errors);
        }
    }

    if (plausability_latch) {
        printErrorAtPos(n_error, 0, VCU_Latch_String, Color_VCU_Error[0], Color_VCU_Error[1], Color_VCU_Error[2]);
    }
}