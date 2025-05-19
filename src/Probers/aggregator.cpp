#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

bool stop = 0;

int main()
{
    while(1)
    {
        /*Tant que la frontale tourne, lire tous les fichiers d'information, agréger dans un fichier par étage, puis envoyer*/
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    }
    return 0;
}