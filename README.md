# SY40_Projet
## Sujet

## Notre solution
- chaque véhicule est un nouveau thread
- un portique est une strucutre qui contrôle :
    - les arrivées des véhicules avec des segments de mémoires partagées (mutex)
    - les conteneurs à charger/décharger avec des files de messages
    - les varibles du portiques sont protégés en lecture/écriture par des sémaphores
- un poste de contrôle (thread) manipule le portique : 
    - il manipule la grue du portique en recevant des messages des files `conteneursADecharger` et en les envoyants dans les files `conteneursCharges`
- chaque véhicule possède une destination :
    - bateau = New York
    - train = Amsterdam
    - camion = Paris
- Lorsque le portique ne trouve pas de place libre pour un conteneur:
    - d'un bateau ou d'un train, on le remet dans la file `conteneursAdecharger`, si le portique a essayé de décharger tous les conteneurs restants sans succès, le bateau ou le train part
    - d'un camion, le camion part simplement sans avoir déchargé le conteneur
- Lorsqu'un vehicule n'a pas recu de chargement pendant N manoeuvre de portique, le vehicule quitte le portique
