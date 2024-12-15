#!/bin/bash

#make clean && make
#make mpi

scenes=(simple materials grid biglittle color_grid kiss)
samples=(8 64 256 512)
threads=(1 8 2 4)

echo "# OMP SAMPLES ########################################################"
./render materials --samples-per-pixel 8 --image-width 640 --num-threads 1 --no-write
./render grid --samples-per-pixel 8 --image-width 640 --num-threads 1 --no-write
./render biglittle --samples-per-pixel 8 --image-width 640 --num-threads 1 --no-write

./render color_grid --samples-per-pixel 8 --image-width 640 --num-threads 1 --no-write
./render color_grid --samples-per-pixel 64 --image-width 640 --num-threads 1 --no-write

./render kiss --samples-per-pixel 8 --image-width 640 --num-threads 1 --no-write
./render kiss --samples-per-pixel 64 --image-width 640 --num-threads 1 --no-write
./render kiss --samples-per-pixel 256 --image-width 640 --num-threads 1 --no-write
./render kiss --samples-per-pixel 512 --image-width 640 --num-threads 1 --no-write

./render simple --samples-per-pixel 8 --image-width 640 --num-threads 8 --no-write
./render simple --samples-per-pixel 64 --image-width 640 --num-threads 8 --no-write
./render simple --samples-per-pixel 256 --image-width 640 --num-threads 8 --no-write
./render simple --samples-per-pixel 512 --image-width 640 --num-threads 8 --no-write

./render materials --samples-per-pixel 8 --image-width 640 --num-threads 8 --no-write
./render materials --samples-per-pixel 64 --image-width 640 --num-threads 8 --no-write
./render materials --samples-per-pixel 256 --image-width 640 --num-threads 8 --no-write
./render materials --samples-per-pixel 512 --image-width 640 --num-threads 8 --no-write

./render grid --samples-per-pixel 8 --image-width 640 --num-threads 8 --no-write
./render grid --samples-per-pixel 64 --image-width 640 --num-threads 8 --no-write
./render grid --samples-per-pixel 256 --image-width 640 --num-threads 8 --no-write
./render grid --samples-per-pixel 512 --image-width 640 --num-threads 8 --no-write

./render biglittle --samples-per-pixel 8 --image-width 640 --num-threads 8 --no-write
./render biglittle --samples-per-pixel 64 --image-width 640 --num-threads 8 --no-write
./render biglittle --samples-per-pixel 256 --image-width 640 --num-threads 8 --no-write
./render biglittle --samples-per-pixel 512 --image-width 640 --num-threads 8 --no-write

./render color_grid --samples-per-pixel 8 --image-width 640 --num-threads 8 --no-write
./render color_grid --samples-per-pixel 64 --image-width 640 --num-threads 8 --no-write


./render kiss --samples-per-pixel 8 --image-width 640 --num-threads 8 --no-write
./render kiss --samples-per-pixel 64 --image-width 640 --num-threads 8 --no-write
./render kiss --samples-per-pixel 256 --image-width 640 --num-threads 8 --no-write
./render kiss --samples-per-pixel 512 --image-width 640 --num-threads 8 --no-write

./render color_grid --samples-per-pixel 256 --image-width 640 --num-threads 8 --no-write
./render color_grid --samples-per-pixel 512 --image-width 640 --num-threads 8 --no-write
./render color_grid --samples-per-pixel 256 --image-width 640 --num-threads 1 --no-write
./render color_grid --samples-per-pixel 512 --image-width 640 --num-threads 1 --no-write


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
