#include "../inc/portique.h"

void *creer_post_de_controle(void *portique_p, int numPortique)
{
    portique *portique_m = (portique *)portique_p;
    conteneur grue;
    struct msqid_ds bufConteneurChargerBateau;
    struct msqid_ds bufConteneurDechargerBateau;
    int conteneursAChargerBateau;
    int conteneursADechargerBateau;
    int conteneursAChargerTrain;
    int conteneursADechargerTrain;
    int conteneursAChargerCamion;
    int conteneursADechargerCamion;

    /* Initialisation du portique : tous les emplacements sont libres et il n'y a aucun conteneur à charger/décharger */
    portique_m->trainLibre = true;
    portique_m->camionLibre = true;
    portique_m->bateauLibre = true;
    portique_m->nbConteneursAChargerBateau = 0;
    portique_m->nbConteneursADechargerBateau = 0;
    portique_m->nbConteneursAChargerTrain = 0;
    portique_m->nbConteneursADechargerTrain = 0;
    portique_m->nbConteneursAChargerCamion = 0;
    portique_m->nbConteneursADechargerCamion = 0;

    /* Initialisation des clefs des fils de messages */
    portique_m->clefConteneursAChargerBateau = 314 + numPortique;
    portique_m->clefConteneursADechargerBateau = 324 + numPortique;
    portique_m->clefConteneursAChargerTrain = 334 + numPortique;
    portique_m->clefConteneursADechargerTrain = 344 + numPortique;
    portique_m->clefConteneursAChargerCamion = 354 + numPortique;
    portique_m->clefConteneursADechargerCamion = 364 + numPortique;

    /* Creation des files d'attentes de conteneurs à charger/décharger */
    if ((conteneursAChargerBateau = msgget(portique_m->clefConteneursAChargerBateau, FLAGS)) == -1)
        erreur("Erreur : creation file charger conteneur bateau \n");
    if ((conteneursAChargerCamion = msgget(portique_m->clefConteneursAChargerCamion, FLAGS)) == -1)
        erreur("Erreur : creation file charger conteneur camion \n");
    if ((conteneursAChargerTrain = msgget(portique_m->clefConteneursAChargerTrain, FLAGS)) == -1)
        erreur("Erreur : creation file charger conteneur train\n");
    if ((conteneursADechargerBateau = msgget(portique_m->clefConteneursADechargerBateau, FLAGS)) == -1)
        erreur("Erreur : creation file decharger conteneur bateau \n");
    if ((conteneursADechargerCamion = msgget(portique_m->clefConteneursADechargerCamion, FLAGS)) == -1)
        erreur("Erreur : creation file decharger conteneur camion \n");
    if ((conteneursADechargerTrain = msgget(portique_m->clefConteneursADechargerTrain, FLAGS)) == -1)
        erreur("Erreur : creation file decharger conteneur train\n");

    while (1)
    {

        /* Si le bateau a tout charger et décharger = il est partie = on peut appeler un autre bateau */
        if (portique_m->nbConteneursAChargerBateau <= 0 && portique_m->nbConteneursADechargerBateau <= 0)
        {
            pthread_cond_signal(&portique_m->arriverBateau);
        }
        else
        {
            /* Si il reste des conteneurs a decharger dans le bateau */
            if (portique_m->nbConteneursADechargerBateau > 0)
            {
                printf("debug portique 1\n");

                if (msgrcv(conteneursADechargerBateau, &grue, sizeof(conteneur), 1, 0) == -1)
                    erreur("Erreur : pas de messages dans conteneursADecahrgerBateau");
                printf("debug portique 2\n");
                if (grue.destination == AMSTERDAM)
                {
                    portique_m->nbConteneursAChargerTrain++;
                    msgsnd(conteneursAChargerTrain, &grue, sizeof(conteneur), 0);
                    printf("Portique : conteneur %d chargé du bateau %d  au train\n", grue.idConteneur, grue.idVehicule);
                }
                else if (grue.destination == PARIS)
                {
                    portique_m->nbConteneursAChargerCamion++;
                    msgsnd(conteneursAChargerTrain, &grue, sizeof(conteneur), 0);
                    printf("Portique : conteneur %d chargé du bateau %d  au camion\n", grue.idConteneur, grue.idVehicule);
                }
                else
                    erreur("Erreur : destination conteneur = provenance conteneur");
                portique_m->nbConteneursADechargerBateau--;
                /* Temps de déplacement d'un portique */
                usleep(400000);
            }

            /* Si le bateau est plein, il peut partir */
            if (portique_m->nbConteneursAChargerBateau >= NB_MAX_CONTENEURS_BATEAU)
                pthread_cond_signal(&portique_m->partirBateau);
        }
        usleep(1000);
    }
}