#!/bin/bash

make clean && make
make mpi

scenes=(simple materials grid biglittle color_grid kiss)
sizes=(256 640 1280 1920)
threads=(1 8)

echo "# OMP IMAGE SIZE ########################################################"
for thread in "${threads[@]}"; do
  echo "## $thread core ####################################"
  for scene in "${scenes[@]}"; do
    echo "### $scene #################"
    for size in "${sizes[@]}"; do
      ./render "$scene" --samples-per-pixel 64 --image-width "$size" --num-threads "$thread" --no-write
    done
  done
  echo "==========="
done

echo "# MPI IMAGE SIZE ########################################################"
for thread in "${threads[@]}"; do
  echo "## $thread core ####################################"
  for scene in "${scenes[@]}"; do
    echo "### $scene #################"
    for size in "${sizes[@]}"; do
      mpirun -np "$thread" ./render_mpi "$scene" --samples-per-pixel 64 --image-width "$size" --no-write
    done
  done
  echo "==========="
done
