/**
 * NICOLE SWIERSTRA
 * Can data containers for both storing and transmitting CAN messages
 */

#ifndef _CANTYPES_H_
#define _CANTYPES_H_
#include <stdint.h>


/**
 * File organization:
 * Header | std frame | checksum | std frame | checksum | ext frame | checksum | ...
 * 
 * The checksum is a 16 bit parity word
 * Basically the frame is split up into chunks and all of the chunks are XORed together
 * 
 * 8 bit example:
 * message 0b 1101 0010 0101 0010 1101 0110
 * split:              |         |         |
 * rearrange and bitwise XOR:
 *            1101 0010
 *          | 0101 0010
 *          | 1101 0110
 *            ---------
 *            0101 0110
 * 
 * then this gets appended to the end of the message
 * 
 * if the calculated checksum isn't the same, you can assume that the message is cooked
 */

/* sizes in bytes */
#define HEADERSIZE              (0x10)
#define STD_FRAME_SIZE          (sizeof(CST_CANStdID_t))
#define EXT_FRAME_SIZE          (sizeof(CST_CANExtID_t))

/* FRAME HEADER START CODE 
 * 
 * every header should start with 7 bits equal to 0b1011000 
 * 
 * if the first seven bits aren't equal to 0b1011000, you can assume the rest of your file is probably corrupt.
 * you could try doing a lookahead or a lookbehind by 2 bytes depending on how long you thought the last message was 
 */
#define CST_HEADER_START_MASK   (0xFE0) /* 0b 1111 1110 0000 */
#define CST_HEADER_START_PAT    (0xB00) /* 0b 1011 0000 0000 */

/**
 * FRAME HEADER INFORMATION 
 * the 2nd half of each header contains 3 different things
 * 
 * bits
 * [5]:   bus id
 * [4]:   extid bit
 * [0,3]: dlc
 */

/* is it on bus 1 or bus 2 (single bit) */
#define CST_HEADER_BUS_Pos      (5UL)
#define CST_HEADER_BUS_Msk      (0b1UL << CST_HEADER_BUS_Pos)

/* is the id extended (single bit) */
#define CST_HEADER_EXID_Pos     (4UL)
#define CST_HEADER_EXID_Msk     (0b1UL << CST_HEADER_EXID_Pos)

/* data length code for the can bus */
#define CST_HEADER_DLC_Pos      0UL
#define CST_HEADER_DLC_Msk      (0b1111UL << CST_HEADER_DLC_Pos)

/* length of 144 bits / 18 bytes */
typedef struct __attribute__((packed)) {
    uint64_t header : 12;
    uint64_t timestamp : 52; /* microseconds since jan 1st 1970 */
    uint16_t id;
    uint8_t data[8];
} CST_CANStdID_t;

/* length of 160 bits / 20 bytes */
typedef struct __attribute__((packed)) {
    uint64_t header : 12;
    uint64_t timestamp : 52;
    uint32_t id;
    uint8_t data[8];
} CST_CANExtID_t;


#endif