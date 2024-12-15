#ifndef HITTABLELISTH
#define HITTABLELISTH


#include "hittable.h"

#include <vector>

//#include "material.h"

class hittable_list: public hittable  {
  public:
    std::vector<hittable*> objects;

    hittable_list() {}
    hittable_list(hittable *object) { add(object); }

    void clear() { objects.clear(); }

    void free() {
      //std::cout << "freeing...\n";
      for (hittable* object : objects) {
        //material *mat_ptr = object->mat_ptr;
        //delete mat_ptr;
        delete object;
      }
      objects.clear();
    }

    void add(hittable *object) {
      objects.push_back(object);
      bbox = aabb(bbox, object->bounding_box());
    }

    bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
      hit_record temp_rec;
      bool hit_anything = false;
      auto closest_so_far = t_max;

      for (const auto& object : objects) {
        if (object->hit(r, t_min, closest_so_far, temp_rec)) {
          hit_anything = true;
          closest_so_far = temp_rec.t;
          rec = temp_rec;
        }
      }

      return hit_anything;
    }

    aabb bounding_box() const override { return bbox; }
  private:
    aabb bbox;
};


#endif
