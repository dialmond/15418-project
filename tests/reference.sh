#!/bin/bash

make clean && make

./render --samples-per-pixel 512 --image-width 256 --num-threads 8 --extension "ref"

echo "==========="
./render --samples-per-pixel 512 --image-width 640 --num-threads 8 --extension "ref"

echo "==========="
./render --samples-per-pixel 512 --image-width 1280 --num-threads 8 --extension "ref"

echo "==========="
./render --samples-per-pixel 512 --image-width 1920 --num-threads 8 --extension "ref"
