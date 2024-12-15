#ifndef AABB_H
#define AABB_H

#include "ray.h"
#include <limits>

class aabb {
  public:
    float x_min, x_max, y_min, y_max, z_min, z_max;

    aabb() {} // The default AABB is empty, since intervals are empty by default.

    aabb(const float& x_min, const float& x_max,
        const float& y_min, const float& y_max,
        const float& z_min, const float& z_max)
      :
        x_min(x_min), x_max(x_max),
        y_min(y_min), y_max(y_max),
        z_min(z_min), z_max(z_max) {}

    aabb(const point3& a, const point3& b) {
        // Treat the two points a and b as extrema for the bounding box, so we don't require a
        // particular minimum/maximum coordinate order.

        x_min = std::min(a[0], b[0]);
        x_max = std::max(a[0], b[0]);
        y_min = std::min(a[1], b[1]);
        y_max = std::max(a[1], b[1]);
        z_min = std::min(a[2], b[2]);
        z_max = std::max(a[2], b[2]);
    }

		aabb(const aabb& box0, const aabb& box1) {
        x_min = std::min(box0.x_min, box1.x_min);
        x_max = std::max(box0.x_max, box1.x_max);
        y_min = std::min(box0.y_min, box1.y_min);
        y_max = std::max(box0.y_max, box1.y_max);
        z_min = std::min(box0.z_min, box1.z_min);
        z_max = std::max(box0.z_max, box1.z_max);
    }

    /*const interval& axis_interval(int n) const {
        if (n == 1) return y;
        if (n == 2) return z;
        return x;
    }*/
    const float& axis_min(int n) const {
      if (n == 1) return y_min;
      if (n == 2) return z_min;
      return x_min;
    }
    const float& axis_max(int n) const {
      if (n == 1) return y_max;
      if (n == 2) return z_max;
      return x_max;
    }

    bool hit(const ray& r, float t_min, float t_max) const {
        const point3& ray_orig = r.origin();
        const vec3&   ray_dir  = r.direction();

        for (int axis = 0; axis < 3; axis++) {
            //const interval& ax = axis_interval(axis);
            const float& a_min = axis_min(axis);
            const float& a_max = axis_max(axis);
            const double adinv = 1.0 / ray_dir[axis];

            auto t0 = (a_min - ray_orig[axis]) * adinv;
            auto t1 = (a_max - ray_orig[axis]) * adinv;

            if (t0 < t1) {
                if (t0 > t_min) t_min = t0;
                if (t1 < t_max) t_max = t1;
            } else {
                if (t1 > t_min) t_min = t1;
                if (t0 < t_max) t_max = t0;
            }

            if (t_max <= t_min)
                return false;
        }
        return true;
    }

		int longest_axis() const {
        // Returns the index of the longest axis of the bounding box.

        /*if (x.size() > y.size())
            return x.size() > z.size() ? 0 : 2;
        else
            return y.size() > z.size() ? 1 : 2;*/
      float x_size = x_max - x_min;
      float y_size = y_max - y_min;
      float z_size = z_max - z_min;
      if (x_size > y_size)
        return x_size > z_size ? 0 : 2;
      else
        return y_size > z_size ? 1 : 2;
    }

    static const aabb empty, universe;
};

inline const float inf = std::numeric_limits<float>::max();
inline const aabb aabb::empty    = aabb(+inf,-inf,+inf,-inf,+inf,-inf);
inline const aabb aabb::universe = aabb(-inf,+inf,-inf,+inf,-inf,+inf);

#endif
