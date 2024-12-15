#!/bin/bash

#make clean && make
#make mpi

scenes=(materials grid biglittle color_grid)
sizes=(640)
threads=(1 8)
frames=30

echo "# OMP Rotation ########################################################"
echo "## 1 core ####################################"
for scene in "${scenes[@]}"; do
  echo "### $scene #################"
  for size in "${sizes[@]}"; do
    ./render "$scene" --samples-per-pixel 64 --image-width "$size" --num-threads 1 --no-write --rotate $frames
  done
done
echo "==========="
echo "## 8 core ####################################"
for scene in "${scenes[@]}"; do
  echo "### $scene #################"
  for size in "${sizes[@]}"; do
    ./render "$scene" --samples-per-pixel 64 --image-width "$size" --num-threads 8 --rotate $frames
  done
done
echo "==========="

echo "# MPI Rotation ########################################################"
for thread in "${threads[@]}"; do
  echo "## $thread core ####################################"
  for scene in "${scenes[@]}"; do
    echo "### $scene #################"
    for size in "${sizes[@]}"; do
      mpirun -np "$thread" ./render_mpi "$scene" --samples-per-pixel 64 --image-width "$size" --no-write --rotate $frames
    done
  done
  echo "==========="
done
