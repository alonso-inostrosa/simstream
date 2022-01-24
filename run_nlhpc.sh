#!/bin/bash

TOPOLOGY=$1
NODES=$2
LENGTH=$3

PATH_TO_SIM="/home/mmarin/ainostrosa/storm-simulator"

echo $PATH_TO_SIM"/Simulador -t $TOPOLOGY -n $NODES -l $LENGTH"
srun -n 1 /home/mmarin/ainostrosa/storm-simulator/Simulador -t $TOPOLOGY -n $NODES -l $LENGTH
#> salida.dat
