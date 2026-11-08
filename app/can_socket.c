/*
 * can_socket.c
 * SocketCAN : sous Linux, un bus CAN s'utilise comme une connexion reseau.
 */
#include "can_socket.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

int can_open(const char *interface_name)
{
    int fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (fd < 0)
    {
        return -1;
    }

    /* Trouver le numero de l'interface "vcan0" */
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0)
    {
        close(fd);
        return -1;
    }

    /* Se "brancher" sur cette interface */
    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(fd);
        return -1;
    }
    return fd;
}

int can_send(int fd, uint32_t id, const uint8_t *data, uint8_t dlc)
{
    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));
    frame.can_id  = id;
    frame.can_dlc = dlc;
    memcpy(frame.data, data, dlc);

    ssize_t n;
    do
    {
        n = send(fd, &frame, sizeof(frame), MSG_DONTWAIT);
    } while ((n < 0) && (errno == EINTR));   /* FreeRTOS POSIX peut interrompre l'appel : on recommence */

    return (n == (ssize_t)sizeof(frame)) ? 0 : -1;
}

int can_receive(int fd, uint32_t *id, uint8_t *data, uint8_t *dlc)
{
    struct can_frame frame;
    ssize_t n;
    do
    {
        n = recv(fd, &frame, sizeof(frame), MSG_DONTWAIT);   /* ne bloque pas */
    } while ((n < 0) && (errno == EINTR));

    if (n < 0)
    {
        return ((errno == EAGAIN) || (errno == EWOULDBLOCK)) ? 0 : -1;
    }
    *id  = frame.can_id & CAN_SFF_MASK;
    *dlc = frame.can_dlc;
    memcpy(data, frame.data, 8);
    return 1;
}
