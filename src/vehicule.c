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
        choixPortique = rand() % 2; // valeur aléatoire entre 0 et 1
    V(portiques[0].semid, BATEAU);
    V(portiques[1].semid, BATEAU);

    P(portiques[choixPortique].semid, 0);
    if (!portiques[choixPortique].bateauLibre) /* Attend l'autorisation du portique s'amarrer */
    {
        V(portiques[choixPortique].semid, 0);
        pthread_cond_wait(&portiques[choixPortique].arriverBateau, &portiques[choixPortique].mutex);
    }
    else
    {
        V(portiques[choixPortique].semid, 0);
    }

    P(portiques[choixPortique].semid, 0);
    portiques[choixPortique].bateauLibre = false;
    V(portiques[choixPortique].semid, 0);

    portiques[choixPortique].idBateauAQuai = id;
    printf("Bateau %d: s'amarre au portique %d\n", id, choixPortique + 1);

    /* Recuère les files de messages du portique et transmet le nb de conteneur a décharger */
    if ((conteneursAChargerBateau = msgget(portiques[choixPortique].clefConteneursAChargerBateau, FLAGS)) == -1)
        erreur("Erreur : creation file charger conteneur bateau \n");
    if ((conteneursADechargerBateau = msgget(portiques[choixPortique].clefConteneursADechargerBateau, FLAGS)) == -1)
        erreur("Erreur : creation file decharger conteneur bateau \n");
    portiques[choixPortique].nbConteneursADechargerBateau = nbConteneur;

    /* Liste tous les conteneurs à décharger */
    for (i = 0; i < nbConteneur; i++)
        msgsnd(conteneursADechargerBateau, &conteneurs[i], sizeof(conteneur), 0);

    /* Attend d'être totalement chargé pour partir */
    pthread_cond_wait(&portiques[choixPortique].partirBateau, &portiques[choixPortique].mutex);
    nbConteneur = 0;
    for (i = 0; i < NB_MAX_CONTENEURS_BATEAU; i++)
    {
        if (msgrcv(conteneursAChargerBateau, &conteneurAcharger, sizeof(conteneur), 1, 0) == -1)
            erreur("Erreur : lecture conteneursAChargerBateau");
        conteneurs[nbConteneur] = conteneurAcharger;
        nbConteneur++;
    }

    portiques[choixPortique].nbConteneursAChargerBateau = 0;
    portiques[choixPortique].idBateauAQuai = -1;

    printf("Bateau %d: est totalement charge, il quitte le port\n", id);
    P(portiques[choixPortique].semid, 0);
    portiques[choixPortique].bateauLibre = true;
    V(portiques[choixPortique].semid, 0);

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
        choixPortique = rand() % 2; // valeur aléatoire entre 0 et 1
    V(portiques[0].semid, TRAIN);
    V(portiques[1].semid, TRAIN);

    P(portiques[choixPortique].semid, 0);
    if (!portiques[choixPortique].trainLibre) /* Attend l'autorisation du portique s'arrêter */
    {
        V(portiques[choixPortique].semid, 0);
        pthread_cond_wait(&portiques[choixPortique].arriverTrain, &portiques[choixPortique].mutex);
    }
    else
    {
        V(portiques[choixPortique].semid, 0);
    }

    P(portiques[choixPortique].semid, 0);
    portiques[choixPortique].trainLibre = false;
    V(portiques[choixPortique].semid, 0);

    portiques[choixPortique].idTrainAQuai = id;
    printf("Train %d: s'arrete au portique %d\n", id, choixPortique + 1);

    /* Recuère les files de messages du portique et transmet le nb de conteneur a décharger */
    if ((conteneursAChargerTrain = msgget(portiques[choixPortique].clefConteneursAChargerTrain, FLAGS)) == -1)
        erreur("Erreur : creation file charger conteneur train \n");
    if ((conteneursADechargerTrain = msgget(portiques[choixPortique].clefConteneursADechargerTrain, FLAGS)) == -1)
        erreur("Erreur : creation file decharger conteneur train \n");
    portiques[choixPortique].nbConteneursADechargerTrain = nbConteneur;

    /* Liste tous les conteneurs à décharger */
    for (i = 0; i < nbConteneur; i++)
        msgsnd(conteneursADechargerTrain, &conteneurs[i], sizeof(conteneur), 0);

    /* Attend d'être totalement chargé pour partir */
    pthread_cond_wait(&portiques[choixPortique].partirTrain, &portiques[choixPortique].mutex);
    nbConteneur = 0;
    for (i = 0; i < NB_MAX_CONTENEURS_TRAIN; i++)
    {
        if (msgrcv(conteneursAChargerTrain, &conteneurAcharger, sizeof(conteneur), 1, 0) == -1)
            erreur("Erreur : lecture conteneursAChargerTrain");
        conteneurs[nbConteneur] = conteneurAcharger;
        nbConteneur++;
    }
    portiques[choixPortique].nbConteneursAChargerTrain = 0;
    portiques[choixPortique].idTrainAQuai = -1;

    printf("Train %d: est totalement charge, il quitte le port\n", id);
    P(portiques[choixPortique].semid, 0);
    portiques[choixPortique].trainLibre = true;
    V(portiques[choixPortique].semid, 0);

    /* Liberation de la mémoire des conteneurs */
    free(conteneurs);
}

