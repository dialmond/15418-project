#!/bin/bash

#make clean && make
#make mpi

scenes=(grid color_grid)
sizes=(1 2 5 10)
threads=(1 8)

echo "# OMP SIZE ########################################################"
for thread in "${threads[@]}"; do
  echo "## $thread core ####################################"
  for scene in "${scenes[@]}"; do
    echo "### $scene #################"
    for size in "${sizes[@]}"; do
      ./render "$scene" --samples-per-pixel 64 --image-width 640 --num-threads "$thread" --no-write --size "$size" --use-bvh
    done
  done
  echo "==========="
done

echo ""
echo ""
echo "# MPI SIZE ########################################################"
for thread in "${threads[@]}"; do
  echo "## $thread core ####################################"
  for scene in "${scenes[@]}"; do
    echo "### $scene #################"
    for size in "${sizes[@]}"; do
      mpirun -np "$thread" ./render_mpi "$scene" --samples-per-pixel 64 --image-width 640  --no-write --size "$size" --use-bvh
    done
  done
  echo "==========="
done
