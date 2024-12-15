#include "scenes.h"

#include "sphere.h"
#include "quad.h"

#include <random>

inline double random_double() {
  // Returns a random real in [0,1).
  return std::rand() / (RAND_MAX + 1.0);
}

hittable_list kiss(camera& cam, Params& params) {
  params.scene_name = "kiss";
  hittable_list world;

  auto material = new lambertian(color(0.5, 0.5, 0.5));
  world.add(new sphere(point3(0,-1000,0), 1000, material));
  world.add(new sphere(point3(0,2,0), 2, material));

  cam.aspect_ratio      = 16.0 / 9.0;
  cam.image_width       = 255;
  cam.samples_per_pixel = 8;
  cam.max_depth         = 10;

  cam.lookfrom = point3(0,3,6);
  cam.lookat   = point3(0,2,0);

	return world;
}

hittable_list simple(camera& cam, Params& params) {
  params.scene_name = "simple";
  hittable_list world;

  auto material = new lambertian(color(0.5, 0.5, 0.5));
  world.add(new sphere(point3(0,-1000,0), 1000, material));
  world.add(new sphere(point3(0,2,0), 2, material));

  auto difflight = new diffuse_light(color(4,4,4));
  world.add(new quad(point3(3,1,-2), vec3(2,0,0), vec3(0,2,0), difflight));
  world.add(new quad(point3(-2,6,-2), vec3(4,0,0), vec3(0,0,4), difflight));

  cam.aspect_ratio      = 16.0 / 9.0;
  cam.image_width       = 400;
  cam.samples_per_pixel = 256;
  cam.background        = color(0,0,0);

  cam.vfov     = 20;
  cam.lookfrom = point3(26,3,6);
  cam.lookat   = point3(0,2,0);

	return world;
}

hittable_list grid(camera& cam, Params& params) {
  // creates a grid of size `size` (defined later) of spheres centered at (0,0,0)
  // each sphere is of size `radius`, and the space between spheres is
  // equal to the radius
  params.scene_name = "grid";
  hittable_list world;

  auto material = new metal(color(.5, .5, .5), 0.1);

  int s = (params.size ? params.size : 5);
  std::vector<int> size = {s, s, s}; // x y z
  double radius = 0.5;
  double space_between = radius;

  // most positive corner
  std::vector<double> corners = {0, 0, 0};
  for (int i = 0; i < 3; i++){
    corners[i] = (size[i] / 2) * (2 * radius + space_between);
    if (size[i] % 2 == 0)
      corners[i] -= 0.5 * space_between + radius;
  }

  // +x is right, +y is up, -z is in direction of view
  for (int x = 0; x < size[0]; x++) {
    for (int y = 0; y < size[1]; y++) {
      for (int z = 0; z < size[2]; z++) {
        double x_d = corners[0] - (2 * radius + space_between) * x;
        double y_d = corners[1] - (2 * radius + space_between) * y;
        double z_d = corners[2] - (2 * radius + space_between) * z;
        world.add(new sphere(point3(x_d, y_d, z_d),  radius, material));
      }
    }
  }

  cam.aspect_ratio      = 16.0 / 9.0;
  cam.image_width       = 640;
  cam.samples_per_pixel = 64;

  cam.vfov     = 40;
  double look_d = 2 * radius;
  cam.lookfrom = point3(corners[0], corners[1], corners[2]) + point3(0, look_d, -3 * s * look_d);
  cam.lookat   = point3(0, -0.5 * look_d, 0);

	return world;
}

// one thousand spheres, but all concentrated
// on right of image
hittable_list biglittle(camera& cam, Params& params) {
  params.scene_name = "biglittle";
  hittable_list world;

  auto material = new lambertian(color(0.8, 0.8, 0.8));

  int ns = 1000;
  for (int j = 0; j < ns; j++) {
    world.add(new sphere(vec3(16*random_double(),
                              16*random_double(),
                              16*random_double()), 1, material));
  }

  cam.aspect_ratio      = 16.0 / 9.0;
  cam.image_width       = 640;
  cam.samples_per_pixel = 64;

  cam.vfov = 40;
  cam.lookfrom = point3(-20,20,-20);
  cam.lookat   = point3(12,6.5,0);

	return world;
}


