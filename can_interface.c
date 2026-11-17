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

#include "hash/hashmap.h"
#include "datalogging.h"
#include "can_interface.h"

/*
 * prototypes
 */
void recieveHandler(int);
void construct_blacklist(void);
void SetupCAN(void);

struct can_frame* GetCANMsg(void);

int scan;
hashMap_t registered_messages;
const uint32_t timecode_msg_id = 0x400; // TODO: confirm this

/* connect to all availible can busses */
void SetupCAN() {
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

void recieveHandler(int a) {
        (void)a;

        struct can_frame* msg = GetCANMsg();
        while (msg != NULL) { /* just in case there are merged signals */
                struct can_frame msgcpy = *msg;
                uint64_t timestamp = readtime;

                store_frame(msgcpy, timestamp);

                void (*func)(struct can_frame) = hashMap_get(&registered_messages, msgcpy.can_id);
                if (func != NULL) {
                        func(msgcpy);
                }
                
                msg = GetCANMsg();
        }
}

void register_message(int id, void (*func)(struct can_frame)) {
        hashMap_put(&registered_messages, id, func);
}
