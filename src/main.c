#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "../inc/portique.h"
#include "../inc/vehicule.h"
#include "../inc/general.h"

#define NB_BATEAU 5
#define NB_CAMION 5
#define NB_TRAIN 2
#define NB_PORTIQUE 2

int main(int argc, char *argv[])
{
    int i;
    pthread_t bateauThreadId[NB_BATEAU];
    pthread_t camionThreadId[NB_CAMION];
    pthread_t trainThreadId[NB_TRAIN];
    pthread_t posteControlePortique[NB_PORTIQUE];

    printf("Demarrage de la plateforme d'ailguillage\n");

    // Creation des portiques
    portique *portiques;
    portiques = (portique *)malloc(NB_PORTIQUE * sizeof(portique));

    for (i = 0; i < NB_PORTIQUE; i++)
    {
        // Creation des files d'attentes de conteneurs à charger/décharger
        if ((portiques[i].conteneursBateau = msgget(314, IPC_CREAT | 0600)) == -1)
            erreur("Erreur : creation file conteneur bateau \n");
        if ((portiques[i].conteneursCamion = msgget(315, IPC_CREAT | 0600)) == -1)
            erreur("Erreur : creation file conteneur camion \n");
        if ((portiques[i].conteneursTrain = msgget(316, IPC_CREAT | 0600)) == -1)
            erreur("Erreur : creation file conteneur train\n");

        // Initialisation des mutex de contrôle d'arrivée au portique des véhicules
        pthread_mutex_init(&portiques[i].mutex, 0);
        pthread_cond_init(&portiques[i].arriverTrain, 0);
        pthread_cond_init(&portiques[i].arriverBateau, 0);
        pthread_cond_init(&portiques[i].arriverCamion, 0);

        // Création du poste du controle du portique
        pthread_create(&posteControlePortique[i], 0, (void *(*)())creer_post_de_controle, (void *)&portiques[i]);
    }

    // Creation des vehicules
    // for (i = 0; i < NB_BATEAU; i++)
    // {
    //     pthread_create(&bateauThreadId[i], 0, (void *(*)())creer_bateau, (void *)portiques);
    // }
    // for (i = 0; i < NB_TRAIN; i++)
    // {
    //     pthread_create(&trainThreadId[i], 0, (void *(*)())creer_train, (void *)portiques);
    // }
    // for (i = 0; i < NB_CAMION; i++)
    // {
    //     pthread_create(&camionThreadId[i], 0, (void *(*)())creer_camion, (void *)portiques);
    // }

    // // Destruction des vehicules et des portiques
    // for (i = 0; i < NB_BATEAU; i++)
    // {
    //     pthread_join(bateauThreadId[i], NULL);
    // }
    // for (i = 0; i < NB_TRAIN; i++)
    // {
    //     pthread_join(trainThreadId[i], NULL);
    // }
    // for (i = 0; i < NB_CAMION; i++)
    // {
    //     pthread_join(camionThreadId[i], NULL);
    // }
    for (i = 0; i < NB_PORTIQUE; i++)
    {
        pthread_join(posteControlePortique[i], NULL);
        pthread_cond_destroy(&portiques[i].arriverBateau);
        pthread_cond_destroy(&portiques[i].arriverCamion);
        pthread_cond_destroy(&portiques[i].arriverTrain);
    }
    free(portiques);

    printf("Arrêt de la plateforme d'ailguillage\n");
}