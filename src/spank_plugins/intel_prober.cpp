#include <string>
#include <signal.h>
#include <thread>
#include <chrono>
#include <stdlib.h>

bool stop = false;

void end_handler(int sig)
{
    if (sig == SIGTERM)
    {
        stop = true;
    }
}

int main(int argc, char **argv)
{
    signal(SIGTERM, end_handler);
    std::string nodename(argv[1]);
    std::string intel_usage_launch = "intel_gpu_top -c -o " + nodename + ".csv";
    std::string intel_temps_launch = "sensors -j >> " + nodename + "_temps.json";
    std::string frontale = "192.168.1.254";
    std::string username = "front.dalek";
    std::string dest_path = "/home/" + username;
    std::string scp_launch = "scp " + nodename + "_usage.csv " + nodename + "_temps.json " + username + "@" + frontale + ":" + dest_path;

    while (!stop)
    {
        system(intel_usage_launch.c_str());
        system(intel_temps_launch.c_str());
        system(scp_launch.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}