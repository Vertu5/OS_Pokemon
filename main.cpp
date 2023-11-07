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

// Structure for shared memory
struct SharedData {
    int minDistance;   // Shared minimum distance
    char imagePath[256]; // Path of the most similar image
};

// Declare pid1 and pid2 globally to be accessible by the signal handler
pid_t pid1, pid2;

// Global flag to indicate whether a SIGINT was received
volatile bool sigintReceived = false;

// Signal handler for SIGINT
void sigintHandler(int signum) {
    if (signum == SIGINT) {
        sigintReceived = true;
    }
}

// Function to run img-dist and store the result in the results
void runImgDist(const std::string& imgDistCmd, const std::string& image1Path, const std::string& image2Path, SharedData* sharedData) {
    std::string commande = imgDistCmd + " " + image1Path + " " + image2Path;

    FILE *pipe = popen(commande.c_str(), "r");
    if (pipe) {
        int status = pclose(pipe);

        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            // Check if the result is smaller than the shared minimum distance
            if (exit_status < sharedData->minDistance) {
                sharedData->minDistance = exit_status;
                std::strcpy(sharedData->imagePath, image2Path.c_str());
            }

            // Check if the result is 0
            if (exit_status == 0) {
                // Exit immediately
                exit(0);
            }
        }
    }
}
int main(int argc, char* argv[]) {
    // Register the signal handler for SIGINT
    signal(SIGINT, sigintHandler);

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <mode> <image_to_compare> <database_path>" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    std::string imageToCompare = argv[2];
    std::string databasePath = argv[3];

    std::string imgDistCmd = "./img-dist/img-dist"; // Assuming img-dist is in the $PATH

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
    std::string imageListPath = "./list-file"; // Assuming list-file is in the same directory
    std::string imageListFilePath;

    if (mode == "-i" || mode == "--interactive") {
        // Interactive mode - Read image paths interactively
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
        // Automatic mode - Read the list of images from the list-file script
        imageListFilePath = imageListPath + " " + databasePath;
        FILE *listFile = popen(imageListFilePath.c_str(), "r");
        if (listFile) {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), listFile)) {
                buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
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
        // The first child process handles the first half of comparisons
        for (const std::string& path : imagePaths) {
            if (sigintReceived) {
                exit(0); // Terminate the child process gracefully
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
            // The second child process handles the second half of comparisons
            for (const std::string& path : imagePaths) {
                if (sigintReceived) {
                    exit(0); // Terminate the child process gracefully
                }
                runImgDist(imgDistCmd, imageToCompare, path, sharedData);
            }
            exit(0);
        } else {
            // This is the parent process
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
