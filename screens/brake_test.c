#include "raylib.h"

#include <math.h>
#include "stdio.h"
#include "../screens.h"
#include "../car_state.h"
#include "common.h"

#define MINIMUM_BRAKE_SPEED_MPH     35.0
#define BAR_FULL_WIDTH              500
#define BAR_FILLED_WIDTH            800
#define BAR_FILLED_MARGIN           5
#define INVPI                       0.318309886184 

#define MAX(x,y) ((x > y) ? (x) : (y))
#define MIN(x,y) ((x < y) ? (x) : (y))


Color BAR_OUTLINE_COL = {1, 5, 10, 255};

Color frompercentage(double percentage) {
    double normalized = (MIN(MAX(percentage * percentage * percentage, 0.0), 1.0) * 0.33) * 360.0;
    return ColorFromHSV(normalized, 1.0, 1.0);
}

double meterspsecondtomph(double in) {
    return in * 2.237;
}

// Title Screen Draw logic
void DrawBrakeTestScreen()
{
    static unsigned char scroll = 0;
    if (IsKeyDown(KEY_DOWN)) scroll++;
    int vcu_state = CST_VCUState();
    int battery = CST_BatteryPercentage();
    double carspeed = (asin(sin(GetTime())) * INVPI) * 20 + 10; //CST_GetSpeed();

    Color bg = (Color){0, 0, 0, 255};
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), bg);
    Vector2 pos = { 880, 128 };
    DrawTextEx(font, "Battery", pos, font.baseSize * 0.2f, 4, WHITE);
    Vector2 pos2 = { 880, 224 };
    char batstr[10];
    sprintf(batstr, "%02d", battery);

    Vector2 pos3 = { 880, 48 };
    DrawTextEx(font, modes[vcu_state], pos3, font.baseSize*0.2f, 4, modecols[vcu_state]);

    DrawTextEx(font, batstr, pos2, font.baseSize * 1.0f, 4, battery > 15 ? WHITE : RED);

    char speedstr[16];
    Vector2 pos4 = { 30, 80 };
    sprintf(speedstr, "%02.1f", meterspsecondtomph(carspeed));
    DrawTextEx(font, speedstr, pos4, font.baseSize * 0.5f, 4, WHITE);

    Vector2 pos5 = { 530, 80 };
    sprintf(speedstr, "%02.1f", MINIMUM_BRAKE_SPEED_MPH);
    DrawTextEx(font, speedstr, pos5, font.baseSize * 0.5f, 4, WHITE);

    
    double brakepercentage = meterspsecondtomph(carspeed) / MINIMUM_BRAKE_SPEED_MPH;
    DrawRectangle(30, 224, BAR_FILLED_WIDTH, 232, DARKGRAY);
    DrawRectangle(30 + BAR_FILLED_MARGIN, 224 + BAR_FILLED_MARGIN, BAR_FILLED_WIDTH - (BAR_FILLED_MARGIN * 2), 232 - (BAR_FILLED_MARGIN * 2), BAR_OUTLINE_COL);
    DrawRectangle(30 + BAR_FILLED_MARGIN, 224 + BAR_FILLED_MARGIN, MIN(brakepercentage * BAR_FULL_WIDTH, BAR_FILLED_WIDTH - (BAR_FILLED_MARGIN * 2)), 232 - (BAR_FILLED_MARGIN * 2), frompercentage(brakepercentage));
    DrawRectangle(30 + BAR_FULL_WIDTH, 224, BAR_FILLED_MARGIN, 232, DARKGRAY);

    printErrors(CST_GetMCFaults(), CST_GetBMSFaults(), CST_GetVCUFaults(), CST_GetVCUPlausabilityLatch(), scroll);
}
