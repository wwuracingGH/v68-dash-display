#include "raylib.h"

#include <math.h>
#include <stdio.h>
#include "../screens.h"
#include "../car_state.h"
#include "common.h"
#include "VehicleWireframe.c" 

#define MINIMUM_BRAKE_SPEED_MPH     30.0
#define MPH_TO_1000_RPM             15.618
#define BAR_FULL_WIDTH              (SCREEN_WIDTH  * (25.0 / 64.0))
#define BAR_FILLED_WIDTH            (SCREEN_WIDTH  * ( 5.0 /  8.0))
#define BAR_FILLED_MARGIN           (SCREEN_HEIGHT * ( 1.0 / 96.0))
#define INVPI                       0.318309886184 

#define WHEEL_WIDTH                 (int)(0.06 * SCREEN_WIDTH)
#define WHEEL_HEIGHT                (int)(0.08 * SCREEN_WIDTH)

#define VEHICLE_DISPLAY_START_X     (int)(SCREEN_WIDTH  * (13.0 / 16.0))
#define VEHICLE_DISPLAY_START_Y     (int)(SCREEN_HEIGHT * ( 1.0 /  2.0))
#define VEHICLE_DISPLAY_SCALE       (int)(SCREEN_WIDTH  * ( 1.0 / 16.0))

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

        Vector2 p0 = {vehiclepoints[idx1][0] * VEHICLE_DISPLAY_SCALE + VEHICLE_DISPLAY_START_X, 
                      vehiclepoints[idx1][2] * VEHICLE_DISPLAY_SCALE + VEHICLE_DISPLAY_START_Y};
        Vector2 p1 = {vehiclepoints[idx2][0] * VEHICLE_DISPLAY_SCALE + VEHICLE_DISPLAY_START_X, 
                      vehiclepoints[idx2][2] * VEHICLE_DISPLAY_SCALE + VEHICLE_DISPLAY_START_Y};

        DrawLineEx(p0, p1, 5, DARKGRAY);
    }

    const int wheel_x_offset    = VEHICLE_DISPLAY_SCALE * 1.2;
    const int wheel_x_left      = VEHICLE_DISPLAY_START_X - wheel_x_offset;
    const int wheel_x_right     = VEHICLE_DISPLAY_START_X + wheel_x_offset;
    const int wheel_y_top       = VEHICLE_DISPLAY_START_Y - VEHICLE_DISPLAY_SCALE * ( 0.65 );
    const int wheel_y_bottom    = VEHICLE_DISPLAY_START_Y + VEHICLE_DISPLAY_SCALE * ( 1.90 );

    const Rectangle wheel_fl = { wheel_x_left - (WHEEL_WIDTH / 2), wheel_y_top    - (WHEEL_HEIGHT / 2), WHEEL_WIDTH, WHEEL_HEIGHT};
    const Rectangle wheel_fr = {wheel_x_right - (WHEEL_WIDTH / 2), wheel_y_top    - (WHEEL_HEIGHT / 2), WHEEL_WIDTH, WHEEL_HEIGHT}; 
    const Rectangle wheel_rl = { wheel_x_left - (WHEEL_WIDTH / 2), wheel_y_bottom - (WHEEL_HEIGHT / 2), WHEEL_WIDTH, WHEEL_HEIGHT};
    const Rectangle wheel_rr = {wheel_x_right - (WHEEL_WIDTH / 2), wheel_y_bottom - (WHEEL_HEIGHT / 2), WHEEL_WIDTH, WHEEL_HEIGHT};

    DrawRectangleRounded(wheel_fl, 0.5f, 5, wheelcolor(haspassedspeed, wheelsgreen_lock, wheels.fl_wheelspeed));
    DrawRectangleRounded(wheel_fr, 0.5f, 5, wheelcolor(haspassedspeed, wheelsgreen_lock, wheels.fr_wheelspeed));
    DrawRectangleRounded(wheel_rl, 0.5f, 5, wheelcolor(haspassedspeed, wheelsgreen_lock, wheels.rl_wheelspeed));
    DrawRectangleRounded(wheel_rr, 0.5f, 5, wheelcolor(haspassedspeed, wheelsgreen_lock, wheels.rr_wheelspeed));

    DrawRectangleRoundedLinesEx(wheel_fl, 0.5f, 5, 5, WHITE);
    DrawRectangleRoundedLinesEx(wheel_fr, 0.5f, 5, 5, WHITE);
    DrawRectangleRoundedLinesEx(wheel_rl, 0.5f, 5, 5, WHITE);
    DrawRectangleRoundedLinesEx(wheel_rr, 0.5f, 5, 5, WHITE);
   
    const double left_bound     = SCREEN_WIDTH  * ( 3.0 / 128.0);
    const double text_height    = SCREEN_HEIGHT * ( 1.0 /   6.0);
    const double bar_top        = SCREEN_HEIGHT * ( 7.0 /  15.0);
    const double bar_thickness  = SCREEN_HEIGHT * ( 1.0 /   2.0);

    char speedstr[16];
    Vector2 pos4 = { left_bound, text_height };
    sprintf(speedstr, "%02.1f", meterspsecondtomph(carspeed));
    DrawTextEx(font, speedstr, pos4, font.baseSize * 0.5f, 4, WHITE);

    Vector2 pos5 = { SCREEN_WIDTH * (53.0 / 128.0), text_height };
    sprintf(speedstr, "%02.1f", MINIMUM_BRAKE_SPEED_MPH);
    DrawTextEx(font, speedstr, pos5, font.baseSize * 0.5f, 4, WHITE);

    double brakepercentage = meterspsecondtomph(carspeed) / MINIMUM_BRAKE_SPEED_MPH;
    DrawRectangle(left_bound, bar_top, BAR_FILLED_WIDTH, bar_thickness, DARKGRAY);
    DrawRectangle(left_bound + BAR_FILLED_MARGIN, bar_top + BAR_FILLED_MARGIN, BAR_FILLED_WIDTH - (BAR_FILLED_MARGIN * 2), bar_thickness - (BAR_FILLED_MARGIN * 2), BAR_OUTLINE_COL);
    DrawRectangle(left_bound + BAR_FILLED_MARGIN, bar_top + BAR_FILLED_MARGIN, MIN(brakepercentage * BAR_FULL_WIDTH, BAR_FILLED_WIDTH - (BAR_FILLED_MARGIN * 2)), bar_thickness - (BAR_FILLED_MARGIN * 2), haspassedspeed ? GREEN : RED);
    DrawRectangle(left_bound + BAR_FULL_WIDTH, bar_top, BAR_FILLED_MARGIN, bar_thickness, DARKGRAY);

    printErrors(CST_GetMCFaults(), CST_GetBMSFaults(), CST_GetVCUFaults(), CST_GetVCUPlausabilityLatch(), scroll);
}
