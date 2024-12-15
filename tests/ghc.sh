#!/bin/bash

make clean && make
make mpi

scenes=(materials grid biglittle color_grid simple)
sizes=(640)
threads=(1 8)

echo "# OMP GHC ########################################################"
for scene in "${scenes[@]}"; do
  echo "### $scene ###############################"
  for thread in "${threads[@]}"; do
    perf stat -e cache-misses ./render "$scene" --samples-per-pixel 64 --image-width 640 --num-threads "$thread" --no-write
  done
  echo "==========="
done

echo "# MPI GHC ########################################################"
for scene in "${scenes[@]}"; do
  echo "### $scene ###############################"
  for thread in "${threads[@]}"; do
    perf stat -e cache-misses mpirun -np "$thread" ./render_mpi "$scene" --samples-per-pixel 64 --image-width 640 --num-threads "$thread" --no-write
  done
  echo "==========="
done
