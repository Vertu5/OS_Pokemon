// main.cpp
#include "imgSearchFunctions.h"

int main(int argc, char* argv[]) {
    
    // Enregistrez le gestionnaire de signal pour SIGINT
    signal(SIGINT, sigintHandler);

    // Vérifiez le nombre d'arguments en ligne de commande
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <mode> <image_to_compare> <database_path>" << std::endl;
        return 1;
    }

    // Récupérez les arguments en ligne de commande
    std::string mode = argv[1];
    std::string imageToCompare = argv[2];
    std::string databasePath = argv[3];

    // Construisez la commande pour img-dist
    std::string imgDistCmd = "./img-dist/img-dist"; // Supposons que img-dist est dans le chemin d'exécution

    // Générer la clé pour la mémoire partagée
    key_t shmkey = ftok(".", 'a');

    // Initialisez la mémoire partagée
    SharedData* sharedData = initializeSharedMemory(shmkey);

    // Générez la liste des chemins d'images en fonction du mode
    std::vector<std::string> imagePaths;
    if (mode == "-i" || mode == "--interactive") {
        // Mode interactif - Laissez l'utilisateur entrer les chemins des images
        imagePaths = handleInteractiveMode(databasePath);
    } else if (mode == "-a" || mode == "--automatic") {
        // Mode automatique - Lisez la liste des images à partir du fichier list-file
        std::string imageListPath = "./list-file"; // Supposons que list-file est dans le même répertoire
        std::string imageListFilePath = imageListPath + " " + databasePath;
        imagePaths = handleAutomaticMode(imageListFilePath, databasePath);
    } else {
        // Mode invalide
        std::cerr << "Mode invalide. Utilisez '-i' ou '--interactive' pour le mode interactif, ou '-a' ou '--automatic' pour le mode automatique." << std::endl;
        cleanupSharedMemory(shmkey, sharedData);
        return 1;
    }

    // Fork et effectuez les comparaisons d'images dans des processus enfants
    pid_t pid1 = fork();

    if (pid1 < 0) {
        std::cerr << "Fork a échoué." << std::endl;
        cleanupSharedMemory(shmkey, sharedData);
        return 1;
    } else if (pid1 == 0) {
        // Premier processus enfant gère la première moitié des comparaisons
        performImageComparisons(imgDistCmd, imageToCompare, imagePaths, sharedData);
        exit(0);
    } else {
        // Processus parent
        pid_t pid2 = fork();
        if (pid2 < 0) {
            std::cerr << "Fork a échoué." << std::endl;
            cleanupSharedMemory(shmkey, sharedData);
            return 1;
        } else if (pid2 == 0) {
            // Deuxième processus enfant gère la deuxième moitié des comparaisons
            performImageComparisons(imgDistCmd, imageToCompare, imagePaths, sharedData);
            exit(0);
        } else {
            // Processus parent attend les processus enfants
            int status;
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);

            // Affiche le résultat basé sur les données partagées
            if (sharedData->imagePath[0] != '\0') {
                std::cout << "Most similar image found: '" << sharedData->imagePath << "' with a distance of " << sharedData->minDistance << "." << std::endl;
            } else {
                std::cout << "No similar image found (no comparison could be performed successfully)." << std::endl;
            }
            
            // Nettoie la mémoire partagée
            cleanupSharedMemory(shmkey, sharedData);

            return 0;
        }
    }
}

