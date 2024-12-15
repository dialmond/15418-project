#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"

#include <iostream>

using color = vec3;

inline float clamp(float x) {
  return std::max(0.0f, std::min(x, 1.0f));
}
inline double linear_to_gamma(double linear_component) {
  if (linear_component > 0)
    return clamp(std::sqrt(linear_component));

  return 0;
}

inline void write_color(std::ostream& out, const color& pixel_color) {
  auto r = pixel_color.x();
  auto g = pixel_color.y();
  auto b = pixel_color.z();

  // Apply a linear to gamma transform for gamma 2
  r = linear_to_gamma(r);
  g = linear_to_gamma(g);
  b = linear_to_gamma(b);

  // Translate the [0,1] component values to the byte range [0,255].
  int rbyte = int(255.999 * r);
  int gbyte = int(255.999 * g);
  int bbyte = int(255.999 * b);

  // Write out the pixel color components.
  out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

inline void ptr_color(const color& pixel_color, int *ir, int *ig, int *ib) {
  auto r = pixel_color.x();
  auto g = pixel_color.y();
  auto b = pixel_color.z();

  // Apply a linear to gamma transform for gamma 2
  r = linear_to_gamma(r);
  g = linear_to_gamma(g);
  b = linear_to_gamma(b);

  // Translate the [0,1] component values to the byte range [0,255].
  *ir = int(255.999 * r);
  *ig = int(255.999 * g);
  *ib = int(255.999 * b);
}


#endif