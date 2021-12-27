#ifndef VEHICULE_HEADER
#define VEHICULE_HEADER

#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "general.h"

/**
 * @brief Créé un bateau
 *
 */
void *creer_bateau(void *portiques_p);

/**
 * @brief Créé un train
 *
 */
void *creer_train(void *portiques_p);

/**
 * @brief Créé un camion
 *
 */
void *creer_camion(void *portiques_p);

/**
 * @brief Créé un tableau de nbConteneurs conteneurs
 *
 * @param nbConteneurs nombre de conteneurs à créer
 * @param idVehicule l'id du véhicule propriétaire des conteneurs
 * @param provenance ville de provenance
 * @return conteneur* un tableau de conteneurs
 */
conteneur *creer_conteneurs(int nbConteneurs, int idVehicule, ville provenance);

#endif /* VEHICULE_HEADER */