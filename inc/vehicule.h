#ifndef VEHICULE_HEADER
#define VEHICULE_HEADER

#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define NB_MAX_CONTENEURS 20
#define TAILLE_CONTENEUR 10000000

/**
 * @brief Structure de conteneur
 *
 */
typedef struct
{
    int idVehicule;
    int idConteneur;
    long contenu;
} conteneur;

/**
 * @brief Créé un bateau
 *
 */
void creer_bateau();

/**
 * @brief Créé un train
 *
 */
void creer_train();

/**
 * @brief Créé un camion
 *
 */
void creer_camion();

conteneur *creer_conteneurs(int nbConteneurs, int idVehicule);

#endif