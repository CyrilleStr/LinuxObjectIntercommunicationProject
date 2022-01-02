#include "../inc/portique.h"

void *creer_post_de_controle(void *portique_p)
{
    boolean continuer = true;
    int fin = 0;
    portique *portique_m = (portique *)portique_p;
    conteneur grue;
    struct msqid_ds bufConteneurChargerBateau;
    struct msqid_ds bufConteneurDechargerBateau;

    int /* File de messages */
        conteneursAChargerBateau,
        conteneursADechargerBateau,
        conteneursAChargerTrain,
        conteneursADechargerTrain,
        conteneursAChargerCamion,
        conteneursADechargerCamion,
        /* Compteurs de non-chargement et non-dechargement */
        compteurConteneurNonDechargeBateau = 0,
        compteurConteneurNonDechargeTrain = 0,
        compteurConteneurNonChargeBateau = 0,
        compteurConteneurNonChargeTrain = 0,
        compteurConteneurNonChargeCamion = 0;

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

    while (continuer)
    {
        /* Bateau */
        if (portique_m->bateauLibre)
        {
            /* La place au port est libre, on appelle un nouveau bateau à venir s'amarrer */
            pthread_mutex_lock(&portique_m->mutexBateau);
            pthread_cond_signal(&portique_m->arriverBateau);
            pthread_mutex_unlock(&portique_m->mutexBateau);
        }
        else
        {
            if (portique_m->nbConteneursADechargerBateau > 0)
            {
                /* Il reste des conteneurs a decharger dans le bateau */
                if (msgrcv(conteneursADechargerBateau, &grue, sizeof(conteneur), 1, 0) == -1)
                    erreur("Erreur : lecture conteneursADecahrgerBateau");

                P(portique_m->semId, CAMION);
                P(portique_m->semId, TRAIN);
                P(portique_m->semId, BATEAU);
                if (grue.destination == AMSTERDAM && !portique_m->trainLibre && portique_m->nbConteneursChargesTrain < NB_MAX_CONTENEURS_TRAIN)
                {
                    /* On charge le conteneur dans le train a quai */
                    compteurConteneurNonDechargeBateau = 0;
                    compteurConteneurNonChargeTrain = 0;
                    portique_m->nbConteneursChargesTrain++;
                    msgsnd(conteneursAChargerTrain, &grue, sizeof(conteneur), 0);
                    printf("Portique %d: chargement du conteneur %d du bateau %d sur le train %d\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule, portique_m->idTrainAQuai);
                }
                else if (grue.destination == PARIS && !portique_m->camionLibre && portique_m->nbConteneursChargesCamion < NB_MAX_CONTENEURS_CAMION)
                {
                    /* On charge le conteneur dans le camion a quai */
                    compteurConteneurNonDechargeBateau = 0;
                    compteurConteneurNonChargeCamion = 0;
                    portique_m->nbConteneursChargesCamion++;
                    msgsnd(conteneursAChargerCamion, &grue, sizeof(conteneur), 0);
                    printf("Portique %d: chargement du conteneur %d du bateau %d sur le camion %d\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule, portique_m->idCamionAQuai);
                }
                else
                {
                    compteurConteneurNonDechargeBateau++;
                    if (compteurConteneurNonDechargeBateau < portique_m->nbConteneursADechargerBateau)
                    {
                        /* Il n'y a pas de camion ni de train pour charger ce conteneur, on le remet dans la file de msg */
                        printf("Portique %d: conteneur %d du bateau %d ne peut pas etre decharges, on essaie un autre conteneur\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule);
                        msgsnd(conteneursADechargerBateau, &grue, sizeof(conteneur), 0);
                        portique_m->nbConteneursADechargerBateau++;
                    }
                    else
                    {
                        /* Le portique a essayé de décharger tous les conteneurs restant sans succès, le bateau peut partir */
                        pthread_mutex_lock(&portique_m->mutexBateau);
                        pthread_cond_signal(&portique_m->partirBateau);
                        pthread_mutex_unlock(&portique_m->mutexBateau);
                        compteurConteneurNonDechargeBateau = 0;
                        printf("Portique %d: aucun conteneurs restants du bateau %d ne peut être decharges il part\n", portique_m->numPortique, grue.idVehicule);
                    }
                };
                portique_m->nbConteneursADechargerBateau--;
                V(portique_m->semId, CAMION);
                V(portique_m->semId, TRAIN);
                V(portique_m->semId, BATEAU);

                usleep(TEMPS_MANOEUVRE_PORTIQUE);
            }
            else if (portique_m->nbConteneursChargesBateau >= NB_MAX_CONTENEURS_BATEAU && portique_m->nbConteneursADechargerBateau <= 0)
            {
                /* Le bateau est plein et il a tout dechargé, il peut partir */
                pthread_mutex_lock(&portique_m->mutexBateau);
                pthread_cond_signal(&portique_m->partirBateau);
                pthread_mutex_unlock(&portique_m->mutexBateau);
                printf("Portique %d: bateau %d est plein, il peut partir\n", portique_m->numPortique, portique_m->idBateauAQuai);
            }
            else
            {
                compteurConteneurNonChargeBateau++;
                if (compteurConteneurNonChargeBateau > NB_MANOEUVRE_SANS_CHARGEMENT_AUTORISE)
                {
                    /* Aucun autre conteneur du camion ou du train a quai ne peut être chargé dessus */
                    pthread_mutex_lock(&portique_m->mutexBateau);
                    pthread_cond_signal(&portique_m->partirBateau);
                    pthread_mutex_unlock(&portique_m->mutexBateau);
                    compteurConteneurNonChargeBateau = 0;
                    printf("Portique %d : aucun autre conteneur du camion ou du train a quai ne peut être chargé sur le bateau %d, il part\n", portique_m->numPortique, portique_m->idBateauAQuai);
                }
            }
        }

        /* Train */
        if (portique_m->trainLibre)
        {
            pthread_mutex_lock(&portique_m->mutexTrain);
            pthread_cond_signal(&portique_m->arriverTrain);
            pthread_mutex_unlock(&portique_m->mutexTrain);
        }
        else
        {
            /* Si il reste des conteneurs a decharger dans le train */
            if (portique_m->nbConteneursADechargerTrain > 0)
            {

                if (msgrcv(conteneursADechargerTrain, &grue, sizeof(conteneur), 1, 0) == -1)
                    erreur("Erreur : lecture conteneursADecahrgerTrain");

                P(portique_m->semId, CAMION);
                P(portique_m->semId, TRAIN);
                P(portique_m->semId, BATEAU);

                if (grue.destination == NEWYORK && !portique_m->bateauLibre && portique_m->nbConteneursChargesBateau < NB_MAX_CONTENEURS_BATEAU)
                {
                    /* On charge le conteneur dans le bateau a quai */
                    compteurConteneurNonDechargeTrain = 0;
                    compteurConteneurNonChargeBateau = 0;
                    portique_m->nbConteneursChargesBateau++;
                    msgsnd(conteneursAChargerBateau, &grue, sizeof(conteneur), 0);
                    printf("Portique %d: chargement du conteneur %d du train %d sur le bateau %d\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule, portique_m->idBateauAQuai);
                }
                else if (grue.destination == PARIS && !portique_m->camionLibre && portique_m->nbConteneursChargesCamion < NB_MAX_CONTENEURS_CAMION)
                {
                    /* On charge le conteneur dans le camion a quai */
                    compteurConteneurNonDechargeTrain = 0;
                    compteurConteneurNonChargeCamion = 0;
                    portique_m->nbConteneursChargesCamion++;
                    msgsnd(conteneursAChargerCamion, &grue, sizeof(conteneur), 0);
                    printf("Portique %d: chargement du conteneur %d du train %d sur le camion %d\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule, portique_m->idCamionAQuai);
                }
                else
                {
                    compteurConteneurNonDechargeTrain++;
                    if (compteurConteneurNonDechargeTrain < NB_MAX_CONTENEURS_TRAIN)
                    {
                        /* Il n'y a pas de camion ni de bateau pour charger ce conteneur, on le remet dans la file de msg */
                        printf("Portique %d: conteneur %d du train %d ne peut pas etre decharges, on essaie un autre conteneur\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule);
                        msgsnd(conteneursADechargerTrain, &grue, sizeof(conteneur), 0);
                        portique_m->nbConteneursADechargerTrain++;
                    }
                    else
                    {
                        /* Le portique a essayé de décharger tous les conteneurs restant sans succès, le train peut partir */
                        pthread_mutex_lock(&portique_m->mutexTrain);
                        pthread_cond_signal(&portique_m->partirTrain);
                        pthread_mutex_unlock(&portique_m->mutexTrain);
                        compteurConteneurNonDechargeTrain = 0;
                        printf("Portique %d: aucun conteneurs restants du train %d ne peut être decharges il part\n", portique_m->numPortique, grue.idVehicule);
                    }
                };
                portique_m->nbConteneursADechargerTrain--;
                V(portique_m->semId, CAMION);
                V(portique_m->semId, TRAIN);
                V(portique_m->semId, BATEAU);

                usleep(TEMPS_MANOEUVRE_PORTIQUE);
            }
            else if (portique_m->nbConteneursChargesTrain >= NB_MAX_CONTENEURS_TRAIN && portique_m->nbConteneursADechargerTrain <= 0)
            {
                /* Le train est plein et il a tout déchargé, il peut partir */
                pthread_mutex_lock(&portique_m->mutexTrain);
                pthread_cond_signal(&portique_m->partirTrain);
                pthread_mutex_unlock(&portique_m->mutexTrain);
                printf("Portique %d: train %d est plein, il peut partir\n", portique_m->numPortique, portique_m->idTrainAQuai);
            }
            else
            {
                /* Aucun autre conteneur du camion ou du train a quai ne peut être chargé dessus */
                compteurConteneurNonChargeTrain++;
                if (compteurConteneurNonChargeTrain > NB_MANOEUVRE_SANS_CHARGEMENT_AUTORISE)
                {
                    pthread_mutex_lock(&portique_m->mutexTrain);
                    pthread_cond_signal(&portique_m->partirTrain);
                    pthread_mutex_unlock(&portique_m->mutexTrain);
                    compteurConteneurNonChargeTrain = 0;
                    printf("Portique %d : aucun autre conteneur du camion ou du bateau a quai ne peut être chargé sur le train %d, il part\n", portique_m->numPortique, portique_m->idTrainAQuai);
                }
            }
        }

        /* Camion */
        if (portique_m->camionLibre)
        {
            pthread_mutex_lock(&portique_m->mutexCamion);
            pthread_cond_signal(&portique_m->arriverCamion);
            pthread_mutex_unlock(&portique_m->mutexCamion);
        }
        else
        {
            /* Si il reste des conteneurs a decharger dans le camion */
            if (portique_m->nbConteneursADechargerCamion > 0)
            {

                if (msgrcv(conteneursADechargerCamion, &grue, sizeof(conteneur), 1, 0) == -1)
                    erreur("Erreur : lecture conteneursADecahrgerCamion");

                P(portique_m->semId, CAMION);
                P(portique_m->semId, TRAIN);
                P(portique_m->semId, BATEAU);

                if (grue.destination == NEWYORK && !portique_m->bateauLibre && portique_m->nbConteneursChargesBateau < NB_MAX_CONTENEURS_BATEAU)
                {
                    /* On charge le conteneur dans le bateau a quai */
                    compteurConteneurNonChargeBateau = 0;
                    portique_m->nbConteneursChargesBateau++;
                    msgsnd(conteneursAChargerBateau, &grue, sizeof(conteneur), 0);
                    printf("Portique %d: chargement du conteneur %d du camion %d sur le bateau %d\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule, portique_m->idBateauAQuai);
                }
                else if (grue.destination == AMSTERDAM && !portique_m->trainLibre && portique_m->nbConteneursChargesTrain < NB_MAX_CONTENEURS_TRAIN)
                {
                    /* On charge le conteneur dans le train a quai */
                    compteurConteneurNonChargeTrain = 0;
                    portique_m->nbConteneursChargesTrain++;
                    msgsnd(conteneursAChargerTrain, &grue, sizeof(conteneur), 0);
                    printf("Portique %d: chargement du conteneur %d du camion %d sur le train %d\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule, portique_m->idTrainAQuai);
                }
                else
                {
                    /* Il n'y a pas de bateau ni de train pour decharger ce conteneur, il peut partir */
                    pthread_mutex_lock(&portique_m->mutexCamion);
                    pthread_cond_signal(&portique_m->partirCamion);
                    pthread_mutex_unlock(&portique_m->mutexCamion);
                    printf("Portique %d: conteneur %d du camion %d ne peut pas etre decharges, il peut partir\n", portique_m->numPortique, grue.idConteneur + 1, grue.idVehicule);
                };
                portique_m->nbConteneursADechargerCamion--;
                V(portique_m->semId, CAMION);
                V(portique_m->semId, TRAIN);
                V(portique_m->semId, BATEAU);

                usleep(TEMPS_MANOEUVRE_PORTIQUE);
            }
            else if (portique_m->nbConteneursChargesCamion >= NB_MAX_CONTENEURS_CAMION && portique_m->nbConteneursADechargerCamion <= 0)
            {
                /* Le camion est plein et il a tout dechargé, il peut partir */
                printf("Portique %d: camion %d est plein, il peut partir\n", portique_m->numPortique, portique_m->idCamionAQuai);
                pthread_mutex_lock(&portique_m->mutexCamion);
                pthread_cond_signal(&portique_m->partirCamion);
                pthread_mutex_unlock(&portique_m->mutexCamion);
            }
            else
            {
                /* Aucun autre conteneur du camion ou du train a quai ne peut être chargé dessus */
                compteurConteneurNonChargeCamion++;
                if (compteurConteneurNonChargeCamion > NB_MANOEUVRE_SANS_CHARGEMENT_AUTORISE)
                {
                    pthread_mutex_lock(&portique_m->mutexCamion);
                    pthread_cond_signal(&portique_m->partirCamion);
                    pthread_mutex_unlock(&portique_m->mutexCamion);
                    printf("Portique %d : aucun autre conteneur du bateau ou du train a quai ne peut être chargé sur le camion %d, il part\n", portique_m->numPortique, portique_m->idCamionAQuai);
                    compteurConteneurNonChargeCamion = 0;
                }
            }
        }
        usleep(10000);
        // printf("%d: nbConteneursChargesBateau %d nbConteneursADechargesBateau %d nbConteneursChargesTrain %d nbConteneursADechargesTrain %d nbConteneursChargesCamion %d nbConteneursADechargerCamion %d\n", portique_m->numPortique, portique_m->nbConteneursChargesBateau, portique_m->nbConteneursADechargerBateau, portique_m->nbConteneursChargesTrain, portique_m->nbConteneursADechargerTrain, portique_m->nbConteneursChargesCamion, portique_m->nbConteneursADechargerCamion);
    }
    return NULL;
}