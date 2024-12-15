#ifndef SCENESH
#define SCENESH

#include "hittable_list.h"
#include "camera.h"
#include <string>
// command line arguments
struct Params {
    int image_width = 0,
        image_height = 0,
        samples_per_pixel = 0,
        max_depth = 0,
        bvh = 0,
        show_progress = 0,
        animate = 0,
        frames = 60,
        frame = 0,
        size = 0,
        no_write = 0,
        write_intermittent = 0;
    std::string extension = "";
    std::string scene_name = "";
    int num_threads = 1;
    int pid = 0;
};

hittable_list kiss(camera& cam, Params& params);
hittable_list simple(camera& cam, Params& params);
hittable_list grid(camera& cam, Params& params);
hittable_list biglittle(camera& cam, Params& params);
hittable_list materials(camera& cam, Params& params);
hittable_list color_grid(camera& cam, Params& params);

// list of scenes user can select
const std::string scene_names[] = {
	"simple",
	"materials",
	"grid",
	"biglittle",
	"color_grid",
  "kiss",
};
const int num_scenes = sizeof(scene_names) / sizeof(scene_names[0]);

#endif
