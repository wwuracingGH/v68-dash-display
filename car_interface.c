#include "car_interface.h"

#include <sys/socket.h>
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

const char* blacklist_fp = "blacklist.txt";
hashSet_t blacklist;

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

void recieveHandler();

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


}

struct can_frame readframe;
struct uint64_t readtime;

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
        int err = ioctrl(scan, SIOCGSTAMP, &timestamp);

        if (err != -1) {
               readtime = timestamp.tv_sec * 1000000 + timestamp.tv_usec;
        }

        return &readframe;
}

void recieveHandler() {
        struct can_frame* msg = GetCANMsg();
        if (msg == NULL) return;

        printf("[%ld], %lx\n", readtime, msg->can_id);

        void (*func)(struct can_frame) = hashMap_get(&registered_messages, msg->can_id);
        if (func != NULL) {
                func(*msg);
        }
}

void register_message(int id, void (*func)(struct can_frame)) {
        hashMap_put(&registered_messages, id, func);
}