hittable_list materials(camera& cam, Params& params) {
  params.scene_name = "materials";
  hittable_list world;

  auto ground_material = new lambertian(color(0.5, 0.5, 0.5));
  //world.add(new quad(point3(-10,0, -10), vec3(20, 0, 0), vec3(0, 0,20), ground_material));
  world.add(new sphere(point3(0,-1000,0), 1000, ground_material));

  for (int a = -5; a < 5; a++) {
    for (int b = -5; b < 5; b++) {
      float choose_mat = random_double();
      vec3 center(a+0.9*random_double(),0.2,b+0.9*random_double());

      if ((center-vec3(4,0.2,0)).length() > 0.9) {
        if (choose_mat < 0.8) {  // diffuse
          world.add(new sphere(
              center, 0.2,
              new lambertian(vec3(random_double()*random_double(),
                                  random_double()*random_double(),
                                  random_double()*random_double()))
          ));
        }
        else if (choose_mat < 0.95) { // metal
          world.add(new sphere(
              center, 0.2,
              new metal(vec3(0.5*(1 + random_double()),
                             0.5*(1 + random_double()),
                             0.5*(1 + random_double())),
                        0.5*random_double())
          ));
        }
        else {  // glass
          world.add(new sphere(center, 0.2, new dielectric(1.5)));
        }
      }
    }
  }

  // negative number moves it further away from the camera
  auto material1 = new dielectric(1.5);
  world.add(new sphere(point3(5, 2, 5), 2.0, material1));

  auto material2 = new metal(color(0.7, 0.6, 0.4), 0.1);
  world.add(new sphere(point3(-5, 2, -5), 2.0, material2));

  cam.aspect_ratio      = 16.0 / 9.0;
  cam.image_width       = 640;
  cam.samples_per_pixel = 16;

  cam.vfov     = 20;
  //cam.lookfrom   = point3(0,5,20);
  cam.lookfrom   = point3(11.2,5,16.6);
  cam.lookat   = point3(0,0,0);

	return world;
}

hittable_list color_grid(camera& cam, Params& params) {
  // creates a grid of size `size` (defined later) of boxes centered at (0,0,0)
  params.scene_name = "color_grid";
  hittable_list world;

  int s = (params.size ? params.size : 5);
  std::vector<int> size = {s, s, s}; // x y z
  double length = 1;
  double space_between = 1;

  // most positive corner
  std::vector<double> corners = {0, 0, 0};
  for (int i = 0; i < 3; i++){
    corners[i] = (size[i] / 2) * (length + space_between);
    if (size[i] % 2 == 0)
      corners[i] -= 0.5 * space_between + length;
  }

  // +x is right, +y is up, -z is in direction of view
  int sx = size[0]; int sy = size[1]; int sz = size[2];
  for (int x = 0; x < sx; x++) {
    for (int y = 0; y < sy; y++) {
      for (int z = 0; z < sz; z++) {
        double x_d = corners[0] - (length + space_between) * x;
        double y_d = corners[1] - (length + space_between) * y;
        double z_d = corners[2] - (length + space_between) * z;
        auto c = color(float(x)/float(sx), float(y)/float(sy), float(z)/float(sz));
        world.add(
            box(point3(x_d, y_d, z_d),
              point3(x_d + length, y_d+length, z_d+length),
              new lambertian(c)));
      }
    }
  }

  cam.aspect_ratio      = 16.0 / 9.0;
  cam.image_width       = 640;
  cam.samples_per_pixel = 64;

  cam.vfov     = 40;
  double look_d = 2 * length;
  cam.lookfrom = point3(corners[0], corners[1], corners[2]) + point3(0, look_d, -2.2 * s * look_d);
  cam.lookat   = point3(0, 0, 0);

	return world;
}


