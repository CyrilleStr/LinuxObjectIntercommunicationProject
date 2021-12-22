#ifndef GENERAL_HEADER
#define GENERAL_HEADER

#include <stdlib.h>
#include <stdio.h>

#define CLE_FILE_CONTENEUR_BATEAU (key_t)314
#define CLE_FILE_CONTENEUR_CAMION (key_t)315
#define CLE_FILE_ATTENTE_TRAIN (key_t)316

/**
 * @brief Structure du portique contrôlant les arrivées des véhicules (via segment de mémoire partagée)
 * et le transfert de conteneurs (via file de messages)
 *
 */
typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t arriverCamion;
    pthread_cond_t arriverTrain;
    pthread_cond_t arriverBateau;
    int conteneursBateau;
    int conteneursTrain;
    int conteneursCamion;
} portique;

typedef enum
{
    false = 0,
    true = 1
} boolean;

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
 * @brief todo
 *
 */
typedef struct
{
    boolean portLibre;
    boolean quaiLibre;
    boolean parkingLibre;
    conteneur conteneur;
} requete;

static void erreur(const char *msg)
{
    perror(msg);
    exit(1);
}

#endif
