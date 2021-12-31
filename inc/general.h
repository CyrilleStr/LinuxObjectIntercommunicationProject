#ifndef GENERAL_HEADER
#define GENERAL_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/msg.h>

#define CLE_FILE_CONTENEUR_BATEAU (key_t)314
#define CLE_FILE_CONTENEUR_CAMION (key_t)315
#define CLE_FILE_ATTENTE_TRAIN (key_t)316
#define SKEY (key_t) IPC_PRIVATE

#define NB_BATEAU 5
#define NB_CAMION 40
#define NB_TRAIN 5
#define NB_PORTIQUE 2
#define FLAGS (0600 | IPC_CREAT)
#define NB_VILLES 3
#define NB_MAX_CONTENEURS_TRAIN 10
#define NB_MAX_CONTENEURS_BATEAU 5
#define NB_MAX_CONTENEURS_CAMION 1
#define TAILLE_CONTENEUR 10000000
#define TEMPS_MANOEUVRE_PORTIQUE 400000

static struct sembuf sem_oper_P; /* Operation P */
static struct sembuf sem_oper_V; /* Operation V */
static key_t clefsSem[2] = {556, 557};
static int tailles[NB_VILLES] = {NB_MAX_CONTENEURS_BATEAU, NB_MAX_CONTENEURS_TRAIN, NB_MAX_CONTENEURS_CAMION};
static int compteurIds;

typedef enum
{
    false = 0,
    true = 1
} boolean;

/**
 * @brief Enum des destinations (New york est déservie par bateau, amsterdam par le train et paris par camion)
 *
 */
typedef enum
{
    NEWYORK = 0,
    AMSTERDAM = 1,
    PARIS = 2
} ville;

/**
 * @brief Enum des vehicule
 *
 */
typedef enum
{
    BATEAU = 0,
    TRAIN = 1,
    CAMION = 2
} vehicule;

/**
 * @brief Structure du portique contrôlant les arrivées des véhicules (via segment de mémoire partagée)
 * et le transfert de conteneurs (via file de messages)
 *
 */
typedef struct
{
    int numPortique;
    int semid;
    int idBateauAQuai;
    int idTrainAQuai;
    int idCamionAQuai;
    boolean bateauLibre;
    boolean trainLibre;
    boolean camionLibre;
    pthread_mutex_t mutex;
    pthread_cond_t arriverCamion;
    pthread_cond_t arriverTrain;
    pthread_cond_t arriverBateau;
    pthread_cond_t partirBateau;
    pthread_cond_t partirTrain;
    pthread_cond_t partirCamion;
    pthread_cond_t bateauEstParti;
    pthread_cond_t trainEstParti;
    pthread_cond_t camionEstParti;
    int nbConteneursChargesBateau;
    int nbConteneursADechargerBateau;
    int nbConteneursChargesTrain;
    int nbConteneursADechargerTrain;
    int nbConteneursChargesCamion;
    int nbConteneursADechargerCamion;
    key_t clefConteneursAChargerBateau;
    key_t clefConteneursADechargerBateau;
    key_t clefConteneursAChargerTrain;
    key_t clefConteneursADechargerTrain;
    key_t clefConteneursAChargerCamion;
    key_t clefConteneursADechargerCamion;
} portique;

/**
 * @brief Structure de conteneur
 *
 */
typedef struct
{
    long type;
    int idVehicule;
    int idConteneur;
    long contenu;
    ville destination;

} conteneur;

/**
 * @brief Paramêtres nécessaires pour les fonctions creer_vehicule
 *
 */
typedef struct
{
    portique *portiques;
    int numVehicule;
} vehiculeParam;

static void erreur(const char *msg)
{
    perror(msg);
    exit(1);
}

static void P(int semid, int semnum)
{
    int val_sem = semctl(semid, semnum, GETVAL);
    sem_oper_P.sem_num = semnum;
    sem_oper_P.sem_op = val_sem + 1;
    sem_oper_P.sem_flg = 0;
    semop(semid, &sem_oper_P, 1);
}

static void V(int semid, int semnum)
{
    int val_sem = semctl(semid, semnum, GETVAL);
    sem_oper_V.sem_num = semnum;
    sem_oper_V.sem_op = val_sem - 1;
    sem_oper_V.sem_flg = 0;
    semop(semid, &sem_oper_V, 1);
}

static int initsem(key_t semkey)
{

    int status = 0;
    int semid_init;
    union semun
    {
        int val;
        struct semid_ds *semop;
        ushort *array;
    } ctl_arg;
    if ((semid_init = semget(semkey, 3, (0600 | IPC_CREAT))) > 0)
    {

        ushort array[3] = {0, 0, 0};
        ctl_arg.array = array;
        status = semctl(semid_init, 0, SETALL, ctl_arg);
    }
    if (semid_init == -1 || status == -1)
    {
        perror("Erreur initsem");
        return (-1);
    }
    else
        return (semid_init);
}

#endif
