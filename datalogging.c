#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <linux/can/raw.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "datalogging.h"
#include "CanStorage.h"
#include "hash/hashset.h"
#include "crc/crc16dnp.h"

const char* blacklist_fp = "v68-vsw-definitions/blacklist.txt";
hashSet_t blacklist;
FILE* output = NULL;

void store_frame(struct can_frame msg, uint64_t timestamp) {
        if (hashSet_get(&blacklist, (int)msg.can_id))
                return;
        int busnum = 0;
        int dlc = msg.len8_dlc & 0xF;
        int extra_bytes = 8 - msg.len;
        printf("eb: %d\n", extra_bytes);
        if ((uint32_t)msg.can_id & 0x1FFFF800) {
                CST_CANExtID_t canstorage = {
                        CST_HEADER_START_PAT |
                        busnum << CST_HEADER_BUS_Pos | 
                        1 << CST_HEADER_EXID_Pos, 
                        dlc,
                        timestamp,
                        msg.can_id,
                        { 
                                msg.data[0], 
                                msg.data[1], 
                                msg.data[2], 
                                msg.data[3], 
                                msg.data[4], 
                                msg.data[5], 
                                msg.data[6], 
                                msg.data[7], 
                        },
                };
                fwrite(&canstorage, sizeof(CST_CANExtID_t) - extra_bytes, 1, output);

                uint16_t crc = crc16dnp_byte(0, &canstorage, sizeof(CST_CANExtID_t) - extra_bytes);
                fwrite(&crc, sizeof(uint16_t), 1, output);
        } else {
                CST_CANStdID_t canstorage = {
                        CST_HEADER_START_PAT |
                        busnum << CST_HEADER_BUS_Pos | 
                        0 << CST_HEADER_EXID_Pos,
                        dlc,
                        timestamp,
                        msg.can_id,
                        {
                                msg.data[0], 
                                msg.data[1], 
                                msg.data[2], 
                                msg.data[3], 
                                msg.data[4], 
                                msg.data[5], 
                                msg.data[6], 
                                msg.data[7], 
                        },
                };
                fwrite(&canstorage, sizeof(CST_CANStdID_t) - extra_bytes, 1, output);
                
                uint16_t crc = crc16dnp_byte(0, &canstorage, sizeof(CST_CANStdID_t) - extra_bytes);
                fwrite(&crc, sizeof(uint16_t), 1, output);
        }
}

/**
 * Constructs the file blacklist
 */
void construct_blacklist() {
    FILE* bl = fopen(blacklist_fp, "r");

    if (bl == NULL) {
        fprintf(stderr, "BLACKLIST FILE AT [%s] NOT FOUND!!!\n", blacklist_fp);
        return;
    }

    hashSet_init(&blacklist);
    char* line = NULL;
    size_t linesize = 1024;
    
    while (getline(&line, &linesize, bl) != -1) {
        if (line != NULL)
            hashSet_put(&blacklist, (unsigned int)strtol(line, NULL, 0));
    }
}

void settimeandstartlogging(struct can_frame c) {
        if (output != NULL) 
                return;

        uint64_t recievedstamp = *(uint64_t*)c.data;

        struct timeval toset;
        toset.tv_sec = recievedstamp / 1000000;
        toset.tv_usec = recievedstamp % 1000000;
        settimeofday(&toset, NULL);

        struct tm *t = localtime(&toset.tv_sec);

        char filename[128];
        sprintf(filename, "/logs/%d-%d-%d_%02d%02d", 
                t->tm_mon + 1, 
                t->tm_mday, 
                t->tm_year + 1900, 
                t->tm_hour, 
                t->tm_min);
        
        output = fopen(filename, "wb");
}
