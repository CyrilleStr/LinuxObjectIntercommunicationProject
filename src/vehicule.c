#include "../inc/vehicule.h"

void *creer_bateau()
{
    int id = (int)pthread_self();
    int nbConteneur = rand() % NB_MAX_CONTENEURS;
    srand(id);

    printf("Creation bateau numero %d\n", id);
    conteneur *conteneurs = creer_conteneurs(nbConteneur, id);
    printf("Bateau %d: arrive au port\n", id);
    /* Attente bateau */
    printf("Bateau %d: charge\n", id);
    /* Liberation de la mémoire des conteneurs */
    printf("contenu bateau %d: %ld\n", 0, conteneurs[0].contenu);
    free(conteneurs);
}

void *creer_train() {}

void *creer_camion() {}

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