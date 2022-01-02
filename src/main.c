#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "../inc/portique.h"
#include "../inc/vehicule.h"
#include "../inc/general.h"

int main(int argc, char *argv[])
{
    int i;

    pthread_t bateauThreadId[NB_BATEAU];
    pthread_t camionThreadId[NB_CAMION];
    pthread_t trainThreadId[NB_TRAIN];
    pthread_t posteControlePortique[NB_PORTIQUE];

    printf("Demarrage de la plateforme d'ailguillage\n");

    /* Creation des portiques */
    portique *portiques;
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
        pthread_create(&posteControlePortique[i], 0, (void *(*)())creer_post_de_controle, (void *)&portiques[i]);
    }

    usleep(10000);

    /* Creation des sémpahores d'accès de lecture au variables compteurConteneurBateau, compteurConteneurTrain et compteurConteneurCamion */
    semCompteurId = initsem(465);

    vehiculeParam *paramsBateau = (vehiculeParam *)malloc(sizeof(vehiculeParam) * NB_BATEAU);
    vehiculeParam *paramsTrain = (vehiculeParam *)malloc(sizeof(vehiculeParam) * NB_TRAIN);
    vehiculeParam *paramsCamion = (vehiculeParam *)malloc(sizeof(vehiculeParam) * NB_CAMION);

    /* Creation des vehicules */
    for (i = 0; i < NB_BATEAU; i++)
    {
        paramsBateau[i].numVehicule = i;
        paramsBateau[i].portiques = portiques;
        pthread_create(&bateauThreadId[i], 0, (void *(*)())creer_bateau, (void *)&paramsBateau[i]);
    }
    for (i = 0; i < NB_TRAIN; i++)
    {
        paramsTrain[i].numVehicule = i;
        paramsTrain[i].portiques = portiques;
        pthread_create(&trainThreadId[i], 0, (void *(*)())creer_train, (void *)&paramsTrain[i]);
    }
    for (i = 0; i < NB_CAMION; i++)
    {
        paramsCamion[i].numVehicule = i;
        paramsCamion[i].portiques = portiques;
        pthread_create(&camionThreadId[i], 0, (void *(*)())creer_camion, (void *)&paramsCamion[i]);
    }
    /* Destruction des vehicules et des portiques */
    for (i = 0; i < NB_BATEAU; i++)
    {
        pthread_join(bateauThreadId[i], NULL);
    }
    for (i = 0; i < NB_TRAIN; i++)
    {
        pthread_join(trainThreadId[i], NULL);
    }
    for (i = 0; i < NB_CAMION; i++)
    {
        pthread_join(camionThreadId[i], NULL);
    }
    for (i = 0; i < NB_PORTIQUE; i++)
    {
        pthread_join(posteControlePortique[i], NULL);
        pthread_cond_destroy(&portiques[i].arriverBateau);
        pthread_cond_destroy(&portiques[i].arriverCamion);
        pthread_cond_destroy(&portiques[i].arriverTrain);
        printf("debug fin 5\n");
        if (msgctl(portiques[i].semId, IPC_RMID, NULL) == -1)
            erreur("Erreur : file de message non detectee\n");
    }

    free(portiques);

    printf("Arrêt de la plateforme d'ailguillage\n");
}