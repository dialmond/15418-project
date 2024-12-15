#!/bin/bash

make SCATTER_PROB=0.05 render_scatter
./render_scatter color_grid --samples-per-pixel 64 --image-width 640 --num-threads 1
./render_scatter color_grid --samples-per-pixel 64 --image-width 640 --num-threads 8
rm -f render_scatter

make SCATTER_PROB=0.25 render_scatter
./render_scatter color_grid --samples-per-pixel 64 --image-width 640 --num-threads 1
./render_scatter color_grid --samples-per-pixel 64 --image-width 640 --num-threads 8
rm -f render_scatter

make SCATTER_PROB=0.5 render_scatter
./render_scatter color_grid --samples-per-pixel 64 --image-width 640 --num-threads 1
./render_scatter color_grid --samples-per-pixel 64 --image-width 640 --num-threads 8
rm -f render_scatter

make SCATTER_PROB=0.75 render_scatter
./render_scatter color_grid --samples-per-pixel 64 --image-width 640 --num-threads 1
./render_scatter color_grid --samples-per-pixel 64 --image-width 640 --num-threads 8
rm -f render_scatter

make SCATTER_PROB=0.95 render_scatter
./render_scatter color_grid --samples-per-pixel 64 --image-width 640 --num-threads 1
./render_scatter color_grid --samples-per-pixel 64 --image-width 640 --num-threads 8
rm -f render_scatter
