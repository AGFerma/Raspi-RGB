# ARGB for power management
Repository for PSESI 2025 : Controlling ARGB with a raspberry pi to get a visual representation of a load in a cluster of computing nodes  

## Table of contents
- [ARGB for power management](#argb-for-power-management)
  - [Table of contents](#table-of-contents)
  - [Summary](#summary)
  - [Technical objectives by order of priority](#technical-objectives-by-order-of-priority)
  - [Further developments](#further-developments)
  - [Controlling the RGB strips](#controlling-the-rgb-strips)
    - [The WS2812B protocol](#the-ws2812b-protocol)
    - [Hardware modules on the Raspberry Pi](#hardware-modules-on-the-raspberry-pi)
    - [PWM module](#pwm-module)
      - [](#)
  - [Gathering and formatting the data](#gathering-and-formatting-the-data)
    - [](#-1)


## Summary
As computing power advances, so does the power consumption linked to it.  
Despite all the advancements in power efficiency, semiconductors still need a lot of power to operate, even when not actively used.  
Targeting this idle use of power, we are tasked with two main objectives : 
1. Using a raspberry pi, we shall implement algorithms and drivers that are able to control ARGB (short for Adressable Red-Green-Blue) LED strips depending on the current load of a given node, group of nodes, or the entire cluster;
2. Using the same raspberry pi to order a node to shut down if the load gets low enough, and turn it back on as needed.


## Technical objectives by order of priority
1. Controlling the ARGB strips
   - [x] Making one LED work : controlling its perceived intensity (through PWM) and color, as well as other effects like breathing, blinking, etc.
   - [x] Making all the LEDs of a strip work at once : ensuring that the addressing part of our algorithms work properly, ie if we want only one LED on in the middle, it must be alone in that state, and at the right place
   - [x] Interfacing multiples strips at the same time : either using more pins of the Raspberry Pi or multiplexing the output in order to control the strips of each level independantly
2. Gathering data from the cluster
   - [ ] ~~Polling approach : make the Raspberry Pi connect to the cluster and ask it about its activity level on a periodic basis~~
   OR
   - [ ] Interrupt approach : make each cluster send data to the Raspberry pi periodically~~, when there is a big enough activity change, including turnin on/off the cluster, and hysteresis management~~
3. Putting it all together
   - [ ] Using the gathered data to control the strips : periodically update the pattern (polling) or change the pattern on demand (interrupt)
4. Controlling the cluster
   - [x] Effectively ordering shut downs without disturbing the normal function of the cluster
   - [ ] ~~Using the gathered data from previous steps to establish the conditions for ordering a shutdown or a startup~~
   - [ ] Establishing a user-friendly interface to customize varied parameters 

## Further developments
The aforementionned developments will be carried out at user level, and will not touch the kernel in any way.  
However, if time allows it, it may be useful to develop a custom kernel module in order to maybe reduce the overhead and lower, even marginally, the cpu usage of the raspberry pi.

## Controlling the RGB strips
### The WS2812B protocol
In this project, we intend to control an adressable LED strip that follows the WS2812B specification : using 4 wires per LED (VDD, GND, Data_In, Data_Out), it is possible to feed each LED with 24 bits (8 per color) using a PWM encoding. This encoding requires a frequency of 800kHz (1.25µs/bit), and represents the logical 0 by a duty cycle of 32% +- 12%, and the logical 1 by a duty cycle of 64% +- 12%. As such, each LED requires 30µs to receive all its control bits.  
After receiving those 24 bits, an LED starts to act as a passthrough, letting all subsequent bits go from Data_In to Data_Out, until a reset code (50µs or more at electrical level of GND) is sent.  
### Hardware modules on the Raspberry Pi
Since timings need to be extremely tight and regular, wa can't simply drive the GPIOs through software alone.  
Instead, we need to use one or more hardware peripherals that ensure a steady clock, including most notably the PWM, PCM, or SPI modules.  
We shall control those modules through the WiringPi library, which offers easy interfaces to hardware modules, without requiring a kernel driver.
### PWM module

#### 

## Gathering and formatting the data
###