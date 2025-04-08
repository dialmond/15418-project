#include <iostream>
#include <time.h>
#include <float.h>
#include <curand_kernel.h>
#include "vec3.h"
#include "ray.h"
#include "sphere.h"
#include "hittable_list.h"
#include "camera.h"
#include "material.h"
#include "color.h"


#define DEBUG
#ifdef DEBUG
#define cudaCheckError(ans) cudaAssert((ans), __FILE__, __LINE__);
inline void cudaAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
    if (code != cudaSuccess)
    {
        fprintf(stderr, "CUDA Error: %s at %s:%d\n",
                cudaGetErrorString(code), file, line);
        if (abort) exit(code);
    }
}
#else
#define cudaCheckError(ans) ans
#endif

__device__ float random_float_cuda(curandState* local_rand_state) {
    // Returns a random real in [0,1).
    return curand_uniform(local_rand_state);
}

__device__ vec3 sample_square(curandState* local_rand_state) {
    // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
    return vec3(random_float_cuda(local_rand_state) - 0.5, random_float_cuda(local_rand_state) - 0.5, 0);
}

__device__ point3 defocus_disk_sample(camera* cam, curandState* local_rand_state) {
    // Returns a random point in the camera defocus disk.
    auto p = random_in_unit_disk(local_rand_state);
    return cam->center + (p[0] * cam->defocus_disk_u) + (p[1] * cam->defocus_disk_v);
}

__device__ ray get_ray(int i, int j, camera* cam, curandState* local_rand_state) {
    // Construct a camera ray originating from the defocus disk and directed at a randomly
    // sampled point around the pixel location i, j.

    auto offset = sample_square(local_rand_state);
    auto pixel_sample = cam->pixel00_loc
                        + ((i + offset.x()) * cam->pixel_delta_u)
                        + ((j + offset.y()) * cam->pixel_delta_v);
    
    // if (i == 0 && j == 0) {
    //     printf("Pixel: %d, %d\n", i, j);
    //     printf("Pixel sample: %d\n", pixel_sample);
    //     printf("Pixel delta u: %f\n", cam->pixel_delta_u);
    //     printf("Pixel delta v: %f\n", cam->pixel_delta_v);
    //     printf("Offset: %d %d\n", offset.x(), offset.y());
    //     printf("Pixel 00 loc: %f\n", cam->pixel00_loc);
    // }

    auto ray_origin = (cam->defocus_angle <= 0) ? cam->center : defocus_disk_sample(cam, local_rand_state);
    auto ray_direction = pixel_sample - ray_origin;

    return ray(ray_origin, ray_direction);
}

__device__ color ray_color(ray& r, int depth, hittable** world, curandState* local_rand_state) {
    ray cur_ray = r;
    vec3 cur_attenuation = vec3(1.0, 1.0, 1.0);

    for (int i = 0; i < depth; i++) {
        hit_record rec;
        if ((*world)->hit(cur_ray, 0.001f, FLT_MAX, rec)) {
            ray scattered;
            vec3 attenuation;
            if (rec.mat->scatter(cur_ray, rec, attenuation, scattered, local_rand_state)) {
                cur_attenuation = cur_attenuation * attenuation;
                cur_ray = scattered;
            } else {
                return color(0, 0, 0);
            }
        }
        else {
            vec3 unit_direction = unit_vector(r.direction());
            auto a = 0.5*(unit_direction.y() + 1.0);
            return cur_attenuation * ((1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0));
        }
    }
    return color(0, 0, 0); // exceeded recursion

}

__global__ void print_thread_indices() {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;
    int k = blockIdx.z * blockDim.z + threadIdx.z;

    printf("Thread index: (%d, %d, %d)\n", i, j, k);
}

__global__ void render_kernel(color* framebuffer,  hittable** world, camera** c) {
    printf("Inside render kernel\n");
    camera* cam = *c;
    // hittable_list* world = *w;
    int image_width = cam->image_width;
    int image_height = cam->image_height;
    int samples_per_pixel = cam->samples_per_pixel;
    int max_depth = cam->max_depth;

    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;

    int pixel_index = j * image_width + i;


    if (i >= image_width || j >= image_height) return;

    // printf("Thread index: (%d, %d)\n", i, j);
    // printf("Pixel index: %d\n", pixel_index);

    // Initialize CURAND state
    curandState local_rand_state;
    curand_init(1984, pixel_index, 0, &local_rand_state); //1984 is the seed

    
    color pixel_color(0, 0, 0);
    for (int sample = 0; sample < samples_per_pixel; sample++) {
        ray r = get_ray(i, j, cam, &local_rand_state);
        // printf("Ray: %f %f %f\n", r.direction().x(), r.direction().y(), r.direction().z());
        pixel_color += ray_color(r, max_depth, world, &local_rand_state);
    // //     pixel_color += ray_color(r, max_depth, world);
        // printf("After ray color function\n");
        // printf("Pixel color: %f %f %f\n", pixel_color.x(), pixel_color.y(), pixel_color.z());
    // }
    }

    framebuffer[pixel_index] = pixel_color;
}

