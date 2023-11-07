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

// Structure pour la mémoire partagée 
struct SharedData {
    int minDistance;   // Distance partagée minimale
    char imagePath[256]; // Chemin vers l'image la plus semblable
};

// Déclaration globale de pid1 et pid2 pour être accessibles pour le gestionnaire de signaux
pid_t pid1, pid2;

// Flag globale pour indiquer la réception de SIGINT
volatile bool sigintReceived = false;

// Gestionnaire de signal pour SIGINT
void sigintHandler(int signum) {
    if (signum == SIGINT) {
        sigintReceived = true;
    }
}

// Fonction pour lancer img-dist et stocker ses résultats
void runImgDist(const std::string& imgDistCmd, const std::string& image1Path, const std::string& image2Path, SharedData* sharedData) {
    std::string commande = imgDistCmd + " " + image1Path + " " + image2Path;

    FILE *pipe = popen(commande.c_str(), "r");
    if (pipe) {
        int status = pclose(pipe);

        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            // Vérification si le résultats est inférieur à la distance partagée minimale
            if (exit_status < sharedData->minDistance) {
                sharedData->minDistance = exit_status;
                std::strcpy(sharedData->imagePath, image2Path.c_str());
            }

            // Vérifier si le résultat est 0
            if (exit_status == 0) {
                // Sortie immédiate
                exit(0);
            }
        }
    }
}
int main(int argc, char* argv[]) {
    // Déclaration du gestionnaire de signaux pour SIGINT
    signal(SIGINT, sigintHandler);

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <mode> <image_to_compare> <database_path>" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    std::string imageToCompare = argv[2];
    std::string databasePath = argv[3];

    std::string imgDistCmd = "./img-dist/img-dist"; // En supposant que img-dist se trouve dans $PATH

    key_t shmkey = ftok(".", 'a');
    int shmid = shmget(shmkey, sizeof(SharedData), IPC_CREAT | 0666);
    SharedData* sharedData = (SharedData*)shmat(shmid, nullptr, 0);

    if (shmid == -1) {
        std::cerr << "Error creating shared memory." << std::endl;
        return 1;
    }

    if (sharedData == (SharedData*)-1) {
        std::cerr << "Error attaching to shared memory." << std::endl;
        return 1;
    }

    sharedData->minDistance = std::numeric_limits<int>::max();
    sharedData->imagePath[0] = '\0';

    std::vector<std::string> imagePaths;
    std::string imageListPath = "./list-file"; // En suppoosant que la list-file se trouve dans le même dossier
    std::string imageListFilePath;

    if (mode == "-i" || mode == "--interactive") {
        // Interactive mode - Mode interactif - lecture immédiate des chemins des images 
        std::string imagePath;
        std::cout << "Interactive mode - Enter image paths one by one (type 'exit' to finish):\n";
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
                    std::cerr << "Invalid image path: " << imagePath << std::endl;
                }
            } else {
                break;
            }
        }
    } else if (mode == "-a" || mode == "--automatic") {
        // Mode automatique - lecture de la liste des images du script list-file
        imageListFilePath = imageListPath + " " + databasePath;
        FILE *listFile = popen(imageListFilePath.c_str(), "r");
        if (listFile) {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), listFile)) {
                buffer[strcspn(buffer, "\n")] = '\0'; // Retire le charactère saut de ligne
                std::string imagePath = databasePath + buffer;
                if (listFile) {
                    imagePaths.push_back(imagePath);
                }
            }
            pclose(listFile);
        } else {
            std::cerr << "Error reading the list of images." << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Invalid mode. Use '-i' or '--interactive' for interactive mode, or '-a' or '--automatic' for automatic mode." << std::endl;
        return 1;
    }

    pid1 = fork();

    if (pid1 < 0) {
        std::cerr << "Fork failed." << std::endl;
        return 1;
    } else if (pid1 == 0) {
        // Le premier processus fils gère la première moitié des comparaisons
        for (const std::string& path : imagePaths) {
            if (sigintReceived) {
                exit(0); // Termine le processus fils de manière propre
            }
            runImgDist(imgDistCmd, imageToCompare, path, sharedData);
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
                if (sigintReceived) {
                    exit(0); // Termine le processus fils de manière propre
                }
                runImgDist(imgDistCmd, imageToCompare, path, sharedData);
            }
            exit(0);
        } else {
            // Processus père
            int status;
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);

            if (sharedData->imagePath[0] != '\0') {
                std::cout << "Most similar image found: '" << sharedData->imagePath << "' with a distance of " << sharedData->minDistance << "." << std::endl;
            } else {
                std::cout << "No similar image found (no comparison could be performed successfully)." << std::endl;
            }

            shmdt(sharedData);
            shmctl(shmid, IPC_RMID, nullptr);

            return 0;
        }
    }
}
