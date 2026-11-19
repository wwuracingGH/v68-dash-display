#include "raylib.h"

#include <math.h>
#include <stdio.h>
#include "../screens.h"
#include "../car_state.h"
#include "common.h"
#include "VehicleWireframe.c" 

#define MINIMUM_BRAKE_SPEED_MPH     30.0
#define MPH_TO_1000_RPM             15.618
#define BAR_FULL_WIDTH              500
#define BAR_FILLED_WIDTH            800
#define BAR_FILLED_MARGIN           5
#define INVPI                       0.318309886184 

#define WHEEL_WIDTH                 (int)(0.04375 * SET_SCREEN_WIDTH)
#define WHEEL_HEIGHT                (int)(0.15 * SET_SCREEN_HEIGHT)

#define MAXIMUM_BRAKE_WHEELSPEED    2.0

#define MAX(x,y) ((x > y) ? (x) : (y))
#define MIN(x,y) ((x < y) ? (x) : (y))


Color BAR_OUTLINE_COL = {1, 5, 10, 255};

double meterspsecondtomph(double in) {
    return in * 2.237;
}

Color wheelcolor(int haspassedspeed, int lock, double wheelspeed) {
    return !haspassedspeed ? BLACK : (((wheelspeed < MAXIMUM_BRAKE_WHEELSPEED) || lock) ? GREEN : RED);
}

// Title Screen Draw logic
void DrawBrakeTestScreen()
{
    static unsigned char scroll = 0;
    if (IsKeyDown(KEY_DOWN)) scroll++;
    
    static int haspassedspeed = 0; 
    static int wheelsgreen_lock = 0;
    int vcu_state = CST_VCUState();
    double carspeed = (asin(sin(GetTime())) * INVPI) * 14 + 7; //CST_GetSpeed();
    Wheelspeeds_Groundspeed wheels = {carspeed, carspeed, carspeed, carspeed}; //CST_GetWheelspeeds();

    if (carspeed < 2.2) haspassedspeed = 0;
    if (meterspsecondtomph(carspeed) > MINIMUM_BRAKE_SPEED_MPH && !haspassedspeed) {
        haspassedspeed = 1;
        wheelsgreen_lock = 0;
    }

    /* TODO: REMOVE!!! */
    if (haspassedspeed && carspeed < 10 && carspeed > 8.0) {
        wheels.fl_wheelspeed = (carspeed - 8.0) * 0.0;
        wheels.fr_wheelspeed = (carspeed - 8.0) * 2.0;
        wheels.rl_wheelspeed = (carspeed - 8.0) * 10.0;
        wheels.rr_wheelspeed = (carspeed - 8.0) * 0.5;
    }

    if (wheels.fl_wheelspeed < MAXIMUM_BRAKE_WHEELSPEED &&
        wheels.fr_wheelspeed < MAXIMUM_BRAKE_WHEELSPEED &&
        wheels.rl_wheelspeed < MAXIMUM_BRAKE_WHEELSPEED &&
        wheels.rr_wheelspeed < MAXIMUM_BRAKE_WHEELSPEED) {

        wheelsgreen_lock = 1;
    }
    
    Color bg = (Color){0, 0, 0, 255};
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), bg);
    
    for(int i = 0; i < 48; i++) {
        int idx1 = vehiclepairs[i][0] - 1;
        int idx2 = vehiclepairs[i][1] - 1;

        Vector2 p0 = {vehiclepoints[idx1][0] * 80 + 1050, vehiclepoints[idx1][2] * 80 + 240};
        Vector2 p1 = {vehiclepoints[idx2][0] * 80 + 1050, vehiclepoints[idx2][2] * 80 + 240};

        DrawLineEx(p0, p1, 5, DARKGRAY);
    }
    
    const Rectangle wheel_fl = { 970 - (WHEEL_WIDTH / 2), 155, WHEEL_WIDTH, WHEEL_HEIGHT};
    const Rectangle wheel_fr = {1130 - (WHEEL_WIDTH / 2), 155, WHEEL_WIDTH, WHEEL_HEIGHT}; 
    const Rectangle wheel_rl = { 970 - (WHEEL_WIDTH / 2), 355, WHEEL_WIDTH, WHEEL_HEIGHT};
    const Rectangle wheel_rr = {1130 - (WHEEL_WIDTH / 2), 355, WHEEL_WIDTH, WHEEL_HEIGHT};

    DrawRectangleRounded(wheel_fl, 0.5f, 5, wheelcolor(haspassedspeed, wheelsgreen_lock, wheels.fl_wheelspeed));
    DrawRectangleRounded(wheel_fr, 0.5f, 5, wheelcolor(haspassedspeed, wheelsgreen_lock, wheels.fr_wheelspeed));
    DrawRectangleRounded(wheel_rl, 0.5f, 5, wheelcolor(haspassedspeed, wheelsgreen_lock, wheels.rl_wheelspeed));
    DrawRectangleRounded(wheel_rr, 0.5f, 5, wheelcolor(haspassedspeed, wheelsgreen_lock, wheels.rr_wheelspeed));

    DrawRectangleRoundedLinesEx(wheel_fl, 0.5f, 5, 5, WHITE);
    DrawRectangleRoundedLinesEx(wheel_fr, 0.5f, 5, 5, WHITE);
    DrawRectangleRoundedLinesEx(wheel_rl, 0.5f, 5, 5, WHITE);
    DrawRectangleRoundedLinesEx(wheel_rr, 0.5f, 5, 5, WHITE);
    
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
    DrawRectangle(30 + BAR_FILLED_MARGIN, 224 + BAR_FILLED_MARGIN, MIN(brakepercentage * BAR_FULL_WIDTH, BAR_FILLED_WIDTH - (BAR_FILLED_MARGIN * 2)), 232 - (BAR_FILLED_MARGIN * 2), haspassedspeed ? GREEN : RED);
    DrawRectangle(30 + BAR_FULL_WIDTH, 224, BAR_FILLED_MARGIN, 232, DARKGRAY);

    printErrors(CST_GetMCFaults(), CST_GetBMSFaults(), CST_GetVCUFaults(), CST_GetVCUPlausabilityLatch(), scroll);
}
