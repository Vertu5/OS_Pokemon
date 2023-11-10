// imgSearchFunctions.cpp
#include "imgSearchFunctions.h"

volatile bool sigintReceived = false;

// Gestionnaire de signal pour SIGINT
void sigintHandler(int signum) {
    if (signum == SIGINT) {
        sigintReceived = true;
    }
}

// Fonction pour exécuter img-dist et stocker le résultat dans les données partagées
void runImgDist(const std::string& imgDistCmd, const std::string& image1Path, const std::string& image2Path, SharedData* sharedData) {
    std::string commande = imgDistCmd + " " + image1Path + " " + image2Path;

    FILE* pipe = popen(commande.c_str(), "r");
    if (pipe) {
        int status = pclose(pipe);

        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            // Vérifier si le résultat est plus petit que la distance minimale partagée
            if (exit_status < sharedData->minDistance) {
                sharedData->minDistance = exit_status;
                std::strcpy(sharedData->imagePath, image2Path.c_str());
            }

            // Vérifier si le résultat est 0
            if (exit_status == 0) {
                // Quitter immédiatement
                exit(0);
            }
        }
    }
}

// Fonction pour initialiser la mémoire partagée
SharedData* initializeSharedMemory(key_t shmkey) {
    int shmid = shmget(shmkey, sizeof(SharedData), IPC_CREAT | 0666);
    SharedData* sharedData = (SharedData*)shmat(shmid, nullptr, 0);

    if (shmid == -1 || sharedData == (SharedData*)-1) {
        std::cerr << "Erreur lors de la création ou de la liaison à la mémoire partagée." << std::endl;
        exit(1);
    }

    sharedData->minDistance = std::numeric_limits<int>::max();
    sharedData->imagePath[0] = '\0';

    return sharedData;
}

// Fonction pour nettoyer la mémoire partagée
void cleanupSharedMemory(int shmid, SharedData* sharedData) {
    shmdt(sharedData);
    shmctl(shmid, IPC_RMID, nullptr);
}

// Fonction pour effectuer les comparaisons d'images
void performImageComparisons(const std::string& imgDistCmd, const std::string& imageToCompare, const std::vector<std::string>& imagePaths, SharedData* sharedData) {
    for (const std::string& path : imagePaths) {
        if (sigintReceived) {
            exit(0); // Terminer le processus enfant de manière élégante
        }
        runImgDist(imgDistCmd, imageToCompare, path, sharedData);
    }
}

// Fonction pour le mode interactif
std::vector<std::string> handleInteractiveMode(const std::string databasePath) {
    std::vector<std::string> imagePaths;
    std::string imagePath;
    std::cout << "Mode interactif - Entrez les chemins des images un par un (tapez 'exit' pour terminer):\n";
    while (true) {
        if (std::cin >> imagePath) {
            if (imagePath == "exit") {
                break;
            }
            if (imagePath.find(databasePath) != 0) {
                imagePath = databasePath + imagePath;
            }
            if (access(imagePath.c_str(), F_OK) != -1) {
                imagePaths.push_back(imagePath);
            } else {
                std::cerr << "Chemin d'image invalide : " << imagePath << std::endl;
            }
        } else {
            break;
        }
    }
    return imagePaths;
}

// Fonction pour le mode automatique
std::vector<std::string> handleAutomaticMode(const std::string& imageListFilePath, const std::string databasePath) {
    std::vector<std::string> imagePaths;
    FILE* listFile = popen(imageListFilePath.c_str(), "r");
    if (listFile) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), listFile)) {
            buffer[strcspn(buffer, "\n")] = '\0'; // Supprimer le caractère de nouvelle ligne
            std::string imagePath = databasePath + buffer;
            if (listFile) {
                imagePaths.push_back(imagePath);
            }
        }
        pclose(listFile);
    } else {
        std::cerr << "Erreur lors de la lecture de la liste des images." << std::endl;
        exit(1);
    }
    return imagePaths;
}

