#include "../inc/vehicule.h"

void *creer_bateau(void *params)
{
    vehiculeParam *parametre = (vehiculeParam *)params;
    int id = parametre->numVehicule;
    portique *portiques = parametre->portiques;
    int nbConteneur = rand() % NB_MAX_CONTENEURS_BATEAU + 1;
    conteneur *conteneurs = creer_conteneurs(nbConteneur, id, NEWYORK);
    conteneur conteneurAcharger;
    int choixPortique, i;
    int conteneursAChargerBateau, conteneursADechargerBateau;
    srand(id);

    // printf("Bateau %d: arrive au port\n", id);

    /* Si un des 2 portiques est libre, le bateau y va, sinon il fait la queue à un portique choisi au hasard */
    P(portiques[0].semId, BATEAU);
    P(portiques[1].semId, BATEAU);
    if (portiques[0].bateauLibre)
        choixPortique = 0;
    else if (portiques[1].bateauLibre)
        choixPortique = 1;
    else
        choixPortique = rand() % NB_PORTIQUE;
    V(portiques[0].semId, BATEAU);
    V(portiques[1].semId, BATEAU);

    pthread_mutex_lock(&portiques[choixPortique].mutexBateau);
    P(portiques[choixPortique].semId, 0);
    if (!portiques[choixPortique].bateauLibre) /* Attend l'autorisation du portique s'amarrer */
    {
        V(portiques[choixPortique].semId, BATEAU);
        pthread_cond_wait(&portiques[choixPortique].arriverBateau, &portiques[choixPortique].mutexBateau);
    }
    else
    {
        V(portiques[choixPortique].semId, BATEAU);
    }

    P(portiques[choixPortique].semId, BATEAU);

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

    V(portiques[choixPortique].semId, BATEAU);

    /* Attend d'être totalement chargé pour partir */
    pthread_cond_wait(&portiques[choixPortique].partirBateau, &portiques[choixPortique].mutexBateau);
    for (i = 0; i < portiques[choixPortique].nbConteneursChargesBateau; i++)
    {
        if (msgrcv(conteneursAChargerBateau, &conteneurAcharger, sizeof(conteneur), 1, 0) == -1)
            erreur("Erreur : lecture conteneursAChargerBateau");
        conteneurs[i] = conteneurAcharger;
    }

    P(portiques[choixPortique].semId, BATEAU);
    portiques[choixPortique].bateauLibre = true;
    portiques[choixPortique].idBateauAQuai = -1;
    V(portiques[choixPortique].semId, BATEAU);
    pthread_mutex_unlock(&portiques[choixPortique].mutexBateau);
    printf("Bateau %d: quitte le port\n", id);

    /* Liberation de la mémoire des conteneurs */
    free(conteneurs);

    return NULL;
}

