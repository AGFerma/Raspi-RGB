#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>

int stop = 0;

void sig_handler(int sig)
{
    if (sig == SIGTERM)
    {
        stop = 1;
    }
}

int main(int argc, char *argv[])
{
    /*
    argv[1] : intervale d'actualisation en µs
    argv[2] : utilisateur raspi
    argv[3] : hôte raspi (choix de l'étage)
    argv[4] : numéro du noeud (section du bandeau)*/
    signal(SIGTERM, sig_handler);
    FILE *proc_stat_stream = fopen("/proc/stat", "r");
    std::string cmd;
    std::string values;
    unsigned long delay = 500000;   //délai du polling par défaut

    if (argc >= 1)
    {
        if (argc >= 3)
        {
            cmd = "ssh " + std::string(argv[2]) + "@" + std::string(argv[3]) + "echo ";
        }
        delay = std::stoi(std::string(argv[1]));
    }

    unsigned long user, user_nice, system_time, idle, total, used_sum;
    unsigned long old_user, old_user_nice, old_system, old_idle;
    unsigned long current_user, current_user_nice, current_system, current_idle;
    float UC;
    char line[64];
    fscanf(proc_stat_stream, "%s %lu %lu %lu %lu", line, &old_user, &old_user_nice, &old_system, &old_idle);
    while (!stop)
    {
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

        UC = ((float)used_sum / (float)total) * 100;

        printf("%lu, %lu, %lu, %lu, %lu, %lu ==> %f\n", user, user_nice, system_time, idle, used_sum, total, UC);
        values = "2;" + std::to_string(UC); // + ";" + std::to_string(temperature);

        // system((cmd + values + std::string(" >> node_") + std::string(argv[4])).c_str());

        usleep(delay);
    }
}