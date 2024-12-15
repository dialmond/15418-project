#include <iostream>
#include <vector>
#include "float.h"
#include "color.h"

#include <cstdlib>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <filesystem>

#include "scenes.h"
#include "bvh.h"

#define ROTATE 1
#define PAN 2

#ifdef P_MPI
#include <mpi.h>
#else
#include <omp.h>
#endif

#define ROOT 0


// helper fn to pad numbers to three digits
std::string pad(int number) {
  std::ostringstream oss;
  oss << std::setw(3) << std::setfill('0') << number;
  return oss.str();
}

void write(
    const std::vector<color>& image,
    const Params& params
  ) {
  int ny = params.image_height;
  int nx = params.image_width;

  // write to the file fname
  std::string fname = "outs/" +
    params.scene_name + "_" + std::to_string(nx) + "x" +
    std::to_string(ny) +
    (params.extension.empty() ? "" : "_") + params.extension;

  if (params.animate) {
    // create the folder to store the animation
    if (!std::filesystem::exists(fname))
        std::filesystem::create_directories(fname);
    // the actual filename is just going to be the frame number
    fname += "/" + pad(params.frame);
  }
  fname += ".ppm";

  std::ofstream fout(fname, std::fstream::out);
  if(!fout) {
    std::cerr << "Unable to open file: " << fname << '\n';
    exit(EXIT_FAILURE);
  }

  fout << "P3\n" << nx << ' ' << ny << "\n255\n";

  //for (int j = 0; j < ny; j++) {
  for (int j = ny-1; j >= 0; j--) {
    //for (int i = 0; i < nx; i++) {
    for (int i = 0; i < nx; i++) {
      write_color(fout, image[j*nx+i]);
    }
  }

  // close file when finished
  fout.close();
}

#ifdef P_MPI
void init_MPI_struct(MPI_Datatype *newtype) {
  int count = 1;
  int array_of_blocklengths[count] = {3};
  MPI_Aint array_of_displacements[count];
  MPI_Datatype array_of_types[count] = {MPI_FLOAT};

  vec3 dummy = color(0,0,0);
  MPI_Aint base_address;
  MPI_Get_address(&dummy, &base_address);
  MPI_Get_address(&dummy.e[0], &array_of_displacements[0]);

  // Adjust displacement relative to the base address
  array_of_displacements[0] -= base_address;

  MPI_Type_create_struct(count, array_of_blocklengths, array_of_displacements, array_of_types, newtype);
  MPI_Type_commit(newtype);
}

// hierarchically combine images into one
void combine_images(
    std::vector<color>& image,
    Params& params,
    MPI_Datatype MPI_COLOR) {
  const int IMAGE_SIZE = params.image_height * params.image_width;
  const int nproc = params.num_threads;
  const int pid = params.pid;
  std::vector<color> received_image(IMAGE_SIZE);

  for (int step = 1; step < nproc; step *= 2) {
    if (pid % (2 * step) == 0) {
      // receive an image from its partner
      int partner = pid + step;
      MPI_Recv(received_image.data(), IMAGE_SIZE, MPI_COLOR, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      // average the image with the received
      for (int i = 0; i < IMAGE_SIZE; i++) {
        image[i] = (image[i] + received_image[i]) / 2.0;
      }
    } else if (pid % step == 0) {
      // send image to its partner
      int partner = pid - step;
      MPI_Send(image.data(), IMAGE_SIZE, MPI_COLOR, partner, 0, MPI_COMM_WORLD);
      break;
    }
  }
}
#endif

void rotate_animation(camera& cam, hittable_list& world, Params& params) {
#ifdef P_MPI
  MPI_Datatype MPI_COLOR;
  init_MPI_struct(&MPI_COLOR);
#endif
  const int pid = params.pid;

  point3 point = cam.lookfrom;
  const point3 origin = cam.lookat;
  const double theta = 2 * M_PI / params.frames;
  const point3 dp = point - origin;

  for (int frame = 0; frame < params.frames; frame++) {
    // rotate where we're looking from around looking at
    double theta_prime = theta * frame;
    double dx = dp.x() * cos(theta_prime) + dp.z() * sin(theta_prime);
    double dz = -1 * dp.x() * sin(theta_prime) + dp.z() * cos(theta_prime);
    cam.lookfrom = origin + point3(dx, -origin.y() + point.y(), dz);

    std::vector<color> image = cam.render(world);
#ifdef P_MPI
    combine_images(image, params, MPI_COLOR);
#endif

    params.frame = frame;
    if (pid == ROOT && !params.no_write)
      write(image, params);
  }
}

void render_wrapper(camera& cam, hittable_list& world, Params& params) {
  // set the parameters from params if present
  if (params.image_width) cam.image_width = params.image_width;
  if (params.samples_per_pixel) cam.samples_per_pixel = params.samples_per_pixel;
  if (params.max_depth) cam.max_depth = params.max_depth;
  if (params.bvh) world = hittable_list(new bvh_node(world));

  int pid = params.pid;

#ifdef P_MPI
  cam.samples_per_pixel /= params.num_threads;
  srand48(pid);
#else
  omp_set_num_threads(params.num_threads);
#endif

  std::vector<color> image;

  cam.set_height();
  params.image_height = cam.image_height;
  params.image_width = cam.image_width;

  // time rendering the image
  const auto compute_start = std::chrono::steady_clock::now();
  if (params.animate == ROTATE) {
    rotate_animation(cam, world, params);
  }
  else {
    image = cam.render(world);
  }

#ifdef P_MPI
  // write the individual test images
  if (params.write_intermittent && !params.animate) {
    std::string tmp = params.extension;
    params.extension = tmp + (tmp.empty() ? "" : "_") + "mpi_" + pad(pid);
    write(image, params);
    params.extension = tmp;
  }

  // now we need all the workers to broadcast their vectors
  // eventually to pid 1, which will combine everything
  if (!params.animate) {
    MPI_Datatype MPI_COLOR;
    init_MPI_struct(&MPI_COLOR);
    combine_images(image, params, MPI_COLOR);
  }
#endif

  // write the image to a file
  if (pid == ROOT && !params.animate && !params.no_write)
    write(image, params);

  if (pid == ROOT) {
    const double compute_time = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - compute_start).count();
    std::cout << "Computation time (" << params.scene_name << "): " << std::fixed << std::setprecision(10) << compute_time << " s\n";
  }
  // finish computation

  // clean up memory...
  //if (pid == ROOT)
  //  world.free();
  // free the world, give peace a chance

}

