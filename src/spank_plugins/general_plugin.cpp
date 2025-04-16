#include <slurm/spank.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <signal.h>

int slurm_spank_init(spank_t spank, int argc, char *argv[])
{
    spank_context_t calling_context = spank_context(); // renvoie un élément de l'enum (on s'intéresse surtout à local, remote, et slurmd)
    char nodename_env_var[] = "SLURMD_NODENAME";
    char *nodename;
    nodename = getenv(nodename_env_var);

    std::fstream pid_file;
    pid_t graphics_stats_prober, aggregator;

    switch (calling_context)
    {
    case S_CTX_ERROR:
        std::cout << "Erreur de contexte (init)" << std::endl;
        break;
    case S_CTX_LOCAL:
        std::cout << "Contexte local (init)" << std::endl;
        break;
    case S_CTX_REMOTE:
        std::cout << "Contexte remote (init)" << std::endl;
        switch (nodename[4]) // selon la 5ème lettre du nom, on sait si c'est amd, nvidia, intel
        {
        case 'n': // nvidia
            if (!(graphics_stats_prober = fork()))
            {
                execl("nvidia_prober.x", nodename, (char*)NULL);
            }
            else // sauvegarde du pid pour kill la tâche lors de exit
            {
                pid_file.open("pid_file.txt", std::ios::out);
                pid_file << graphics_stats_prober << '\n'
                         << std::flush;
                pid_file.close();
            }
            break;

        case 'a': // amd
            if (!(graphics_stats_prober = fork()))
            {
                execl("amd_prober.x", nodename, (char*)NULL);
            }
            else // sauvegarde du pid pour kill la tâche lors de exit
            {
                pid_file.open("pid_file.txt", std::ios::out);
                pid_file << graphics_stats_prober << '\n'
                         << std::flush;
                pid_file.close();
            }
            break;

        case 'i': // intel
            if (!(graphics_stats_prober = fork()))
            {
                execl("intel_prober.x", nodename, (char*)NULL);
            }
            else // sauvegarde du pid pour kill la tâche lors de exit
            {
                pid_file.open("pid_file.txt", std::ios::out);
                pid_file << graphics_stats_prober << '\n'
                         << std::flush;
                pid_file.close();
            }
            break;
        }
        break;

    case S_CTX_ALLOCATOR:
        std::cout << "Contexte allocator (init)" << std::endl;
        break;

    case S_CTX_SLURMD:
        std::cout << "Contexte slurmd (frontale) (init)" << std::endl;
        if (!(aggregator = fork()))
        {
            execl("frontale.x", nodename, (char*)NULL);
        }
        else // sauvegarde du pid pour kill la tâche lors de exit
        {
            pid_file.open("pid_file.txt", std::ios::out);
            pid_file << aggregator << '\n'
                     << std::flush;
            pid_file.close();
        }
        break;

    case S_CTX_JOB_SCRIPT:
        std::cout << "Contexte job script (init)" << std::endl;
        break;
    }
    return 0;
}

int slurm_spank_exit(spank_t spank, int argc, char *argv[])
{
    spank_context_t calling_context = spank_context(); // renvoie un élément de l'enum
    std::fstream pid_file;
    pid_t graphics_stats_prober, aggregator;
    std::string pid_str;

    switch (calling_context)
    {
    case S_CTX_ERROR:
        std::cout << "Erreur de contexte (exit)" << std::endl;
        break;

    case S_CTX_LOCAL:
        std::cout << "Contexte local (exit)" << std::endl;
        break;

    case S_CTX_REMOTE:  //kill le prober
        std::cout << "Contexte remote (exit)" << std::endl;
        pid_file.open("pid_file.txt");
        std::getline(pid_file, pid_str);
        pid_file.close();
        graphics_stats_prober = std::stoi(pid_str);
        kill(graphics_stats_prober, SIGTERM);
        break;

    case S_CTX_ALLOCATOR:
        std::cout << "Contexte allocator (exit)" << std::endl;
        break;

    case S_CTX_SLURMD:  //kill l'agrégateur
        std::cout << "Contexte slurmd (frontale) (exit)" << std::endl;
        std::cout << "Contexte remote (exit)" << std::endl;
        pid_file.open("pid_file.txt");
        std::getline(pid_file, pid_str);
        pid_file.close();
        aggregator = std::stoi(pid_str);
        kill(aggregator, SIGTERM);
        break;

    case S_CTX_JOB_SCRIPT:
        std::cout << "Contexte job script (exit)" << std::endl;
        break;
    }
    return 0;
}