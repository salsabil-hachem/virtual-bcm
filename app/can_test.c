/*
 * can_test.c
 * Petit programme pour tester le branchement au reseau CAN :
 *   1. il ENVOIE un message BCM_Lamps (lampe gauche allumee)
 *   2. il ECOUTE pendant 20 secondes et affiche les messages recus
 *      (et traduit DASH_Inputs grace a bcm_comm)
 */
#include <stdio.h>
#include <unistd.h>

#include "can_socket.h"
#include "bcm_comm.h"

int main(void)
{
    /* 1. Se brancher sur vcan0 */
    const int fd = can_open("vcan0");
    if (fd < 0)
    {
        printf("Erreur : impossible d'ouvrir vcan0. Lance d'abord : ./scripts/setup_vcan.sh\n");
        return 1;
    }
    printf("Branche sur vcan0.\n");

    /* 2. Envoyer un message : 200#01000000 = lampe gauche allumee */
    const uint8_t lamps[4] = { 1U, 0U, 0U, 0U };
    if (can_send(fd, CAN_ID_BCM_LAMPS, lamps, 4U) == 0)
    {
        printf("Envoye : ID=0x200 [4] 01 00 00 00  (BCM_Lamps : lampe gauche ON)\n");
    }

    /* 3. Ecouter pendant 20 secondes (2000 x 10 ms) */
    printf("J'ecoute pendant 20 s... envoie un message avec cansend dans une autre fenetre.\n");
    bcm_comm_t comm;
    comm_init(&comm);

    for (int i = 0; i < 2000; i++)
    {
        uint32_t id;
        uint8_t  data[8];
        uint8_t  dlc;
        while (can_receive(fd, &id, data, &dlc) == 1)
        {
            printf("Recu   : ID=0x%03X [%u]", (unsigned int)id, (unsigned int)dlc);
            for (uint8_t k = 0U; k < dlc; k++)
            {
                printf(" %02X", (unsigned int)data[k]);
            }

            if (comm_receive(&comm, id, data, dlc, 0U) && (id == CAN_ID_DASH_INPUTS))
            {
                static const char *const lever_names[] = { "milieu", "GAUCHE", "DROITE" };
                printf("  -> levier=%s, warning=%s, contact=%s",
                       lever_names[comm.inputs.lever],
                       comm.inputs.hazard ? "OUI" : "non",
                       comm.inputs.ignition_on ? "ON" : "off");
            }
            printf("\n");
            fflush(stdout);
        }
        usleep(10000);   /* attendre 10 ms */
    }

    close(fd);
    printf("Fin du test.\n");
    return 0;
}
