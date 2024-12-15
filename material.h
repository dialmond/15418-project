#ifndef MATERIALH
#define MATERIALH

#include "ray.h"
#include "hittable.h"

struct hit_record;


float schlick(float cosine, float ref_idx);


bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted);


vec3 reflect(const vec3& v, const vec3& n);


vec3 random_in_unit_sphere();


class material  {
  public:
    //virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
    virtual ~material() = default;

		virtual vec3 emitted(const point3& p) const {
      return vec3(0,0,0);
    }

    virtual bool scatter(
      const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered
    ) const = 0;
};

#ifdef SCATTER_PROB
const float scatter_p = SCATTER_PROB;
const float scatter_p_scale = 1.0f / SCATTER_PROB;
#endif

class lambertian : public material {
  public:
    lambertian(const vec3& a) : albedo(a) {}
    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const  {
      vec3 scatter_direction = rec.normal + random_in_unit_sphere();
      if (scatter_direction.near_zero())
        scatter_direction = rec.normal;
      scattered = ray(rec.p, scatter_direction);

#ifdef SCATTER_PROB
      /*if (scatter_p < 0) { // scatter with prob 1-reflectance
        float reflectance = (albedo.x() + albedo.y() + albedo.z()) / 3.0;
        if (drand48() < (1.0 - reflectance)) {
          attenuation = vec3(1.0, 1.0, 1.0);
          return true;
        }
        else {
          return false;
        }
      }*/
      // scatter with whatever scatter_p is
      attenuation = albedo * scatter_p_scale;
      return drand48() < scatter_p;
#else
      // default behavior, just always scatter
      attenuation = albedo;
      return true;
#endif
    }
    vec3 albedo;
};


class metal : public material {
    public:
        metal(const vec3& a, float f) : albedo(a) { if (f < 1) fuzz = f; else fuzz = 1; }
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const  {
            vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            scattered = ray(rec.p, reflected + fuzz*random_in_unit_sphere());
            attenuation = albedo;
            return (dot(scattered.direction(), rec.normal) > 0);
        }
        vec3 albedo;
        float fuzz;
};


class dielectric : public material {
    public:
        dielectric(float ri) : ref_idx(ri) {}
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const  {
             vec3 outward_normal;
             vec3 reflected = reflect(r_in.direction(), rec.normal);
             float ni_over_nt;
             attenuation = vec3(1.0, 1.0, 1.0);
             vec3 refracted;
             float reflect_prob;
             float cosine;
             if (dot(r_in.direction(), rec.normal) > 0) {
                  outward_normal = -rec.normal;
                  ni_over_nt = ref_idx;
               // cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
                  cosine = dot(r_in.direction(), rec.normal) / r_in.direction().length();
                  cosine = sqrt(1 - ref_idx*ref_idx*(1-cosine*cosine));
             }
             else {
                  outward_normal = rec.normal;
                  ni_over_nt = 1.0 / ref_idx;
                  cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
             }
             if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
                reflect_prob = schlick(cosine, ref_idx);
             else
                reflect_prob = 1.0;
             if (drand48() < reflect_prob)
                scattered = ray(rec.p, reflected);
             else
                scattered = ray(rec.p, refracted);
             return true;
        }

        float ref_idx;
};

class diffuse_light : public material {
  public:
    diffuse_light(const vec3& emit) : emit(emit) {}
    virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const  {
      return false;
    }

    vec3 emitted(const point3& p) const override {
        return emit;
    }

		vec3 emit;
};


#endif
