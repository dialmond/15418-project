#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"

class camera {
    public:
        float aspect_ratio = 1.0; 
        int image_width  = 100; 
        int samples_per_pixel = 10;  
        int max_depth = 10;   

        float vfov = 90;  
        point3 lookfrom = point3(0,0,0);  
        point3 lookat   = point3(0,0,-1);  //
        vec3   vup      = vec3(0,1,0);    

        float defocus_angle = 0;  
        float focus_dist = 10;   

        int    image_height;   
        float pixel_samples_scale;  
        point3 center;       
        point3 pixel00_loc;    
        vec3   pixel_delta_u;  
        vec3   pixel_delta_v;  
        vec3   u, v, w;              
        vec3   defocus_disk_u; 
        vec3   defocus_disk_v;      

    __host__ __device__ void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = lookfrom;

        // Determine viewport dimensions.
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta/2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (float(image_width)/image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;    
        vec3 viewport_v = viewport_height * -v;  

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }
};

#endif