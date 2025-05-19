#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <wait.h>

int stop = 0, enabled = 0;

void sig_handler(int sig)
{
    printf("signal recu : %d\n", sig);
    switch (sig)
    {
    case SIGINT:
    case SIGTERM:
        stop = 1;
        break;

    case SIGUSR1:
        enabled = 1;
        signal(SIGUSR1, sig_handler);
        break;

    case SIGUSR2:
        enabled = 0;
        signal(SIGUSR2, sig_handler);
        break;
    }
}

int main(int argc, char *argv[])
{
    /*
    intervale d'actualisation en ms (-d [delay])
    utilisateur raspi (-u [user])
    hôte raspi (choix de l'étage) (-h [host])
    port (si différent de 22) (-p [port])
    chemin de la fifo de destination (-l [chemin])*/
    signal(SIGTERM, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);
    FILE *proc_stat_stream = fopen("/proc/stat", "r");
    std::string values;
    std::string ssh_user, host, port = "22", link = "/tmp/fifo";
    unsigned long delay = 500; // délai du polling par défaut en ms
    bool found_host = false, found_user = false;

    pid_t current_pid = getpid();
    //printf("PID : %d\n", current_pid);

    int flags, opt;

    sigset_t suspend_set;
    sigemptyset(&suspend_set);

    while ((opt = getopt(argc, argv, "d:u:h:p:l:")) != -1)
    {
        switch (opt)
        {
        case 'd':
            delay = atoi(optarg);
            break;

        case 'u':
            ssh_user = std::string(optarg);
            found_user = true;
            break;

        case 'h':
            host = std::string(optarg);
            found_host = true;
            break;

        case 'p':
            port = std::string(optarg);
            break;

        case 'l':
            link = "\"" + std::string(optarg) + "\"";
            break;
        }
    }

    if (!(found_host && found_user))
    {
        //printf("Missing host or user argument. Exiting.\n");
        exit(-1);
    }

    unsigned long user, user_nice, system_time, idle, total, used_sum;
    unsigned long old_user, old_user_nice, old_system, old_idle;
    unsigned long current_user, current_user_nice, current_system, current_idle;
    unsigned long UC;
    char line[64];
    fscanf(proc_stat_stream, "%s %lu %lu %lu %lu", line, &old_user, &old_user_nice, &old_system, &old_idle);
    while (!stop)
    {
        if (!enabled)
        {
            if (fork() == 0) // on envoie une trame "idle"
            {
                execl("/bin/ssh", "-v", "-p", port.c_str(), (ssh_user + "@" + host).c_str(), "echo", "\"1,0\"", ">>", link.c_str(), (char *)NULL);
            }
            sigsuspend(&suspend_set); // suspend l'envoi des trames ==> noeud allumé mais sans job

            if (stop) // si le signal de réveil est SIGINT ou SIGTERM
            {
                break;
            }

            if (!enabled) // si on a été réveillé par autre chose que les signaux définis, ignorer la boucle
            {
                continue;
            }
        }
        proc_stat_stream = freopen("/proc/stat", "r", proc_stat_stream);
        fscanf(proc_stat_stream, "%s %lu %lu %lu %lu", line, &user, &user_nice, &system_time, &idle);
        current_user = user - old_user;
        current_user_nice = user_nice - old_user_nice;
        current_system = system_time - old_system;
        current_idle = idle - old_idle;

        total = current_user + current_user_nice + current_system + current_idle;
        used_sum = current_user + current_user_nice + current_system;

        old_user = user;
        old_user_nice = user_nice;
        old_system = system_time;
        old_idle = idle;

        if (total <= 0)
        {
            UC = 0;
        }
        else
        {
            UC = ((double)used_sum / (double)total) * 100;
        }

        values = "\"2," + std::to_string(UC) + "\"";

        if (fork() == 0)
        {
            execl("/bin/ssh", "-v", "-p", port.c_str(), (ssh_user + "@" + host).c_str(), "echo", values.c_str(), ">>", link.c_str(), (char *)NULL);
        }

        usleep(delay * 1000);
    }
    pid_t last_ssh;
    if ((last_ssh = fork()) == 0) // la machine va s'eteindre sur SIGTERM ou SIGINT ==> envoi d'une trame "stopped"
    {
        execl("/bin/ssh", "-v", "-p", port.c_str(), (ssh_user + "@" + host).c_str(), "echo", "\"0,0\"", ">>", link.c_str(), (char *)NULL);
    }
    waitpid(last_ssh, NULL, 0);

    
    //printf("sending end\n");
}