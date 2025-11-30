#!/bin/bash

# Build External Priority
g++ -std=c++17 interrupts_EP_101214895_101324143.cpp -o EP

# Build Round Robin
g++ -std=c++17 interrupts_RR_101214895_101324143.cpp -o RR

# Build External Priority + Round Robin (Preemptive)
g++ -std=c++17 interrupts_EP_RR_101214895_101324143.cpp -o EP_RR
