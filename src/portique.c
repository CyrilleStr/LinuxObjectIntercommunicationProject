#include "../inc/portique.h"

void *creer_post_de_controle(void *portique_p)
{
    portique *portique_m = (portique *)portique_p;
    struct msqid_ds buf;
    while (1)
    {
        // Si une file de conteneur est vide, le véhicule a alors tout décharger => chargement
        if (msgctl(portique_m->conteneursBateau, IPC_STAT, &buf) == -1)
            erreur("erreur : lecture file conteneurs bateau");
        if (buf.msg_qnum <= 0)
            pthread_cond_signal(&portique_m->arriverBateau);

        usleep(500000);
    }
}