#!/bin/bash

#make clean && make
#make mpi

scenes=(simple materials grid biglittle color_grid kiss)
samples=(8 64 256 512)
threads=(1 8 2 4)

echo "# OMP SAMPLES ########################################################"
for thread in "${threads[@]}"; do
  echo "## $thread core ####################################"
  for scene in "${scenes[@]}"; do
    echo "### $scene #################"
    for sample in "${samples[@]}"; do
      ./render "$scene" --samples-per-pixel "$sample" --image-width 640 --num-threads "$thread" --no-write
    done
  done
  echo "==========="
done

echo ""
echo ""
echo "# MPI SAMPLES ########################################################"
for thread in "${threads[@]}"; do
  echo "## $thread core ####################################"
  for scene in "${scenes[@]}"; do
    echo "### $scene #################"
    for sample in "${samples[@]}"; do
      mpirun -np "$thread" ./render_mpi "$scene" --samples-per-pixel "$sample" --image-width 640 --no-write
    done
  done
  echo "==========="
done
