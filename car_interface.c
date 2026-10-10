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

void recieveHandler();

const char* blacklist_fp = "blacklist.txt";
hashSet_t blacklist;
FILE* output = NULL;

uint32_t timecode_msg_id = 0x304; // TODO: confirm this

/**
 * Constructs the file blacklist
 */
void construct_blacklist() {
    FILE* bl = fopen(blacklist_fp, "r");

    hashSet_init(&blacklist);
    char* line = NULL;
    size_t linesize = 1024;
    
    while (getline(&line, &linesize, bl) != -1) {
        if (line != NULL)
            hashSet_put(&blacklist, (unsigned int)strtol(line, NULL, 0));
    }
}

int scan;
hashMap_t registered_messages;

void settimeandstartlogging(struct can_frame c) {
        uint64_t recievedstamp = (uint64_t*)c.data;

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
        
        if (nbytes < 0) {
                perror("can raw socket read");
                return NULL;
        }
        
        if (nbytes < sizeof(struct can_frame)) {
                fprintf(stderr, "read: incomplete CAN frame\n");
                return NULL;
        }
        
        if (nbytes == 0) return NULL;
        
        struct timeval timestamp;
        int err = ioctl(scan, SIOCGSTAMP, &timestamp);

        if (err != -1) {
               readtime = timestamp.tv_sec * 1000000 + timestamp.tv_usec;
        }

        return &readframe;
}

void recieveHandler() {
        struct can_frame* msg = GetCANMsg();
        if (msg == NULL) return;

        void (*func)(struct can_frame) = hashMap_get(&registered_messages, msg->can_id);
        if (func != NULL) {
                func(*msg);
        }
}

void register_message(int id, void (*func)(struct can_frame)) {
        hashMap_put(&registered_messages, id, func);
}