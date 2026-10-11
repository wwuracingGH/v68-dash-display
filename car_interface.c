#include "car_interface.h"

#include <sys/socket.h>
#include <linux/sockios.h> /* what */
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <linux/can/raw.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include "CanStorage.h"
#include "hash/hashmap.h"
#include "hash/hashset.h"

/*
 * prototypes
 */
void recieveHandler(int);
void construct_blacklist(void);
void settimeandstartlogging(struct can_frame);
void SetupCAN(void);

struct can_frame* GetCANMsg(void);

void store_frame(struct can_frame, uint64_t);

const char* blacklist_fp = "blacklist.txt";
hashSet_t blacklist;
FILE* output = NULL;

int scan;
hashMap_t registered_messages;

uint32_t timecode_msg_id = 0x304; // TODO: confirm this

/**
 * Constructs the file blacklist
 */
void construct_blacklist() {
    FILE* bl = fopen(blacklist_fp, "r");

    if (bl == NULL) {
        bl = fopen("~/v68-dash-display/blacklist.txt", "r");
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
        sprintf(filename, "/logs/%d-%d-%d_%d%d", 
                t->tm_mon + 1, 
                t->tm_mday, 
                t->tm_year + 1900, 
                t->tm_hour, 
                t->tm_min);
        
        output = fopen(filename, "wb");
}

/* connect to all availible can busses */
void SetupCAN() {
        construct_blacklist();
        
        struct sockaddr_can addr;
        hashMap_init(&registered_messages);

        scan = socket(PF_CAN, SOCK_RAW, CAN_RAW);

        addr.can_family = AF_CAN;
        addr.can_ifindex = 0;

        bind(scan, (struct sockaddr *)&addr, sizeof(addr));

        signal(SIGIO, recieveHandler);

        if (fcntl(scan, __F_SETOWN, getpid()) < 0) {
                perror("fcntl F_SETOWN");
                close(scan);
                return;
        }

        int flags = fcntl(scan, F_GETFL);
        if (fcntl(scan, F_SETFL, flags | O_ASYNC) < 0) {
                perror("fcntl F_SETFL O_ASYNC");
                close(scan);
                return;
        }

        register_message(timecode_msg_id, settimeandstartlogging);
}

struct can_frame readframe;
uint64_t readtime;

struct can_frame* GetCANMsg() {
        int nbytes = read(scan, &readframe, sizeof(struct can_frame));
        
        if (nbytes == 0) return NULL;
        
        if (nbytes < 0) {
                perror("can raw socket read");
                return NULL;
        }
        
        if ((uint64_t)nbytes < sizeof(struct can_frame)) {
                fprintf(stderr, "read: incomplete CAN frame\n");
                return NULL;
        }
        
        struct timeval timestamp;
        int err = ioctl(scan, SIOCGSTAMP, &timestamp);

        if (err != -1) {
               readtime = timestamp.tv_sec * 1000000 + timestamp.tv_usec;
        }

        return &readframe;
}

void store_frame(struct can_frame msg, uint64_t timestamp) {
        int busnum = 0;
        int dlc = msg.len8_dlc & 0xF;
        int extra_bytes = 8 - msg.len;

        if ((uint32_t)msg.can_id & 0x1FFFF800) {
                CST_CANExtID_t canstorage = {
                        busnum << CST_HEADER_BUS_Pos | 
                        1 << CST_HEADER_EXID_Pos | 
                        dlc << CST_HEADER_DLC_Pos,
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
        } else {
                CST_CANStdID_t canstorage = {
                        busnum << CST_HEADER_BUS_Pos | 
                        1 << CST_HEADER_EXID_Pos | 
                        dlc << CST_HEADER_DLC_Pos,
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
        }
}

void recieveHandler(int a) {
        (void)a;

        struct can_frame* msg = GetCANMsg();
        while (msg != NULL) { /* just in case there are merged signals */
                struct can_frame msgcpy = *msg;
                uint64_t timestamp = readtime;

                if (!hashSet_get(&blacklist, (int)msgcpy.can_id)) {
                        store_frame(msgcpy, timestamp);
                }

                void (*func)(struct can_frame) = hashMap_get(&registered_messages, msgcpy.can_id);
                if (func != NULL) {
                        func(msgcpy);
                }
                
                struct can_frame* msg = GetCANMsg();
        }
}

void register_message(int id, void (*func)(struct can_frame)) {
        hashMap_put(&registered_messages, id, func);
}
