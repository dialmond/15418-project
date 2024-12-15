#ifndef QUADH
#define QUADH

#include "hittable.h"
#include "hittable_list.h"


class quad : public hittable {
  public:
    quad(const point3& Q, const vec3& u, const vec3& v, material *mat_ptr)
      : Q(Q), u(u), v(v), mat_ptr(mat_ptr)
    {
      auto n = cross(u, v);
      normal = unit_vector(n);
      D = dot(normal, Q);
      w = n / dot(n,n);

      set_bounding_box();
    }

    virtual void set_bounding_box() {
      // Compute the bounding box of all four vertices.
      auto bbox_diagonal1 = aabb(Q, Q + u + v);
      auto bbox_diagonal2 = aabb(Q + u, Q + v);
      bbox = aabb(bbox_diagonal1, bbox_diagonal2);
    }

    aabb bounding_box() const override { return bbox; }



    bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
        auto denom = dot(normal, r.direction());

        // No hit if the ray is parallel to the plane.
        if (std::fabs(denom) < 1e-8)
            return false;

        // Return false if the hit point parameter t is outside the ray interval.
        auto t = (D - dot(normal, r.origin())) / denom;
        if (!(t < t_max && t > t_min))
            return false;

        // Determine if the hit point lies within the planar shape using its plane coordinates.
        auto intersection = r.point_at_parameter(t);
        vec3 planar_hitpt_vector = intersection - Q;
        auto alpha = dot(w, cross(planar_hitpt_vector, v));
        auto beta = dot(w, cross(u, planar_hitpt_vector));

        if (!is_interior(alpha, beta, rec))
            return false;

        // Ray hits the 2D shape; set the rest of the hit record and return true.
        rec.t = t;
        rec.p = intersection;
        rec.mat_ptr = mat_ptr;
        rec.normal = normal;

        return true;
    }

    virtual bool is_interior(float a, float b, hit_record& rec) const {
        // primitive, otherwise set the hit record UV coordinates and return true.
        if (!(0 <= a && a <= 1) || !(0 <= b && b <= 1))
          return false;

        return true;
    }

  private:
    point3 Q;
    vec3 u, v;
    vec3 w;
    material *mat_ptr;
    vec3 normal;
    double D;
    aabb bbox;
};


inline hittable_list* box(const point3& a, const point3& b, material* mat) {
    // Returns the 3D box (six sides) that contains the two opposite vertices a & b.
    auto sides = new hittable_list();

    // Construct the two opposite vertices with the minimum and maximum coordinates.
    auto min = point3(std::fmin(a.x(),b.x()), std::fmin(a.y(),b.y()), std::fmin(a.z(),b.z()));
    auto max = point3(std::fmax(a.x(),b.x()), std::fmax(a.y(),b.y()), std::fmax(a.z(),b.z()));

    auto dx = vec3(max.x() - min.x(), 0, 0);
    auto dy = vec3(0, max.y() - min.y(), 0);
    auto dz = vec3(0, 0, max.z() - min.z());

    sides->add(new quad(point3(min.x(), min.y(), max.z()),  dx,  dy, mat)); // front
    sides->add(new quad(point3(max.x(), min.y(), max.z()), -dz,  dy, mat)); // right
    sides->add(new quad(point3(max.x(), min.y(), min.z()), -dx,  dy, mat)); // back
    sides->add(new quad(point3(min.x(), min.y(), min.z()),  dz,  dy, mat)); // left
    sides->add(new quad(point3(min.x(), max.y(), max.z()),  dx, -dz, mat)); // top
    sides->add(new quad(point3(min.x(), min.y(), min.z()),  dx,  dz, mat)); // bottom

    return sides;
}


#endif