int main(int argc, char* argv[]) {
  const auto total_start = std::chrono::steady_clock::now();

  Params params;

  int pid = 0;
  int nproc = 0;
#ifdef P_MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);
#endif

  int scene_index = -1;

  int start_args = 1;

  // get which scene to render from a list of options
  if (argc > 1) {
    std::string scene = argv[1];
    for (int i = 0; i < num_scenes; ++i) {
      if (scene_names[i] == scene) {
        scene_index = i;
        start_args++;
        break;
      }
    }
    if (scene == "all") {
      start_args++;
      scene_index = -1;
    }
  }

  for (int i = start_args; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--image-width" && i + 1 < argc) {
      params.image_width = std::atoi(argv[++i]);
    } else if (arg == "--samples-per-pixel" && i + 1 < argc) {
      params.samples_per_pixel = std::atoi(argv[++i]);
    } else if (arg == "--max-depth" && i + 1 < argc) {
      params.max_depth = std::atoi(argv[++i]);
    } else if (arg == "--use-bvh") {
      params.bvh = 1;
    } else if (arg == "--show-progress") {
      params.show_progress = 1;
    } else if (arg == "--extension" && i + 1 < argc) {
      params.extension = argv[++i];
    // get the animation to use, if prsent
    } else if (arg == "--rotate") {
      params.animate = ROTATE;
      if  (i + 1 < argc) params.frames = std::atoi(argv[++i]);
    } else if (arg == "--pan") {
      params.animate = PAN;
      if  (i + 1 < argc) params.frames = std::atoi(argv[++i]);
    } else if (arg == "--size" && i + 1 < argc) {
      params.size = std::atoi(argv[++i]);
    //unknown argument
    } else if (arg == "--num-threads" && i + 1 < argc) {
      params.num_threads = std::atoi(argv[++i]);
    } else if (arg == "--no-write") {
      params.no_write = 1;
    } else if (arg == "--write-intermittent") {
      params.write_intermittent = 1;
    } else if (!(scene_index >= 0 && i == 1)) {
      std::cerr << "Unknown argument: " << arg << '\n';
    }
  }

#ifdef SCATTER_PROB
  const float scatter_p = SCATTER_PROB;
    std::ostringstream oss;
    oss.precision(2);
    oss << std::fixed << scatter_p;
    params.extension += (params.extension.empty() ? "" : "_");
    params.extension += "scatter_" + oss.str();
#endif

  if (pid == ROOT) {
    if (scene_index >= 0 && scene_index < num_scenes)
      std::cout << "rendering scene: " << scene_names[scene_index] << '\n';
    else
      std::cout << "rendering scene: all" << '\n';

    if (params.num_threads)
      std::cout << "threads: " << params.num_threads << '\n';
    if (nproc)
      std::cout << "threads: " << nproc << '\n';
    if (params.image_width)
      std::cout << "image_width: " << params.image_width << '\n';
    if (params.samples_per_pixel)
      std::cout << "samples_per_pixel: " << params.samples_per_pixel << '\n';
    if (params.max_depth)
      std::cout << "max_depth: " << params.max_depth << '\n';
    if (params.size)
      std::cout << "size: " << params.size << '\n';
    if (params.animate == ROTATE)
      std::cout << "rotation frames: " << params.frames << '\n';

#ifdef SCATTER_PROB
    if (scatter_p)
      std::cout << "scatter probability: " << scatter_p << '\n';
#endif
  }
  params.pid = pid;
#ifdef P_MPI
  params.num_threads = nproc;
#endif

  // if scene_index is -1 render all scenes. otherwise render scenes based on
  // their index
  // pass in command line params to scenes

  camera cam;
  camera original;
  hittable_list world;
  if (scene_index == 0 || scene_index == -1) {
    world = simple(cam, params);
    render_wrapper(cam, world, params);
    cam = original;
  }
  if (scene_index == 1 || scene_index == -1) {
    world = materials(cam, params);
    render_wrapper(cam, world, params);
    cam = original;
  }
  if (scene_index == 2 || scene_index == -1) {
    world = grid(cam, params);
    render_wrapper(cam, world, params);
    cam = original;
  }
  if (scene_index == 3 || scene_index == -1) {
    world = biglittle(cam, params);
    render_wrapper(cam, world, params);
    cam = original;
  }
  if (scene_index == 4 || scene_index == -1) {
    world = color_grid(cam, params);
    render_wrapper(cam, world, params);
    cam = original;
  }
  if (scene_index == 5 || scene_index == -1) {
    world = kiss(cam, params);
    render_wrapper(cam, world, params);
    cam = original;
  }

#ifdef P_MPI
  MPI_Finalize();
#endif

  if (pid == ROOT) {
    const double total_time = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - total_start).count();
    std::cout << "Total time: " << std::fixed << std::setprecision(10) << total_time << " s\n\n";
  }

  return 0;
}
