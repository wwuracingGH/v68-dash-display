/**
 * NICOLE SWIERSTRA
 * Can data containers for both storing and transmitting CAN messages
 */

#ifndef _CANTYPES_H_
#define _CANTYPES_H_
#include <stdint.h>

/**
 * header contains 3 different things
 * 
 * bits
 * [5,7]: bus id
 * [4]:   extid bit
 * [0,3]: dlc
 */

#define CST_HEADER_BUS_Pos      (5UL)
#define CST_HEADER_BUS_Msk      (0b111UL << CST_HEADER_BUS_Pos)

#define CST_HEADER_EXID_Pos     (4UL)
#define CST_HEADER_EXID_Msk     (0b1UL << CST_HEADER_EXID_Pos)

#define CST_HEADER_DLC_Pos      0UL
#define CST_HEADER_DLC_Msk      (0b1111UL << CST_HEADER_DLC_Pos)

typedef struct __attribute__((packed)) {
    uint64_t header : 8;
    uint64_t timestamp : 56; /* us since last storage timestamp message */
    uint16_t id;
    uint8_t data[8];
} CST_CANStdID_t;

typedef struct __attribute__((packed)) {
    uint64_t header : 8;
    uint64_t timestamp : 56;
    uint32_t id;
    uint8_t data[8];
} CST_CANExtID_t;

#endif