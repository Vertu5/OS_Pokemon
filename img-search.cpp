#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib> // Include for std::rand
#include <ctime>   // Include for std::time
#include <sys/ipc.h>
#include <sys/shm.h>
#include <limits> // Include for std::numeric_limits
#include <cstring> // Include for strcpy

// Structure pour la mémoire partagée
struct SharedData {
    int minDistance;   // Distance minimale partagée
    char imagePath[256]; // Chemin de l'image la plus similaire
};

// Fonction pour exécuter img-dist et stocker le résultat dans les résultats
void runImgDist(const std::string& imgDistCmd, const std::string& image1Path, const std::string& image2Path, int& result, char* imagePath) {
    std::string command = imgDistCmd + " " + image1Path + " " + image2Path;

    FILE *pipe = popen(command.c_str(), "r");
    if (pipe) {
        int status = pclose(pipe);

        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            result = exit_status;
            std::strcpy(imagePath, image2Path.c_str());
        }
    }
}

int main() {
    std::string imgDistCmd = "./img-dist/img-dist"; // Assuming img-dist is in the $PATH

    // Créez une mémoire partagée pour stocker la distance minimale et le chemin de l'image
    key_t shmkey = ftok(".", 'a');
    int shmid = shmget(shmkey, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        std::cerr << "Error creating shared memory." << std::endl;
        return 1;
    }

    SharedData* sharedData = (SharedData*)shmat(shmid, nullptr, 0);
    if (sharedData == (SharedData*)-1) {
        std::cerr << "Error attaching to shared memory." << std::endl;
        return 1;
    }

    // Initialise la distance minimale partagée à une valeur maximale
    sharedData->minDistance = std::numeric_limits<int>::max();
    sharedData->imagePath[0] = '\0'; // Initialise le chemin de l'image à une chaîne vide

    // Lire l'image à comparer
    std::string imageToCompare;
    std::cin >> imageToCompare;

    // Lire la liste d'images à comparer
    std::vector<std::string> imagePaths;
    std::string imagePath;

    while (std::cin >> imagePath) {
        // Évitez de comparer l'image avec elle-même
        if (imageToCompare != imagePath) {
            imagePaths.push_back(imagePath);
        }
    }

    // Créez deux processus fils pour comparer l'image avec chaque moitié
    pid_t pid1, pid2;
    pid1 = fork();

    if (pid1 < 0) {
        std::cerr << "Fork failed." << std::endl;
        return 1;
    } else if (pid1 == 0) {
        // Le premier processus fils gère la première moitié des comparaisons
        for (const std::string& path : imagePaths) {
            int result;
            char imagePath[256];
            runImgDist(imgDistCmd, imageToCompare, path, result, imagePath);

            // Vérifiez si le résultat est plus petit que la distance minimale partagée
            // Update shared memory if a smaller distance is found
            if (result < sharedData->minDistance) {
                sharedData->minDistance = result;
                std::strcpy(sharedData->imagePath, path.c_str());
            }
            else if (result == sharedData->minDistance) {
                 // Randomly choose which path to keep
                 if (std::rand() % 2 == 0) {
                    std::strcpy(sharedData->imagePath, path.c_str());
                 }
            }
        }
        exit(0);
    } else {
        pid2 = fork();

        if (pid2 < 0) {
            std::cerr << "Fork failed." << std::endl;
            return 1;
        } else if (pid2 == 0) {
            // Le deuxième processus fils gère la deuxième moitié des comparaisons
            for (const std::string& path : imagePaths) {
                int result;
                char imagePath[256];
                runImgDist(imgDistCmd, imageToCompare, path, result, imagePath);

                // Vérifiez si le résultat est plus petit que la distance minimale partagée
                if (result < sharedData->minDistance) {
                    sharedData->minDistance = result;
                    std::strcpy(sharedData->imagePath, imagePath);
                }
            }
            exit(0);
        } else {
            // C'est le processus parent
            // Attendre que les deux processus fils se terminent
            int status;
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);

            // Affichez l'image la plus similaire
            if (sharedData->imagePath[0] != '\0') {
                std::cout << "Most similar image found: '" << sharedData->imagePath << "' with a distance of " << sharedData->minDistance << "." << std::endl;
            } else {
                std::cout << "No similar image found (no comparison could be performed successfully)." << std::endl;
            }
        }
    }

    // Détachez la mémoire partagée
    shmdt(sharedData);

    // Supprimez la mémoire partagée
    shmctl(shmid, IPC_RMID, nullptr);

    return 0;
}

