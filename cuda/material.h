// material.h
#ifndef MATERIAL_H
#define MATERIAL_H

struct hit_record;

#include "ray.h"
#include "hittable.h"
#include "color.h"

class material {
    public:
        __device__ virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation,
                            ray& scattered, curandState* local_rand_state) const = 0;
};

class lambertian : public material {
    public:
        __device__ lambertian(const color& a) : albedo(a) {}

        __device__ virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation,
                    ray& scattered, curandState* local_rand_state) const override {
            vec3 scatter_direction = rec.normal + random_unit_vector(local_rand_state);

            // Catch degenerate scatter direction
            if (scatter_direction.near_zero())
                scatter_direction = rec.normal;

            scattered = ray(rec.p, scatter_direction);
            attenuation = albedo;
            return true;
        }

        color albedo;
};

class metal : public material {
  public:
    __device__ metal(const color& albedo, float fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    __device__ virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation,
                ray& scattered, curandState* local_rand_state) const override {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector(local_rand_state));
        scattered = ray(rec.p, reflected);
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }

    color albedo;
    float fuzz;
};

class dielectric : public material {
public:
    __device__ dielectric(float ri) : refraction_index(ri) {}

    __device__ virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation,
                ray& scattered, curandState* local_rand_state) const override {
        attenuation = color(1.0, 1.0, 1.0);
        float etai_over_etat = rec.front_face ? (1.0 / refraction_index) : refraction_index;

        vec3 unit_direction = unit_vector(r_in.direction());
        float cos_theta = dot(-unit_direction, rec.normal);
        if (cos_theta > 1.0) {
            cos_theta = 1.0;
        }
        float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = etai_over_etat * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, etai_over_etat) > random_float(local_rand_state))
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, etai_over_etat);

        scattered = ray(rec.p, direction);
        return true;
    }

    float refraction_index;

    __device__ static float reflectance(float cosine, float ref_idx) {
        auto r0 = (1.0 - ref_idx) / (1.0 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1.0 - r0) * pow((1.0 - cosine), 5);
    }
};

#endif 