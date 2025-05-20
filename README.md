# ARGB for power management
Repository for PSESI 2025 : Controlling ARGB with a raspberry pi to get a visual representation of a load in a cluster of computing nodes  

## Table of contents
- [ARGB for power management](#argb-for-power-management)
  - [Table of contents](#table-of-contents)
  - [Summary](#summary)
  - [Compiling](#compiling)
    - [ARGB controller](#argb-controller)
    - [CPU Prober](#cpu-prober)
  - [Running](#running)
    - [ARGB controller](#argb-controller-1)
      - [Arguments description](#arguments-description)
      - [Arguments full example](#arguments-full-example)
    - [CPU Prober](#cpu-prober-1)
      - [As a standalone program](#as-a-standalone-program)
      - [Arguments description](#arguments-description-1)
      - [Behaviour while running](#behaviour-while-running)
      - [As a systemd service](#as-a-systemd-service)


## Summary
Raspi RGB is a project aiming to provide a complete framework for linking CPU and GPU statistics to ARGB LED strips.  
Is it built upon the rpi_ws281x library, which makes use of a Raspberry Pi PWM, SPI, and PCM hardware modules to generate on its GPIOs the required control signals.  
The CPU prober and ARGB controllers are both entirely standalone, and the GPU probers need the varied manufacturers' softwares to get GPU stats.  
Right now, only the CPU prober fully works on any machine after recompiling.  
The ARGB controller works on Raspberry Pi 1-4, as compatibility with model 5 is not guaranteed by the library we used.

## Compiling
### ARGB controller
On the Raspberry Pi, you first need to install the rpi_ws281x by following the instructions from [the library readme](https://github.com/jgarff/rpi_ws281x/blob/master/README.md).  
After that, go to "src/ARGB Controller" and compile the code using the command below:  
```
gcc ARGB_controller.c -o ARGB_controller.x -lws2811 -lm
```

### CPU Prober
On the computer you want the prober to run on, go to "src/Probers" and compile the code using the command below:
```
g++ -O3 CPU_prober.cpp -o CPU_prober.x
```

## Running
### ARGB controller
On the Raspberry Pi, where your executable is located, you can run the controller by using the following format:
```
sudo ./ARGB_controller.x --sections {# of sections} --parallel {Single/Dual channel} --fifo-prefix {Prefix} [--pfifo_prefix {Prefix}] [--l1 {# of LEDs in section} --r1 {red value} --g1 {green value} --b1 {blue value} --m1 {effect} --tcpu1 {light level}] [--pl1 {# of LEDs in section} --pr1 {red value} --pg1 {green value} --pb1 {blue value} --pm1 {effect} --ptcpu1 {light level}]
```

#### Arguments description
- --sections : required, must be between 1 and 6. Allows to divide a single strip in up to 6 different zones
- --parallel : enables a secondary control channel if set to 1. Uses only one if set to 0. Each LED strip needs one channel
- --fifo-prefix : name or path of the Posix pipe that will be read for state and CPU usage for the main channel
- --pfifo-prefix : name or path of the Posix pipe that will be read for state and CPU usage for the secondary channel (only required if --parallel is set to 1)
- For each section of the main channel, with n ranging from 1 to 6 (depends on --sections parameter):
    - --ln : number of LEDs in the section
    - --rn : red intensity from 0 to 255 for the default effect
    - --gn : green intensity from 0 to 255 for the default effect
    - --bn : blue in intensity from 0 to 255 for the default effect
    - --mn : default effect from the following : blink, chase, fill, fillrev, flow, breathe, idle, rainbow, comet, wave (case sensitive)
    - --tcpun : intensity of the light (reflects a simulated CPU temperature) from 0 to 100
- For each section of the secondary channel, with n ranging from 1 to 6 (depends on --sections parameter):
    - --pln : number of LEDs in the section
    - --prn : red intensity from 0 to 255 for the default effect
    - --pgn : green intensity from 0 to 255 for the default effect
    - --pbn : blue in intensity from 0 to 255 for the default effect
    - --pmn : default effect from the following : blink, chase, fill, fillrev, flow, breathe, idle, rainbow, comet, wave (case sensitive)
    - --ptcpun : intensity of the light (reflects a simulated CPU temperature) from 0 to 100

#### Arguments full example
```
sudo ./ARGB_controller.x --sections 6 --parallel 1 \
  --l1 8  --r1 100 --g1 0 --b1 0 --m1 static --tcpu1 80 \
  --l2 8  --r2 0 --g2 255 --b2 0 --m2 blink --tcpu2 70 \
  --l3 8  --r3 0 --g3 0 --b3 255 --m3 chase --tcpu3 60 \
  --l4 8  --r4 128 --g4 128 --b4 0 --m4 fill --tcpu4 50 \
  --l5 8  --r5 0 --g5 128 --b5 128 --m5 fillrev --tcpu5 40 \
  --l6 8  --r6 128 --g6 0 --b6 128 --m6 flow --tcpu6 30 \
  --pl1 8 --pr1 255 --pg1 0 --pb1 0 --pm1 static --ptcpu1 80 \
  --pl2 8 --pr2 0 --pg2 255 --pb2 0 --pm2 blink --ptcpu2 70 \
  --pl3 8 --pr3 0 --pg3 0 --pb3 255 --pm3 chase --ptcpu3 60 \
  --pl4 8 --pr4 128 --pg4 128 --pb4 0 --pm4 fill --ptcpu4 50 \
  --pl5 8 --pr5 0 --pg5 128 --pb5 128 --pm5 fillrev --ptcpu5 40 \
  --pl6 8 --pr6 128 --pg6 0 --pb6 128 --pm6 flow --ptcpu6 30 \
  --fifo-prefix fifo --pfifo-prefix fifop
```

### CPU Prober
#### As a standalone program
To run the CPU prober, go to the folder where the executable is and run the following command : 
```
./CPU_prober.x -d {delay in ms} -h {host} -u{user} -p {port} -l {path to pipe}
```
#### Arguments description
- -d : probing period. By default 500ms
- -h : host where the RGB controller is. If on the same machine, put localhost (required argument)
- -u : username of the account running the RBG controller. (required argument)
- -p : ssh port. By default 22
- -l : either absolute or relative path to the pipe used for sending CPU usage and state. Usually in the same folder as the RGB controller

#### Behaviour while running
By default, the prober sends a packet of the form "1,0" to indicate an idle state. It will pause after sending this packet.
To unpause it and make it send data packets of the form "2,xxx", one has to send a SIGUSR1, either using the kill command, or using another program. The PID has to be known, and as such it is useful to launch the prober as detached from the terminal.  
To pause packet sending and go back to idle mode, send a SIGUSR2.  
The program sends a packet of the form "0,0" and closes upon receiving either a SIGINT or a SIGTERM.

#### As a systemd service
The cpu prober can be run as systemd service.
For this purpose, you can edit the file found in src/Probers called "cpu_prober.service" to match all your required arguments. Note that the user and group has to be the one with access to your ssh keys linked to your Raspberry Pi.
Copy the modified file to /etc/systemd/system, then in a terminal run the following commands : 
```
sudo systemctl daemon-reload
```
After that, you can run `sudo systemctl start cpu_prober.service` to laucnh the prober, and `sudo systemctl stop cpu_prober.service` to stop the service.  
Run `sudo systemctl kill -s 10 cpu_prober.service` to start the periodic sending of packets, and run `sudo systemctl kill -s 12 cpu_prober.service` to pause the sending of packets.