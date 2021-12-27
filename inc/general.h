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
#define FLAGS (0600 | IPC_CREAT)
#define NB_VILLES 3
#define NB_MAX_CONTENEURS_TRAIN 10
#define NB_MAX_CONTENEURS_BATEAU 20
#define TAILLE_CONTENEUR 10000000

static key_t clefsSem[2] = {556, 557};
static struct sembuf sem_oper_P; /* Operation P */
static struct sembuf sem_oper_V; /* Operation V */

typedef enum
{
    false = 0,
    true = 1
} boolean;

/**
 * @brief Enumération des destinations (New york est déservie par bateau, amsterdam par le train et paris par camion)
 *
 */
typedef enum
{
    NEWYORK = 0,
    AMSTERDAM = 1,
    PARIS = 2
} ville;

/**
 * @brief Structure du portique contrôlant les arrivées des véhicules (via segment de mémoire partagée)
 * et le transfert de conteneurs (via file de messages)
 *
 */
typedef struct
{
    boolean bateauLibre;
    boolean trainLibre;
    boolean camionLibre;
    int semid;
    pthread_mutex_t mutex;
    pthread_cond_t arriverCamion;
    pthread_cond_t arriverTrain;
    pthread_cond_t arriverBateau;
    pthread_cond_t partirBateau;
    pthread_cond_t partirTrain;
    pthread_cond_t partirCamion;
    int nbConteneursAChargerBateau;
    int nbConteneursADechargerBateau;
    int nbConteneursAChargerTrain;
    int nbConteneursADechargerTrain;
    int nbConteneursAChargerCamion;
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
    int idVehicule;
    int idConteneur;
    long contenu;
    ville destination;

} conteneur;

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
