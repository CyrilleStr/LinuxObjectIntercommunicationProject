#include "../inc/vehicule.h"

void creer_bateau()
{
    int id = (int)pthread_self();
    srand(id);

    printf("Creation bateau numero %d", id);
    conteneur *conteneurs = creer_conteneurs(rand() % NB_MAX_CONTENEURS, id);
    printf("Bateau %d: arrive au port", id);
    /* Attente bateau */
    printf("Bateau %d: charge");
}

void creer_train() {}

void creer_camion() {}

conteneur *creer_conteneurs(int nbConteneurs, int idVehicule)
{
    conteneur conteneurs[nbConteneurs];
    for (int i = 0; i < nbConteneurs; i++)
    {
        // conteneurs[i] = malloc(sizeof(conteneur));
        conteneurs[i].idConteneur = i;
        conteneurs[i].idVehicule = idVehicule;
        conteneurs[i].contenu = rand() % TAILLE_CONTENEUR;
    }
    return conteneurs;
}