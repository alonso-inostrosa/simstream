#!/bin/bash

#SCRIPT PARA EJECUTAR EN CLUSTER
TOPOLOGY=$1
NODES=$2
LENGTH=$3

PATH_TO_SIM="/home/users/mauricio.marin/ainostrosa/storm-simulator"

echo $PATH_TO_SIM"./Simulador -t $TOPOLOGY -n $NODES -l $LENGTH"
srun -A mauricio.marin -n 1 /home/users/mauricio.marin/ainostrosa/storm-simulator/Simulador -t $TOPOLOGY -n $NODES -l $LENGTH > salida.dat
