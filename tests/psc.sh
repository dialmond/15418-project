#!/bin/bash

make clean && make
make mpi

scenes=(materials grid biglittle color_grid)
sizes=(640)
threads=(1 2 4 8 16 32 64)

echo "# OMP PSC ########################################################"
for scene in "${scenes[@]}"; do
  echo "### $scene ###############################"
  for thread in "${threads[@]}"; do
    ./render "$scene" --samples-per-pixel 64 --image-width 640 --num-threads "$thread" --no-write
  done
  echo "==========="
done

echo "# MPI PSC ########################################################"
for scene in "${scenes[@]}"; do
  echo "### $scene ###############################"
  for thread in "${threads[@]}"; do
    mpirun -np "$thread" ./render_mpi "$scene" --samples-per-pixel 64 --image-width 640 --num-threads "$thread" --no-write
  done
  echo "==========="
done
