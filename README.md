# SY40_Projet
## Sujet

## Notre solution
- chaque véhicule est un nouveau thread
- un portique est une strucutre qui contrôle :
    - les arrivées des véhicules avec des segments de mémoires partagées (mutex)
    - les conteneurs à charger/décharger avec des files de messages
- un poste de contrôle (thread) manipule le portique : 
    - il manipule la grue du portique en recevant des messages des files `conteneursADecharger` et en les envoyants dans les files `conteneursACharger`