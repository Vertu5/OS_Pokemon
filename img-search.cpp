#include <iostream>
#include <string>
#include <cstdio>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <image_path1> <image_path2>" << std::endl;
        return 1;
    }

    std::string imgDistCmd = "./img-dist/img-dist";
    std::string image1Path = argv[1];
    std::string image2Path = argv[2];
    std::string command = imgDistCmd + " " + image1Path + " " + image2Path;

    std::cout << "Commande à exécuter : " << command << std::endl;

    FILE *pipe = popen(command.c_str(), "r");
    if (pipe) {
        char buffer[128];
        std::string result = "";

        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }

        int status = pclose(pipe);
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            std::cout << "Le processus fils s'est terminé avec le code de sortie : " << exit_status << std::endl;

            // Use the exit_status as needed
            if (exit_status >= 0 && exit_status <= 64) {
                // Exit status is between 0 and 64
                std::cout << "Exit status is within the valid range." << std::endl;
            } else {
                // Handle an out-of-range exit status
                std::cerr << "Invalid exit status: " << exit_status << std::endl;
            }
        } else if (WIFSIGNALED(status)) {
            int term_signal = WTERMSIG(status);
            std::cout << "Le processus fils a été terminé par un signal : " << term_signal << std::endl;
        }

        // Affichez le résultat de img-dist
        std::cout << "Résultat de img-dist : " << result << std::endl;
    } else {
        std::cerr << "Échec de l'exécution de img-dist." << std::endl;
        return 1;
    }

    // Informations sur la fin du processus fils
    std::cout << "Processus fils terminé." << std::endl;

    return 0;
}