void *creer_camion(void *params)
{
    vehiculeParam *parametre = (vehiculeParam *)params;
    int id = parametre->numVehicule;
    portique *portiques = parametre->portiques;
    int nbConteneur = NB_CONTENEURS_CAMION;
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
        choixPortique = rand() % 2; // valeur aléatoire entre 0 et 1
    V(portiques[0].semid, CAMION);
    V(portiques[1].semid, CAMION);

    P(portiques[choixPortique].semid, 0);
    if (!portiques[choixPortique].camionLibre) /* Attend l'autorisation du portique se garer */
    {
        V(portiques[choixPortique].semid, 0);
        pthread_cond_wait(&portiques[choixPortique].arriverCamion, &portiques[choixPortique].mutex);
    }
    else
    {
        V(portiques[choixPortique].semid, 0);
    }

    P(portiques[choixPortique].semid, 0);
    portiques[choixPortique].camionLibre = false;
    V(portiques[choixPortique].semid, 0);

    portiques[choixPortique].idCamionAQuai = id;
    printf("Camion %d: se gare au portique %d\n", id, choixPortique + 1);

    /* Recuère les files de messages du portique et transmet le nb de conteneur a décharger */
    if ((conteneursAChargerCamion = msgget(portiques[choixPortique].clefConteneursAChargerCamion, FLAGS)) == -1)
        erreur("Erreur : creation file charger conteneur camion \n");
    if ((conteneursADechargerCamion = msgget(portiques[choixPortique].clefConteneursADechargerCamion, FLAGS)) == -1)
        erreur("Erreur : creation file decharger conteneur camion \n");
    portiques[choixPortique].nbConteneursADechargerCamion = nbConteneur;

    /* Liste tous les conteneurs à décharger */
    for (i = 0; i < nbConteneur; i++)
        msgsnd(conteneursADechargerCamion, &conteneurs[i], sizeof(conteneur), 0);

    /* Attend d'être totalement chargé pour partir */
    pthread_cond_wait(&portiques[choixPortique].partirCamion, &portiques[choixPortique].mutex);
    nbConteneur = 0;
    for (i = 0; i < NB_CONTENEURS_CAMION; i++)
    {
        if (msgrcv(conteneursAChargerCamion, &conteneurAcharger, sizeof(conteneur), 1, 0) == -1)
            erreur("Erreur : lecture conteneursAChargerCamion");
        conteneurs[nbConteneur] = conteneurAcharger;
        nbConteneur++;
    }
    portiques[choixPortique].nbConteneursAChargerCamion = 0;
    portiques[choixPortique].idCamionAQuai = -1;

    printf("Camion %d: est totalement charge, il quitte le parking\n", id);
    P(portiques[choixPortique].semid, 0);
    portiques[choixPortique].camionLibre = true;
    V(portiques[choixPortique].semid, 0);

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