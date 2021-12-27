#include "../inc/vehicule.h"

void *creer_bateau(void *portiques_p)
{
    int id = (int)pthread_self();
    int nbConteneur = rand() % NB_MAX_CONTENEURS_BATEAU;
    portique *portiques = (portique *)portiques_p;
    conteneur *conteneurs = creer_conteneurs(nbConteneur, id, NEWYORK);
    conteneur conteneurAcharger;
    int choixPortique, i;
    int conteneursAChargerBateau, conteneursADechargerBateau;
    srand(id);

    printf("Bateau %d: arrive au port\n", id);

    /* Si un des 2 portiques est libre, le bateau y va, sinon il fait la queue à un portique choisi au hasard */
    P(portiques[0].semid, 0);
    P(portiques[1].semid, 0);
    if (portiques[0].bateauLibre)
        choixPortique = 0;
    else if (portiques[1].bateauLibre)
        choixPortique = 1;
    else
        choixPortique = rand() % 2; // valeur aléatoire entre 0 et 1
    V(portiques[0].semid, 0);
    V(portiques[1].semid, 0);

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

    printf("debug bateau 1\n");
    /* Attend d'être totalement chargé pour partir */
    pthread_cond_wait(&portiques[choixPortique].partirBateau, &portiques[choixPortique].mutex);
    printf("debug fin wait\n");
    nbConteneur = 0;
    while (msgrcv(conteneursAChargerBateau, &conteneurAcharger, sizeof(conteneur), 1, 0) != -1)
    {
        conteneurs[nbConteneur] = conteneurAcharger;
        nbConteneur++;
    }
    conteneurs = (conteneur *)realloc(conteneurs, nbConteneur * sizeof(conteneur));
    portiques[choixPortique].nbConteneursAChargerBateau = 0;

    printf("Bateau %d: est totalement charge, il quitte le port\n", id);
    P(portiques[choixPortique].semid, 0);
    portiques[choixPortique].bateauLibre = true;
    V(portiques[choixPortique].semid, 0);

    /* Liberation de la mémoire des conteneurs */
    free(conteneurs);
}

void *creer_train(void *portiques_p)
{
    int id = (int)pthread_self();
    int nbConteneur = rand() % NB_MAX_CONTENEURS_TRAIN;
    portique *portiques = (portique *)portiques_p;
    conteneur *conteneurs = creer_conteneurs(nbConteneur, id, AMSTERDAM);
    srand(id);

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
    conteneur *conteneurs = creer_conteneurs(1, id, PARIS);
    srand(id);

    printf("Camion %d: arrive au parking\n", id);
    /* Attente bateau */
    printf("Camion %d: charge\n", id);
    /* Liberation de la mémoire des conteneurs */
    free(conteneurs);
}

conteneur *creer_conteneurs(int nbConteneurs, int idVehicule, ville provenance)
{
    conteneur *conteneurs = (conteneur *)malloc(nbConteneurs * sizeof(conteneur));
    for (int i = 0; i < nbConteneurs; i++)
    {
        conteneurs[i].idConteneur = i;
        conteneurs[i].idVehicule = idVehicule;
        conteneurs[i].contenu = rand() % TAILLE_CONTENEUR;
        do // Verifie que la destination attribué n'est pas la provenance
            conteneurs[i].destination = (ville)rand() % NB_VILLES;
        while (conteneurs[i].destination != provenance);
    }
    return conteneurs;
}