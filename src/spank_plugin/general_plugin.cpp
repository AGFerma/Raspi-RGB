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
        system("sudo systemctl kill -s 10 cpu_prober.service");
        switch (nodename[4]) // selon la 5ème lettre du nom, on sait si c'est amd, nvidia, intel
        {
        case 'n': // nvidia

            break;

        case 'a': // amd

            break;

        case 'i': // intel

            break;
        }
        break;

    case S_CTX_ALLOCATOR:
        std::cout << "Contexte allocator (init)" << std::endl;
        break;

    case S_CTX_SLURMD:
        std::cout << "Contexte slurmd (frontale) (init)" << std::endl;
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

    case S_CTX_REMOTE:
        system("sudo systemctl kill -s 12 cpu_prober.service"); // suspension du daemon
        break;

    case S_CTX_ALLOCATOR:
        std::cout << "Contexte allocator (exit)" << std::endl;
        break;

    case S_CTX_SLURMD:

        break;

    case S_CTX_JOB_SCRIPT:
        std::cout << "Contexte job script (exit)" << std::endl;
        break;
    }
    return 0;
}