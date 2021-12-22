#include "../inc/vehicule.h"

void *creer_bateau(void *portiques_p)
{
    int id = (int)pthread_self();
    int nbConteneur = rand() % NB_MAX_CONTENEURS_BATEAU;
    portique *portiques = (portique *)portiques_p;
    srand(id);

    // printf("Creation bateau   numero %d\n", id);
    conteneur *conteneurs = creer_conteneurs(nbConteneur, id);
    printf("Bateau %d: arrive au port\n", id);
    /* Attente bateau */
    printf("Bateau %d: charge\n", id);
    /* Liberation de la mémoire des conteneurs */
    free(conteneurs);
}

void *creer_train(void *portiques_p)
{
    int id = (int)pthread_self();
    int nbConteneur = rand() % NB_MAX_CONTENEURS_TRAIN;
    portique *portiques = (portique *)portiques_p;
    srand(id);

    // printf("Creation train numero %d\n", id);
    conteneur *conteneurs = creer_conteneurs(nbConteneur, id);
    printf("Train %d: arrive à la gare\n", id);
    /* Attente bateau */
    printf("Train %d: charge\n", id);
    /* Liberation de la mémoire des conteneurs */
    free(conteneurs);
}

void *creer_camion(void *portiques_p)
{
    int id = (int)pthread_self();
    portique *portiques = (portique *)portiques_p;
    srand(id);

    // printf("Creation Camion numero %d\n", id);
    conteneur *conteneurs = creer_conteneurs(1, id);
    printf("Camion %d: arrive au parking\n", id);
    /* Attente bateau */
    printf("Camion %d: charge\n", id);
    /* Liberation de la mémoire des conteneurs */
    free(conteneurs);
}

conteneur *creer_conteneurs(int nbConteneurs, int idVehicule)
{
    conteneur *conteneurs = (conteneur *)malloc(nbConteneurs * sizeof(conteneur));
    for (int i = 0; i < nbConteneurs; i++)
    {
        // conteneurs[i] = (conteneur *)malloc(sizeof(conteneur));
        conteneurs[i].idConteneur = i;
        conteneurs[i].idVehicule = idVehicule;
        conteneurs[i].contenu = rand() % TAILLE_CONTENEUR;
        printf("conteneur %d (vehicule %d) : %ld\n", conteneurs[i].idConteneur, conteneurs[i].idConteneur, conteneurs[i].contenu);
    }
    return conteneurs;
}