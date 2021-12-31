#include "../inc/vehicule.h"

void *creer_bateau(void *params)
{
    vehiculeParam *parametre = (vehiculeParam *)params;
    int id = parametre->numVehicule;
    portique *portiques = parametre->portiques;
    int nbConteneur = rand() % NB_MAX_CONTENEURS_BATEAU;
    conteneur *conteneurs = creer_conteneurs(nbConteneur, id, NEWYORK);
    conteneur conteneurAcharger;
    int choixPortique, i;
    int conteneursAChargerBateau, conteneursADechargerBateau;
    srand(id);

    // printf("Bateau %d: arrive au port\n", id);

    /* Si un des 2 portiques est libre, le bateau y va, sinon il fait la queue à un portique choisi au hasard */
    P(portiques[0].semid, BATEAU);
    P(portiques[1].semid, BATEAU);
    if (portiques[0].bateauLibre)
        choixPortique = 0;
    else if (portiques[1].bateauLibre)
        choixPortique = 1;
    else
        choixPortique = rand() % NB_PORTIQUE;
    V(portiques[0].semid, BATEAU);
    V(portiques[1].semid, BATEAU);

    pthread_mutex_lock(&portiques[choixPortique].mutex);
    P(portiques[choixPortique].semid, 0);
    if (!portiques[choixPortique].bateauLibre) /* Attend l'autorisation du portique s'amarrer */
    {
        V(portiques[choixPortique].semid, BATEAU);
        pthread_cond_wait(&portiques[choixPortique].arriverBateau, &portiques[choixPortique].mutex);
    }
    else
    {
        V(portiques[choixPortique].semid, BATEAU);
    }

    P(portiques[choixPortique].semid, BATEAU);

    /*  Transmet les informations du bateau au portique */
    portiques[choixPortique].bateauLibre = false;
    portiques[choixPortique].idBateauAQuai = id;
    portiques[choixPortique].nbConteneursADechargerBateau = nbConteneur;
    portiques[choixPortique].nbConteneursChargesBateau = 0;

    printf("Bateau %d: s'amarre au portique %d\n", id, choixPortique + 1);

    /* Recuère les files de messages du portique et transmet le nb de conteneur a décharger */
    if ((conteneursAChargerBateau = msgget(portiques[choixPortique].clefConteneursAChargerBateau, FLAGS)) == -1)
        erreur("Erreur : creation file charger conteneur bateau \n");
    if ((conteneursADechargerBateau = msgget(portiques[choixPortique].clefConteneursADechargerBateau, FLAGS)) == -1)
        erreur("Erreur : creation file decharger conteneur bateau \n");

    /* Liste tous les conteneurs à décharger */
    for (i = 0; i < nbConteneur; i++)
        msgsnd(conteneursADechargerBateau, &conteneurs[i], sizeof(conteneur), 0);

    V(portiques[choixPortique].semid, BATEAU);

    /* Attend d'être totalement chargé pour partir */
    pthread_cond_wait(&portiques[choixPortique].partirBateau, &portiques[choixPortique].mutex);
    for (i = 0; i < NB_MAX_CONTENEURS_BATEAU; i++)
    {
        // printf("Debug Bateau %d: attend msg\n", id);
        if (msgrcv(conteneursAChargerBateau, &conteneurAcharger, sizeof(conteneur), 1, 0) == -1)
            erreur("Erreur : lecture conteneursAChargerBateau");
        conteneurs[i] = conteneurAcharger;
    }

    P(portiques[choixPortique].semid, BATEAU);
    portiques[choixPortique].bateauLibre = true;
    portiques[choixPortique].idBateauAQuai = -1;
    V(portiques[choixPortique].semid, BATEAU);
    pthread_mutex_unlock(&portiques[choixPortique].mutex);
    printf("Bateau %d: est totalement charge, il quitte le port\n", id);

    /* Liberation de la mémoire des conteneurs */
    free(conteneurs);
}

