// imgSearchFunctions.h
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <limits>
#include <cstring>
#include <csignal>

// Structure pour stocker les données partagées entre les processus
struct SharedData {
    int minDistance;        // Disstance minimale partagée
    char imagePath[256];    // Chemin de l'image la plus similaire
};

// Gestionnaire de signal pour SIGINT
void sigintHandler(int signum);

// Fonction pour exécuter img-dist et stocker le résultat dans les données partagées
void runImgDist(const std::string& imgDistCmd, const std::string& image1Path, const std::string& image2Path, SharedData* sharedData);

// Fonction pour initialiser la mémoire partagée
SharedData* initializeSharedMemory(key_t shmkey);

// Fonction pour nettoyer la mémoire partagée
void cleanupSharedMemory(int shmid, SharedData* sharedData);

// Fonction pour effectuer les comparaisons d'images
void performImageComparisons(const std::string& imgDistCmd, const std::string& imageToCompare, const std::vector<std::string>& imagePaths, SharedData* sharedData);

// Fonction pour le mode interactif
std::vector<std::string> handleInteractiveMode(const std::string databasePath);

// Fonction pour le mode automatique
std::vector<std::string> handleAutomaticMode(const std::string& imageListFilePath, const std::string databasePath);

