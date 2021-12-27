#ifndef PORTIQUE_HEADER
#define PORTIQUE_HEADER

#include <pthread.h>
#include <unistd.h>
#include <sys/msg.h>
#include "general.h"

/**
 * @brief Créé un poste de contrôle qui manipule un portique (struct)
 *
 * @param portique_p portique à manipuler
 * @param numPortique numéro du portique
 * @return void*
 */
void *creer_post_de_controle(void *portique_p, int numPortique);

#endif /* PORTIQUE_HEADER */