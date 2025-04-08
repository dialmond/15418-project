#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"

#include "sphere.h"

class sphere;

class hittable_list : public hittable {
    public:
        hittable **objects;
        int size;

        __device__ hittable_list() : objects(nullptr), size(0) {}
        __device__ hittable_list(hittable** objs, int n) : objects(objs), size(n) {}
        __device__ virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;

};

__device__ __forceinline__ bool sphere_hit(const sphere *s, const ray& r, float t_min, float t_max, hit_record& rec) {
    vec3 oc = r.origin() - s->center;
    float a = dot(r.direction(), r.direction());
    float b = dot(oc, r.direction());
    float c = dot(oc, oc) - s->radius*s->radius;
    float discriminant = b*b - a*c;
    if (discriminant > 0) {
        float temp = (-b - sqrt(discriminant))/a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.at(rec.t);
            rec.normal = (rec.p - s->center) / s->radius;
            rec.mat = s->mat;
            return true;
        }
        temp = (-b + sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.at(rec.t);
            rec.normal = (rec.p - s->center) / s->radius;
            rec.mat = s->mat;
            return true;
        }
    }
    return false;
}

__device__ bool hittable_list::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    float closest_so_far = t_max;
    
    for (int i = 0; i < size; i++) {
        // if (objects[i]->hit(r, t_min, closest_so_far, temp_rec)) {
        if (sphere_hit((sphere*)objects[i],r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }
    return hit_anything;
}




#endif
