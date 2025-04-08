#ifndef HITTABLE_H
#define HITTABLE_H

#include "vec3.h"
#include "interval.h"

class material;

struct hit_record {
  // public:
    point3 p;
    vec3 normal;
    material* mat;
    float t;
    bool front_face;

    /*
    __device__ void set_face_normal(const ray& r, const vec3& outward_normal) {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.

        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
    */
};

class hittable {
  public:
    __device__ virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const = 0;
};

#endif