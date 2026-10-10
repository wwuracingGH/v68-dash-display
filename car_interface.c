#include "car_interface.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can/raw.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>


#include "hashmap.h"

int scan;
hashMap_t registered_messages;

void recieveHandler();

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
}

struct can_frame readframe;

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