#ifndef CAMERAH
#define CAMERAH

#include "hittable_list.h"
#include "material.h"

#include "ray.h"
#include "color.h"

#include <vector>
#include <limits>

//#pragma omp declare reduction(+ : vec3 : omp_out+=omp_in) initializer(omp_priv(0,0,0))

const float maxfloat = std::numeric_limits<float>::max();
class camera {
  public:
    int image_height;
    float aspect_ratio = 1.0;
    int image_width = 100;
    int samples_per_pixel = 64;
    int max_depth = 50;
    color background = color(0.70, 0.80, 1.00);

    float vfov = 90;
    point3 lookfrom = point3(0,0,0);
    point3 lookat = point3(0,0,-1);
    vec3 vup = vec3(0,1,0);

    float focus_dist = 10.0;

    float pixel_samples_scale;

    void set_height() {
      image_height = int(image_width / aspect_ratio);
      image_height = (image_height < 1) ? 1 : image_height;
    }

    void initialize() {
      image_height = int(image_width / aspect_ratio);
      image_height = (image_height < 1) ? 1 : image_height;

      pixel_samples_scale = 1.0 / samples_per_pixel;

      float theta = vfov*M_PI/180;
      float half_height = tan(theta/2);
      float half_width = aspect_ratio * half_height;
      origin = lookfrom;
      w = unit_vector(lookfrom - lookat);
      u = unit_vector(cross(vup, w));
      v = cross(w, u);
      lower_left_corner = origin  - half_width*focus_dist*u -half_height*focus_dist*v - focus_dist*w;
      horizontal = 2*half_width*focus_dist*u;
      vertical = 2*half_height*focus_dist*v;
    }


    vec3 ray_color(const ray& r, int depth, hittable_list& world) {
      // If we've exceeded the ray bounce limit, no more light is gathered.
      if (depth <= 0)
        return color(0,0,0);

      hit_record rec;

      if (!world.hit(r, 0.001, maxfloat, rec))
        return background;

      ray scattered;
      color attenuation;
      // possible TODO: for paralellism sake, is there a way i can
      // combine both emmited and scatter into one fn?
      // that would marginally decrease accesses
      color color_from_emission = rec.mat_ptr->emitted(rec.p);

      if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return color_from_emission;

      color color_from_scatter = attenuation * ray_color(scattered, depth-1, world);

      return color_from_emission + color_from_scatter;
    }

    ray get_ray(float s, float t) {
      return ray(origin, lower_left_corner + s*horizontal + t*vertical - origin);
    }

    std::vector<color> render(hittable_list& world) {
      initialize();

      int ny = image_height;
      int nx = image_width;
      int ns = samples_per_pixel;

      std::vector<color> image(nx * ny);
#ifdef P_MPI
#else
      #pragma omp parallel for schedule(dynamic)
#endif
      for (int j = 0; j < ny; j++) {
        for (int i = 0; i < nx; i++) {
          color pixel_color(0, 0, 0);
          //#pragma omp parallel for reduction(+:pixel_color)
          for (int s=0; s < ns; s++) {
            float u = float(i + drand48()) / float(nx);
            float v = float(j + drand48()) / float(ny);
            ray r = get_ray(u, v);
            pixel_color += ray_color(r, max_depth, world);
          }

          image[j*nx+i] = pixel_color * pixel_samples_scale;
        }
      }
      return image;
    }


    vec3 origin;
    vec3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
};


#endif
