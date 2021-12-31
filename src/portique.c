#include "../inc/portique.h"

void *creer_post_de_controle(void *portique_p)
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
    portique_m->nbConteneursChargesBateau = 0;
    portique_m->nbConteneursADechargerBateau = 0;
    portique_m->nbConteneursChargesTrain = 0;
    portique_m->nbConteneursADechargerTrain = 0;
    portique_m->nbConteneursChargesCamion = 0;
    portique_m->nbConteneursADechargerCamion = 0;
    portique_m->idBateauAQuai = -1;
    portique_m->idTrainAQuai = -1;
    portique_m->idCamionAQuai = -1;

    /* Initialisation des clefs des fils de messages */
    portique_m->clefConteneursAChargerBateau = 314 + portique_m->numPortique;
    portique_m->clefConteneursADechargerBateau = 324 + portique_m->numPortique;
    portique_m->clefConteneursAChargerTrain = 334 + portique_m->numPortique;
    portique_m->clefConteneursADechargerTrain = 344 + portique_m->numPortique;
    portique_m->clefConteneursAChargerCamion = 354 + portique_m->numPortique;
    portique_m->clefConteneursADechargerCamion = 364 + portique_m->numPortique;

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
        // pthread_mutex_lock(&portique_m->mutex);
        // printf("portique %d : %d\n", portique_m->numPortique, portique_m->nbConteneursADechargerBateau);
        /* Bateau */
        if (portique_m->bateauLibre)
        {
            pthread_cond_signal(&portique_m->arriverBateau);
        }
        else
        {
            /* Si il reste des conteneurs a decharger dans le bateau */
            if (portique_m->nbConteneursADechargerBateau > 0)
            {

                if (msgrcv(conteneursADechargerBateau, &grue, sizeof(conteneur), 1, 0) == -1)
                    erreur("Erreur : lecture conteneursADecahrgerBateau");

                if (grue.destination == AMSTERDAM && !portique_m->trainLibre && portique_m->nbConteneursChargesTrain < NB_MAX_CONTENEURS_TRAIN)
                {
                    P(portique_m->semid, TRAIN);
                    portique_m->nbConteneursChargesTrain++;
                    V(portique_m->semid, TRAIN);
                    msgsnd(conteneursAChargerTrain, &grue, sizeof(conteneur), 0);
                    printf("Portique %d: chargement du conteneur %d du bateau %d sur le train %d\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule, portique_m->idTrainAQuai);
                }
                else if (grue.destination == PARIS && !portique_m->camionLibre && portique_m->nbConteneursChargesCamion < NB_MAX_CONTENEURS_CAMION)
                {
                    P(portique_m->semid, CAMION);
                    portique_m->nbConteneursChargesCamion++;
                    V(portique_m->semid, CAMION);
                    msgsnd(conteneursAChargerCamion, &grue, sizeof(conteneur), 0);
                    printf("Portique %d: chargement du conteneur %d du bateau %d sur le camion %d\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule, portique_m->idCamionAQuai);
                }
                else
                {
                    /* Il n'y a pas de camion ni de train pour charger ce conteneur, on le remet dans la file de msg */
                    msgsnd(conteneursADechargerBateau, &grue, sizeof(conteneur), 0);
                    portique_m->nbConteneursADechargerBateau++;
                };
                portique_m->nbConteneursADechargerBateau--;

                usleep(TEMPS_MANOEUVRE_PORTIQUE);
            }

            /* Si le bateau est plein, il peut partir */
            if (portique_m->nbConteneursChargesBateau >= NB_MAX_CONTENEURS_BATEAU && portique_m->nbConteneursADechargerBateau <= 0)
            {
                pthread_cond_signal(&portique_m->partirBateau);
            }
        }

        /* Train */
        if (portique_m->trainLibre)
        {
            pthread_cond_signal(&portique_m->arriverTrain);
        }
        else
        {
            /* Si il reste des conteneurs a decharger dans le train */
            if (portique_m->nbConteneursADechargerTrain > 0)
            {

                if (msgrcv(conteneursADechargerTrain, &grue, sizeof(conteneur), 1, 0) == -1)
                    erreur("Erreur : lecture conteneursADecahrgerTrain");

                if (grue.destination == NEWYORK && !portique_m->bateauLibre && portique_m->nbConteneursChargesBateau < NB_MAX_CONTENEURS_BATEAU)
                {
                    P(portique_m->semid, BATEAU);
                    portique_m->nbConteneursChargesBateau++;
                    V(portique_m->semid, BATEAU);
                    msgsnd(conteneursAChargerBateau, &grue, sizeof(conteneur), 0);
                    printf("Portique %d: chargement du conteneur %d du train %d sur le bateau %d\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule, portique_m->idBateauAQuai);
                }
                else if (grue.destination == PARIS && !portique_m->camionLibre && portique_m->nbConteneursChargesCamion < NB_MAX_CONTENEURS_CAMION)
                {
                    P(portique_m->semid, CAMION);
                    portique_m->nbConteneursChargesCamion++;
                    V(portique_m->semid, CAMION);
                    msgsnd(conteneursAChargerCamion, &grue, sizeof(conteneur), 0);
                    printf("Portique %d: chargement du conteneur %d du train %d sur le camion %d\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule, portique_m->idCamionAQuai);
                }
                else
                {
                    /* Il n'y a pas de bateau ni de camion pour charger ce conteneur, on le remet dans la file de msg */
                    msgsnd(conteneursADechargerTrain, &grue, sizeof(conteneur), 0);
                    portique_m->nbConteneursADechargerTrain++;
                };
                portique_m->nbConteneursADechargerTrain--;

                usleep(TEMPS_MANOEUVRE_PORTIQUE);
            }

            // printf("debug train %d: nbConteneursChargesTrain %d\n", portique_m->idTrainAQuai, portique_m->nbConteneursChargesTrain);
            /* Si le train est plein et qu'il a tout déchargé, il peut partir */
            if (portique_m->nbConteneursChargesTrain >= NB_MAX_CONTENEURS_TRAIN && portique_m->nbConteneursADechargerTrain <= 0)
            {
                pthread_cond_signal(&portique_m->partirTrain);
            }
        }

        /* Camion */
        if (portique_m->camionLibre)
        {
            pthread_cond_signal(&portique_m->arriverCamion);
        }
        else
        {
            /* Si il reste des conteneurs a decharger dans le camion */
            if (portique_m->nbConteneursADechargerCamion > 0)
            {

                if (msgrcv(conteneursADechargerCamion, &grue, sizeof(conteneur), 1, 0) == -1)
                    erreur("Erreur : lecture conteneursADecahrgerCamion");

                if (grue.destination == NEWYORK && !portique_m->bateauLibre && portique_m->nbConteneursChargesBateau < NB_MAX_CONTENEURS_BATEAU)
                {
                    P(portique_m->semid, BATEAU);
                    portique_m->nbConteneursChargesBateau++;
                    V(portique_m->semid, BATEAU);
                    msgsnd(conteneursAChargerBateau, &grue, sizeof(conteneur), 0);
                    printf("Portique %d: chargement du conteneur %d du camion %d sur le bateau %d\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule, portique_m->idBateauAQuai);
                }
                else if (grue.destination == AMSTERDAM && !portique_m->trainLibre && portique_m->nbConteneursChargesTrain < NB_MAX_CONTENEURS_TRAIN)
                {
                    P(portique_m->semid, TRAIN);
                    portique_m->nbConteneursChargesTrain++;
                    V(portique_m->semid, TRAIN);
                    msgsnd(conteneursAChargerTrain, &grue, sizeof(conteneur), 0);
                    printf("Portique %d: chargement du conteneur %d du camion %d sur le train %d\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule, portique_m->idTrainAQuai);
                }
                else
                {
                    /* Il n'y a pas de bateau ni de train pour charger ce conteneur, on le remet dans la file de msg */
                    msgsnd(conteneursADechargerCamion, &grue, sizeof(conteneur), 0);
                    portique_m->nbConteneursADechargerCamion++;
                };

                portique_m->nbConteneursADechargerCamion--;

                usleep(TEMPS_MANOEUVRE_PORTIQUE);
            }

            /* Si le camion est plein, il peut partir */
            if (portique_m->nbConteneursChargesCamion >= NB_MAX_CONTENEURS_CAMION && portique_m->nbConteneursADechargerCamion <= 0)
            {
                // printf("debug camion %d part nbConteneursChargesCamion %d\n", portique_m->idCamionAQuai, portique_m->nbConteneursChargesCamion);
                pthread_cond_signal(&portique_m->partirCamion);
            }
        }
        // pthread_mutex_unlock(&portique_m->mutex);
        usleep(10000);
    }
}