__global__ void createShapeObjects(hittable **d_list, hittable **d_world) {
    if (threadIdx.x == 0 && blockIdx.x == 0) {
        d_list[0] = new sphere(vec3(0.0,-100.5,-1.0), 100.0,
                        new lambertian(vec3(0.8, 0.8, 0.0)));
        d_list[1] = new sphere(vec3(0,0,-1.2), 0.5,
                                new lambertian(vec3(0.1, 0.2, 0.5)));
        d_list[2] = new sphere(vec3(-1.0,0.0,-1.0), 0.5,
                                 new dielectric(1.50));
        d_list[3] = new sphere(vec3(-1.0,0,-1.0), 0.4,
                                 new dielectric(1.00/1.50));
        d_list[4] = new sphere(vec3(1,0,-1), 0.5,
                        new metal(vec3(0.8, 0.6, 0.2), 1.0));
        *d_world = new hittable_list(d_list,5);
    }
}

__global__ void createCamera(camera** cam, const camera& c) {
    *cam = new camera();

    (*cam)->aspect_ratio = 16.0 / 9.0;
    (*cam)->image_width  = 400;
    (*cam)->samples_per_pixel = 10; //was 100
    (*cam)->max_depth = 50;

    (*cam)->vfov     = 20;
    (*cam)->lookfrom = point3(-2,2,1);
    (*cam)->lookat   = point3(0,0,-1);
    (*cam)->vup      = vec3(0,1,0);

    (*cam)->defocus_angle = 10.0;
    (*cam)->focus_dist    = 3.4;
    (*cam)->initialize();
}

// __global__ void checkCamera(camera** cam) {
//     printf("Camera aspect ratio: %f\n", (*cam)->aspect_ratio);
//     printf("Camera image width: %d\n", (*cam)->image_width);
//     printf("Camera samples per pixel: %d\n", (*cam)->samples_per_pixel);
//     printf("Camera max depth: %d\n", (*cam)->max_depth);
// }


void intializeKernel(const camera& cam, int image_width, int image_height) {
    cudaDeviceProp props;
    cudaGetDeviceProperties(&props, 0);
    printf("Max Threads Per Block: %d\n", props.maxThreadsPerBlock);


    hittable** d_world;
    hittable** d_list;
    color* d_framebuffer;
    camera** d_camera;
    size_t framebuffer_size = image_width * image_height * sizeof(color);

    // cudaCheckError();
    // cudaDeviceReset();

    std::clog << "Initializing CUDA kernel...\n";
    // Allocate memory for the framebuffer on the device
    cudaCheckError(cudaMalloc(&d_framebuffer, framebuffer_size));

    // Allocate memory for the camera on the device
    cudaCheckError(cudaMalloc((void **)&d_camera, sizeof(camera*)));
    createCamera<<<1,1>>>(d_camera, cam);
    cudaCheckError(cudaDeviceSynchronize());

    // // Allocate memory for the world on the device
    // cudaCheckError(cudaMalloc((void**)&d_list, 5 * sizeof(hittable*)));
    // cudaCheckError(cudaMalloc((void**)&d_world, sizeof(hittable*)));

    // createShapeObjects<<<1,1>>>(d_list, d_world);
    // cudaCheckError(cudaDeviceSynchronize());
    
    // // Define block and grid sizes
    // dim3 block_size(16, 16);
    // dim3 grid_size((image_width + block_size.x - 1) / block_size.x, 
    //         (image_height + block_size.y - 1) / block_size.y);

    // // Launch the CUDA kernel
    // std::clog << "Launching CUDA kernel...\n";
    // render_kernel<<<grid_size, block_size>>>(d_framebuffer, d_world, d_camera);
    // cudaCheckError(cudaDeviceSynchronize());
    // std::clog << "CUDA kernel finished.\n";

    // // Allocate memory for the framebuffer on the host using malloc
    // color* h_framebuffer = (color*)malloc(image_width * image_height * sizeof(color));
    // cudaMemcpy(h_framebuffer, d_framebuffer, framebuffer_size, cudaMemcpyDeviceToHost);

    // // Free GPU memory
    // cudaFree(d_framebuffer);
    // cudaFree(d_world);
    // cudaFree(d_camera);

    // Output the image
    // std::clog << "Rendering image...\n";
    // std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    // for (int j = 0; j < image_height; j++) {
    //     for (int i = 0; i < image_width; i++) {
    //         write_color(std::cout, cam.pixel_samples_scale * h_framebuffer[j * image_width + i]);
    //     }
    // }
    // std::clog << "\nDone.\n";

    // Free the allocated memory
    // free(h_framebuffer);
}

void rayCUDA(){
    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width  = 400;
    cam.samples_per_pixel = 10; //was 100
    cam.max_depth = 50;

    cam.vfov     = 20;
    cam.lookfrom = point3(-2,2,1);
    cam.lookat   = point3(0,0,-1);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 10.0;
    cam.focus_dist    = 3.4;

    cam.initialize();

    intializeKernel(cam, cam.image_width, cam.image_height);
}