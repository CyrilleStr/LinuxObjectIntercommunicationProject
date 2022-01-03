#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include "../inc/portique.h"
#include "../inc/vehicule.h"
#include "../inc/general.h"

int main(int argc, char *argv[])
{
    int i;
    portique *portiques;
    pthread_t bateauThreadId[NB_BATEAU];
    pthread_t camionThreadId[NB_CAMION];
    pthread_t trainThreadId[NB_TRAIN];
    pthread_t posteControlePortique[NB_PORTIQUE];

    printf("Demarrage de la plateforme d'ailguillage\n");

    /* Creation des portiques */
    portiques = (portique *)malloc(NB_PORTIQUE * sizeof(portique));

    for (i = 0; i < NB_PORTIQUE; i++)
    {
        /* Initialisation des mutex de contrôle d'arrivée au portique des véhicules */
        pthread_mutex_init(&portiques[i].mutexBateau, 0);
        pthread_mutex_init(&portiques[i].mutexCamion, 0);
        pthread_mutex_init(&portiques[i].mutexTrain, 0);
        pthread_cond_init(&portiques[i].arriverTrain, 0);
        pthread_cond_init(&portiques[i].arriverBateau, 0);
        pthread_cond_init(&portiques[i].arriverCamion, 0);
        pthread_cond_init(&portiques[i].partirBateau, 0);
        pthread_cond_init(&portiques[i].partirTrain, 0);
        pthread_cond_init(&portiques[i].partirCamion, 0);
        pthread_cond_init(&portiques[i].bateauEstParti, 0);
        pthread_cond_init(&portiques[i].trainEstParti, 0);
        pthread_cond_init(&portiques[i].camionEstParti, 0);

        /* Creation des sémaphores d'accès de lecture au variable bateauLibre, trainLibre et camionLibre */
        portiques[i].semId = initsem(clefsSem[i]);

        /* Création du poste du controle du portique */
        portiques[i].numPortique = i + 1;
        pthread_create(posteControlePortique + i, 0, (void *(*)())creer_post_de_controle, (void *)&portiques[i]);
    }

    vehiculeParam *paramsBateau = (vehiculeParam *)malloc(sizeof(vehiculeParam) * NB_BATEAU);
    vehiculeParam *paramsTrain = (vehiculeParam *)malloc(sizeof(vehiculeParam) * NB_TRAIN);
    vehiculeParam *paramsCamion = (vehiculeParam *)malloc(sizeof(vehiculeParam) * NB_CAMION);

    /* Creation des vehicules */
    for (i = 0; i < NB_BATEAU; i++)
    {
        paramsBateau[i].numVehicule = i;
        paramsBateau[i].portiques = portiques;
        pthread_create(bateauThreadId + i, 0, (void *(*)())creer_bateau, (void *)&paramsBateau[i]);
    }
    for (i = 0; i < NB_TRAIN; i++)
    {
        paramsTrain[i].numVehicule = i;
        paramsTrain[i].portiques = portiques;
        pthread_create(trainThreadId + i, 0, (void *(*)())creer_train, (void *)&paramsTrain[i]);
    }
    for (i = 0; i < NB_CAMION; i++)
    {
        paramsCamion[i].numVehicule = i;
        paramsCamion[i].portiques = portiques;
        pthread_create(camionThreadId + i, 0, (void *(*)())creer_camion, (void *)&paramsCamion[i]);
    }

    usleep(4000);

    /* Destruction des vehicules et des portiques */
    for (i = 0; i < NB_BATEAU; i++)
    {
        pthread_join(bateauThreadId[i], NULL);
        printf("debug jopin\n");
    }

    for (i = 0; i < NB_CAMION; i++)
    {
        pthread_join(camionThreadId[i], NULL);
    }
    for (i = 0; i < NB_TRAIN; i++)
    {
        pthread_join(trainThreadId[i], NULL);
    }

    for (i = 0; i < NB_PORTIQUE; i++)
    {
        portiques[i].continuer = false;
        pthread_join(posteControlePortique[i], NULL);
        pthread_mutex_destroy(&portiques[i].mutexBateau);
        pthread_mutex_destroy(&portiques[i].mutexCamion);
        pthread_mutex_destroy(&portiques[i].mutexTrain);
        pthread_cond_destroy(&portiques[i].arriverTrain);
        pthread_cond_destroy(&portiques[i].arriverBateau);
        pthread_cond_destroy(&portiques[i].arriverCamion);
        pthread_cond_destroy(&portiques[i].partirBateau);
        pthread_cond_destroy(&portiques[i].partirTrain);
        pthread_cond_destroy(&portiques[i].partirCamion);
        pthread_cond_destroy(&portiques[i].bateauEstParti);
        pthread_cond_destroy(&portiques[i].trainEstParti);
        pthread_cond_destroy(&portiques[i].camionEstParti);
        if (msgctl(portiques[i].semId, IPC_RMID, NULL) == -1)
            erreur("Erreur : file de message non detectee\n");
    }

    free(portiques);

    printf("Arrêt de la plateforme d'ailguillage\n");
}