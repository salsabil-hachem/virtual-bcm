/*
 * can_socket.h
 * Envoyer / recevoir des messages CAN sous Linux (SocketCAN).
 * Sur une vraie carte, seul ce fichier serait remplace par le pilote CAN de la puce.
 */
#ifndef CAN_SOCKET_H
#define CAN_SOCKET_H

#include <stdint.h>

int can_open(const char *interface_name);      /* retourne -1 en cas d'erreur          */
int can_send(int fd, uint32_t id, const uint8_t *data, uint8_t dlc);
int can_receive(int fd, uint32_t *id, uint8_t *data, uint8_t *dlc);
                                                /* 1 = message recu, 0 = rien, -1 = erreur */

#endif /* CAN_SOCKET_H */
