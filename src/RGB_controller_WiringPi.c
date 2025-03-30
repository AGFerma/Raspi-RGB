#include <wiringPi.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>


int Duty_Cycles[2] = {8, 15};
unsigned int Current_DT = 0;
unsigned char stop = 0;

static void set_new_DT(void)
{
    // change Duty cycle
    // printf("Interrupt PWM\n");
    pwmWrite(12, Duty_Cycles[Current_DT]);
    Current_DT = 1 - Current_DT; // oscillates between 0 and 1
}

void stopHandler(int sig)
{
    printf("SIGINT\n");
    stop = 1;
}

int main()
{
    volatile int a = 0;
    signal(SIGINT, stopHandler);
    wiringPiSetupPhys();
    pwmSetRange(24);                       // range = 12
    pwmSetClock(2);                 // main divider = 2 ==> 9.6MHz
    pwmWrite(12, Duty_Cycles[Current_DT]); // 8 for 0, 15 for 1
    Current_DT = 1 - Current_DT;           // oscillates between 0 and 1

    wiringPiISR(36, INT_EDGE_FALLING, set_new_DT); // interrupt to set new DT
    pinMode(12, PWM_MS_OUTPUT);                    // PWM_OUTPUT or PWM_MS_OUTPUT or PWM_BAL_OUTPUT
    // pinMode(36, INPUT);

    while (stop != 1)
    {
        sleep(1);
    }

    printf("Ending program\n");

    wiringPiISRStop(36);
    pinMode(36, PM_OFF);
    pinMode(12, PM_OFF);

    return 0;
}