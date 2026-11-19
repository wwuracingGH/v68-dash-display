#include "can_interface.h"
#include "stdint.h"
#include "v68-vsw-definitions/legacyCanDefinitions.h"
#include "raylib.h"
#include <stdio.h>
#include "math.h"
#include <string.h>
#include "car_state.h"

static int battery = 99;
static int vcu_state = 0;

static double vcu_timeout_start = 0;
static double vcu_timeout_time = 2.0; /* 2 seconds by default */

static double speed = 15.0;
static double accelX;
static double accelY;

struct {
    double lapstarttime;
    int laps;
    double bestlap;
    double last_delta;
    int delta_disp_timer;
    double pred_delta;
} lapTiming = {0, 0, 3599.99, 0, 0, 0.0};

static uint64_t mc_errors = 0b100010000010001;
static uint32_t bms_errors = 1 << 16 | 1 << 20;
static uint16_t vcu_flags = 1 << 14 | 1 << 15;
static uint16_t plaus_latch = 1;

void vcu_state_handler(struct can_frame);
void mc_error_handler(struct can_frame);
void bms_error_handler(struct can_frame);
void battery_handler(struct can_frame);
void time_cmd_handler(struct can_frame);
void lap(uint16_t);

void vcu_estimator_handler(struct can_frame frame) {
    uint16_t ctrl[4];
    memcpy(&ctrl, &frame.data, 8);
    accelX = (double)ctrl[0] * pow(2, -12);
    accelY = (double)ctrl[1] * pow(2, -12);
    speed = (double)ctrl[2] * 0.01;
}

void vcu_ctrl_handler(struct can_frame frame) {
    uint16_t ctrl[4];
    memcpy(&ctrl, &frame.data, 8);
    vcu_flags = ctrl[0];
}

void vcu_state_handler(struct can_frame frame) {
    VCU_VCUState state;
    memcpy(&state, &frame.data, sizeof(state));
    vcu_state = state.state;
    plaus_latch = state.pLatch;
    vcu_timeout_start = GetTime();
}

void mc_error_handler(struct can_frame frame) {
    MC_FaultCodes faults;
    memcpy(&faults, &frame.data, sizeof(faults));
    mc_errors = (uint64_t)faults.postErrors + ((uint64_t)faults.runtimeErrors << 32);
    printf("%lx\n", mc_errors);
}

void bms_error_handler(struct can_frame frame) {
    BMS_FaultsAndStatus faults;
    memcpy(&faults, &frame.data, sizeof(faults));
    bms_errors = (uint64_t)faults.DCT_status1 + ((uint64_t)faults.DCT_status2 << 16);
}

void battery_handler(struct can_frame frame) {
    battery = frame.data[0] >> 1;
}

void lap (uint16_t end_time) {
    lapTiming.last_delta = end_time - lapTiming.bestlap;
    lapTiming.delta_disp_timer = 60;
}

void time_cmd_handler(struct can_frame frame) {
    DASH_TimeCommand dc;
    memcpy(&dc, &frame.data, sizeof(dc));

    double os_time = GetTime();
    double can_time = (0.01f * dc.current_time);
    
    lapTiming.pred_delta = 0.01f * dc.pred_delta;
            
    if (frame.can_dlc == 8) {
        lapTiming.bestlap = 0.01f * dc.best_lap_time;
        lap(0.01f * dc.last_lap_time);
    }
    
    lapTiming.lapstarttime = os_time - can_time;
}


void CST_Setup(double flstarttime) {
    lapTiming.lapstarttime = flstarttime;

    register_message(VCU_CANID_VCU_STATE, vcu_state_handler);
    register_message(VCU_CANID_CTRL_VEC, vcu_ctrl_handler);
    register_message(MC_CANID_FAULTCODES, mc_error_handler);
    register_message(MC_CANID_FAULTCODES, mc_error_handler);
    register_message(261, mc_error_handler);
    register_message(0x203, bms_error_handler);
    register_message(0x205, battery_handler);
    register_message(DL_CANID_DASH_COMMAND, time_cmd_handler);
}

void CST_Update() {
    if (vcu_timeout_start + vcu_timeout_time < GetTime()) {
        vcu_state = 4;
    }
}

int CST_VCUState() {
    return vcu_state;
}

int CST_BatteryPercentage() {
    return battery;
}

double CST_GetElapsedLapTime(double time) {
    return time - lapTiming.lapstarttime;
}

double CST_GetBestLapTime() {
    return lapTiming.bestlap;
}

double CST_GetDeltaTime() {
    return lapTiming.last_delta;
}

int CST_GetLaps() {
    return lapTiming.laps;
}

uint64_t CST_GetMCFaults() {
    return mc_errors;
}
uint16_t CST_GetVCUFaults() {
    return vcu_flags;
}

uint32_t CST_GetBMSFaults() {
    return bms_errors;
}

int CST_GetVCUPlausabilityLatch() {
    return plaus_latch;
}

double CST_GetSpeed(void) {
    return speed;
}

double CST_GetAccelerationX(void) {
    return accelX;
}

double CST_GetAccelerationY(void) {
    return accelY;
}