void *creer_train(void *params)
{
    vehiculeParam *parametre = (vehiculeParam *)params;
    int id = parametre->numVehicule;
    portique *portiques = parametre->portiques;
    int nbConteneur = rand() % NB_MAX_CONTENEURS_TRAIN + 1;
    conteneur *conteneurs = creer_conteneurs(nbConteneur, id, AMSTERDAM);
    conteneur conteneurAcharger;
    int choixPortique, i;
    int conteneursAChargerTrain, conteneursADechargerTrain;
    srand(id);

    // printf("Train %d: arrive a quai\n", id);

    /* Si un des 2 portiques est libre, le Train y va, sinon il fait la queue à un portique choisi au hasard */
    P(portiques[0].semId, TRAIN);
    P(portiques[1].semId, TRAIN);
    if (portiques[0].trainLibre)
        choixPortique = 0;
    else if (portiques[1].trainLibre)
        choixPortique = 1;
    else
        choixPortique = rand() % NB_PORTIQUE;
    V(portiques[0].semId, TRAIN);
    V(portiques[1].semId, TRAIN);

    pthread_mutex_lock(&portiques[choixPortique].mutexTrain);
    P(portiques[choixPortique].semId, TRAIN);
    if (!portiques[choixPortique].trainLibre) /* Attend l'autorisation du portique s'arrêter */
    {
        V(portiques[choixPortique].semId, TRAIN);
        pthread_cond_wait(&portiques[choixPortique].arriverTrain, &portiques[choixPortique].mutexTrain);
    }
    else
    {
        V(portiques[choixPortique].semId, TRAIN);
    }

    P(portiques[choixPortique].semId, TRAIN);

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

    V(portiques[choixPortique].semId, TRAIN);

    /* Attend d'être totalement chargé pour partir */
    pthread_cond_wait(&portiques[choixPortique].partirTrain, &portiques[choixPortique].mutexTrain);
    for (i = 0; i < portiques[choixPortique].nbConteneursChargesTrain; i++)
    {
        // printf("Debug Train %d: attend msg\n", id);
        if (msgrcv(conteneursAChargerTrain, &conteneurAcharger, sizeof(conteneur), 1, 0) == -1)
            erreur("Erreur : lecture conteneursAChargerTrain");
        conteneurs[i] = conteneurAcharger;
    }

    P(portiques[choixPortique].semId, TRAIN);
    portiques[choixPortique].trainLibre = true;
    portiques[choixPortique].idTrainAQuai = -1;
    V(portiques[choixPortique].semId, TRAIN);
    pthread_mutex_unlock(&portiques[choixPortique].mutexTrain);
    printf("Train %d: quitte la gare\n", id);

    /* Liberation de la mémoire des conteneurs */
    free(conteneurs);
    return NULL;
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
    P(portiques[0].semId, CAMION);
    P(portiques[1].semId, CAMION);
    if (portiques[0].camionLibre)
        choixPortique = 0;
    else if (portiques[1].camionLibre)
        choixPortique = 1;
    else
        choixPortique = rand() % NB_PORTIQUE;
    V(portiques[0].semId, CAMION);
    V(portiques[1].semId, CAMION);

    pthread_mutex_lock(&portiques[choixPortique].mutexCamion);
    P(portiques[choixPortique].semId, CAMION);
    if (!portiques[choixPortique].camionLibre) /* Attend l'autorisation du portique pour se garer */
    {
        V(portiques[choixPortique].semId, CAMION);
        pthread_cond_wait(&portiques[choixPortique].arriverCamion, &portiques[choixPortique].mutexCamion);
    }
    else
    {
        V(portiques[choixPortique].semId, CAMION);
    }

    P(portiques[choixPortique].semId, CAMION);

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

    V(portiques[choixPortique].semId, CAMION);

    /* Attend d'être totalement chargé pour partir */
    pthread_cond_wait(&portiques[choixPortique].partirCamion, &portiques[choixPortique].mutexCamion);
    P(portiques[choixPortique].semId, CAMION);
    for (i = 0; i < portiques[choixPortique].nbConteneursChargesCamion; i++)
    {
        // printf("Debug Camion %d: attend msg\n", id);
        if (msgrcv(conteneursAChargerCamion, &conteneurAcharger, sizeof(conteneur), 1, 0) == -1)
            erreur("Erreur : lecture conteneursAChargerCamion");
        conteneurs[i] = conteneurAcharger;
    }

    portiques[choixPortique].camionLibre = true;
    portiques[choixPortique].idCamionAQuai = -1;
    V(portiques[choixPortique].semId, CAMION);

    pthread_mutex_unlock(&portiques[choixPortique].mutexCamion);
    printf("Camion %d: quitte le parking\n", id);

    /* Liberation de la mémoire des conteneurs */
    free(conteneurs);
    return NULL;
}

conteneur *creer_conteneurs(int nbConteneurs, int idVehicule, ville provenance)
{
    int destinatinPleine = false;
    conteneur *conteneurs = (conteneur *)malloc(taillesVehicule[provenance] * sizeof(conteneur));
    for (int i = 0; i < nbConteneurs; i++)
    {
        conteneurs[i].idConteneur = i;
        conteneurs[i].idVehicule = idVehicule;
        conteneurs[i].contenu = rand() % TAILLE_CONTENEUR;
        conteneurs[i].type = 1;

        do
        { /* Verifie que la destination attribué n'est pas la provenance et qu'on ne créer pas plus de conteneurs pour une destination que de capacite pour cette même destination*/
            conteneurs[i].destination = (ville)rand() % NB_VILLES;
            // P(semCompteurId, conteneurs[i].destination);
            // if (compteurConteneurVehicules[conteneurs[i].destination] >= taillesVehicule[conteneurs[i].destination] * nbVehicule[conteneurs[i].destination])
            //     destinatinPleine = true;
            // else
            //     destinatinPleine = false;
        } while (conteneurs[i].destination == provenance || destinatinPleine);
        // compteurConteneurVehicules[conteneurs[i].destination]++;
        // V(semCompteurId, conteneurs[i].destination);
        // printf("%d : compteurConteneur %d\n", conteneurs[i].destination, compteurConteneurVehicules[conteneurs[i].destination]);
    }
    return conteneurs;
}