void *creer_train(void *params)
{
    vehiculeParam *parametre = (vehiculeParam *)params;
    int id = parametre->numVehicule;
    portique *portiques = parametre->portiques;
    int nbConteneur = rand() % NB_MAX_CONTENEURS_TRAIN;
    conteneur *conteneurs = creer_conteneurs(nbConteneur, id, AMSTERDAM);
    conteneur conteneurAcharger;
    int choixPortique, i;
    int conteneursAChargerTrain, conteneursADechargerTrain;
    srand(id);

    // printf("Train %d: arrive a quai\n", id);

    /* Si un des 2 portiques est libre, le Train y va, sinon il fait la queue à un portique choisi au hasard */
    P(portiques[0].semid, TRAIN);
    P(portiques[1].semid, TRAIN);
    if (portiques[0].trainLibre)
        choixPortique = 0;
    else if (portiques[1].trainLibre)
        choixPortique = 1;
    else
        choixPortique = rand() % NB_PORTIQUE;
    V(portiques[0].semid, TRAIN);
    V(portiques[1].semid, TRAIN);

    pthread_mutex_lock(&portiques[choixPortique].mutex);
    P(portiques[choixPortique].semid, TRAIN);
    if (!portiques[choixPortique].trainLibre) /* Attend l'autorisation du portique s'arrêter */
    {
        V(portiques[choixPortique].semid, TRAIN);
        pthread_cond_wait(&portiques[choixPortique].arriverTrain, &portiques[choixPortique].mutex);
    }
    else
    {
        V(portiques[choixPortique].semid, TRAIN);
    }

    P(portiques[choixPortique].semid, TRAIN);

    /* Transmet les informations du train au portique */
    portiques[choixPortique].trainLibre = false;
    portiques[choixPortique].idTrainAQuai = id;
    portiques[choixPortique].nbConteneursADechargerTrain = nbConteneur;
    portiques[choixPortique].nbConteneursChargesTrain = 0;

    printf("Train %d: s'arrete au portique %d\n", id, choixPortique + 1);

    /* Recuère les files de messages du portique */
    if ((conteneursAChargerTrain = msgget(portiques[choixPortique].clefConteneursAChargerTrain, FLAGS)) == -1)
        erreur("Erreur : creation file charger conteneur train \n");
    if ((conteneursADechargerTrain = msgget(portiques[choixPortique].clefConteneursADechargerTrain, FLAGS)) == -1)
        erreur("Erreur : creation file decharger conteneur train \n");

    /* Liste tous les conteneurs à décharger */
    for (i = 0; i < nbConteneur; i++)
        msgsnd(conteneursADechargerTrain, &conteneurs[i], sizeof(conteneur), 0);

    V(portiques[choixPortique].semid, TRAIN);

    /* Attend d'être totalement chargé pour partir */
    pthread_cond_wait(&portiques[choixPortique].partirTrain, &portiques[choixPortique].mutex);
    for (i = 0; i < NB_MAX_CONTENEURS_TRAIN; i++)
    {
        // printf("Debug Train %d: attend msg\n", id);
        if (msgrcv(conteneursAChargerTrain, &conteneurAcharger, sizeof(conteneur), 1, 0) == -1)
            erreur("Erreur : lecture conteneursAChargerTrain");
        conteneurs[i] = conteneurAcharger;
    }

    P(portiques[choixPortique].semid, TRAIN);
    portiques[choixPortique].trainLibre = true;
    portiques[choixPortique].idTrainAQuai = -1;
    V(portiques[choixPortique].semid, TRAIN);
    pthread_mutex_unlock(&portiques[choixPortique].mutex);
    printf("Train %d: est totalement charge, il quitte la gare\n", id);

    /* Liberation de la mémoire des conteneurs */
    free(conteneurs);
}

