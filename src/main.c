#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../inc/portique.h"
#include "../inc/vehicule.h"
#include "../inc/quai.h"

#define NB_BATEAU 5
#define NB_CAMION 5
#define NB_TRAIN 2

int main(int argc, char *argv[])
{
    int i;
    pthread_t bateauThreadId[NB_BATEAU];
    pthread_t camionThreadId[NB_CAMION];
    pthread_t trainThreadId[NB_TRAIN];

    printf("Demarrage de la plateforme d'ailguillage\n");

    // Creation des vehicules
    printf("Création des vehicules\n");
    for (i = 0; i < NB_BATEAU; i++)
    {
        pthread_create(&bateauThreadId[i], 0, (void *(*)())creer_bateau, NULL);
    }

    // Destruction des vehicules
    for (i = 0; i < NB_BATEAU; i++)
    {
        pthread_join(bateauThreadId[i], NULL);
    }
    printf("Arrêt de la plateforme d'ailguillage\n");
}