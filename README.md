# ARGB for power management
Repository for PSESI 2025 : Controlling ARGB with a raspberry pi to get a visual representation of a load in a cluster of computing nodes  

## Table of contents
- [ARGB for power management](#argb-for-power-management)
  - [Table of contents](#table-of-contents)
  - [Summary](#summary)
  - [Technical objectives by order of priority](#technical-objectives-by-order-of-priority)
  - [Further developments](#further-developments)


## Summary
As computing power advances, so does the power consumption linked to it.  
Despite all the advancements in power efficiency, semiconductors still need a lot of power to operate, even when not actively used.  
Targeting this idle use of power, we are tasked with two main objectives : 
1. Using a raspberry pi, we shall implement algorithms and drivers that are able to control ARGB (short for Adressable Red-Green-Blue) LED strips depending on the current load of a given node, group of nodes, or the entire cluster;
2. Using the same raspberry pi to order a node to shut down if the load gets low enough, and turn it back on as needed.


## Technical objectives by order of priority
1. Controlling the ARGB strips
   - [ ] Making one LED work : controlling its perceived intensity (through PWM) and color, as well as other effects like breathing, blinking, etc.
   - [ ] Making all the LEDs of a strip work at once : ensuring that the addressing part of our algorithms work properly, ie if we want only one LED on in the middle, it must be alone in that state, and at the right place
   - [ ] Interfacing multiples strips at the same time : either using more pins of the Raspberry Pi or multiplexing the output in order to control the strips of each level independantly
2. Gathering data from the cluster
   - [ ] Polling approach : make the Raspberry Pi connect to the cluster and ask it about its activity level on a periodic basis
   OR
   - [ ] Interrupt approach : make each cluster send data to the Raspberry pi, when there is a big enough activity change, including turnin on/off the cluster, and hysteresis management
3. Putting it all together
   - [ ] Using the gathered data to control the strips : periodically update the pattern (polling) or change the pattern on demand (interrupt)
4. Controlling the cluster
   - [ ] Effectively ordering shut downs without disturbing the normal function of the cluster
   - [ ] Using the gathered data from previous steps to establish the conditions for ordering a shutdown or a startup
   - [ ] Establishing a user-friendly interface to customize varied parameters 

## Further developments
The aforementionned developments will be carried out at user level, and will not touch the kernel in any way.  
However, if time allows it, it may be useful to develop a custom kernel module in order to maybe reduce the overhead and lower, even marginally, the cpu usage of the raspberry pi.