void *creer_camion(void *params)
{
    vehiculeParam *parametre = (vehiculeParam *)params;
    int id = parametre->numVehicule;
    portique *portiques = parametre->portiques;
    int nbConteneur = NB_MAX_CONTENEURS_CAMION;
    conteneur *conteneurs = creer_conteneurs(nbConteneur, id, PARIS);
    conteneur conteneurAcharger;
    int choixPortique, i;
    int conteneursAChargerCamion, conteneursADechargerCamion;
    srand(id);

    // printf("Camion %d: arrive au parking\n", id);

    /* Si un des 2 portiques est libre, le camion y va, sinon il fait la queue à un portique choisi au hasard */
    P(portiques[0].semid, CAMION);
    P(portiques[1].semid, CAMION);
    if (portiques[0].camionLibre)
        choixPortique = 0;
    else if (portiques[1].camionLibre)
        choixPortique = 1;
    else
        choixPortique = rand() % NB_PORTIQUE;
    V(portiques[0].semid, CAMION);
    V(portiques[1].semid, CAMION);

    pthread_mutex_lock(&portiques[choixPortique].mutex);
    P(portiques[choixPortique].semid, CAMION);
    if (!portiques[choixPortique].camionLibre) /* Attend l'autorisation du portique pour se garer */
    {
        V(portiques[choixPortique].semid, CAMION);
        pthread_cond_wait(&portiques[choixPortique].arriverCamion, &portiques[choixPortique].mutex);
    }
    else
    {
        V(portiques[choixPortique].semid, CAMION);
    }

    P(portiques[choixPortique].semid, CAMION);

    /* Transmet les informations du camion au portique */
    portiques[choixPortique].camionLibre = false;
    portiques[choixPortique].idCamionAQuai = id;
    portiques[choixPortique].nbConteneursADechargerCamion = nbConteneur;
    portiques[choixPortique].nbConteneursChargesCamion = 0;

    printf("Camion %d: se gare au portique %d\n", id, choixPortique + 1);

    /* Recuère les files de messages du portique */
    if ((conteneursAChargerCamion = msgget(portiques[choixPortique].clefConteneursAChargerCamion, FLAGS)) == -1)
        erreur("Erreur : creation file charger conteneur camion \n");
    if ((conteneursADechargerCamion = msgget(portiques[choixPortique].clefConteneursADechargerCamion, FLAGS)) == -1)
        erreur("Erreur : creation file decharger conteneur camion \n");

    /* Liste tous les conteneurs à décharger */
    for (i = 0; i < nbConteneur; i++)
        msgsnd(conteneursADechargerCamion, &conteneurs[i], sizeof(conteneur), 0);

    V(portiques[choixPortique].semid, CAMION);

    /* Attend d'être totalement chargé pour partir */
    pthread_cond_wait(&portiques[choixPortique].partirCamion, &portiques[choixPortique].mutex);
    for (i = 0; i < NB_MAX_CONTENEURS_CAMION; i++)
    {
        // printf("Debug Camion %d: attend msg\n", id);
        if (msgrcv(conteneursAChargerCamion, &conteneurAcharger, sizeof(conteneur), 1, 0) == -1)
            erreur("Erreur : lecture conteneursAChargerCamion");
        conteneurs[i] = conteneurAcharger;
    }
    // portiques[choixPortique].nbConteneursAChargerCamion = 0;

    P(portiques[choixPortique].semid, CAMION);
    portiques[choixPortique].camionLibre = true;
    portiques[choixPortique].idCamionAQuai = -1;
    V(portiques[choixPortique].semid, CAMION);
    pthread_mutex_unlock(&portiques[choixPortique].mutex);
    printf("Camion %d: est totalement charge, il quitte le parking\n", id);

    /* Liberation de la mémoire des conteneurs */
    free(conteneurs);
}

conteneur *creer_conteneurs(int nbConteneurs, int idVehicule, ville provenance)
{
    conteneur *conteneurs = (conteneur *)malloc(tailles[provenance] * sizeof(conteneur));
    for (int i = 0; i < nbConteneurs; i++)
    {
        conteneurs[i].idConteneur = i;
        conteneurs[i].idVehicule = idVehicule;
        conteneurs[i].contenu = rand() % TAILLE_CONTENEUR;
        conteneurs[i].type = 1;
        do // Verifie que la destination attribué n'est pas la provenance
            conteneurs[i].destination = (ville)rand() % NB_VILLES;
        while (conteneurs[i].destination == provenance);
    }
    return conteneurs;
}