#include <iostream>
#include <SDL2/SDL.h>

#include <vector>
#include "color.h"
#include "camera.h"
#include "scenes.h"

#include <chrono>
#include <iomanip>

#include <omp.h>

const int WIDTH = 800;

const double theta = 2 * M_PI / 64;
const float distance = 1.0;
void rotate(vec3& lookfrom, vec3& lookat, vec3 up, int dir) {
    vec3 forward = unit_vector(lookat - lookfrom);
    vec3 right = unit_vector(cross(forward, up));

		vec3 rotated_forward =
        forward * cos(theta * dir) +
				right * sin(theta * dir);

    lookat = lookfrom + rotated_forward;
}
void rotate_y(vec3& lookfrom, vec3& lookat, vec3 up, int dir) {
    vec3 forward = unit_vector(lookat - lookfrom);
		vec3 rotated_forward =
        forward * cos(theta * dir) +
				up * sin(theta * dir);

    lookat = lookfrom + rotated_forward;
}

void take_args(int argc, char *argv[], Params& params, int& scene_index) {
	int start_args = 1;

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
    } else if (arg == "--size" && i + 1 < argc) {
      params.size = std::atoi(argv[++i]);
    } else if (arg == "--num-threads" && i + 1 < argc) {
      params.num_threads = std::atoi(argv[++i]);
    } else {
      std::cerr << "Unknown argument: " << arg << '\n';
    }
  }
}

int main(int argc, char *argv[]) {
		Params params;
		int scene_index = -1;
		take_args(argc, argv, params, scene_index);

		camera cam;

		hittable_list world;
		switch (scene_index) {
			case 0:
				world = simple(cam, params);
				break;
			case 1:
				world = materials(cam, params);
				break;
			case 2:
				world = grid(cam, params);
				break;
			case 3:
				world = biglittle(cam, params);
				break;
			case 4:
				world = color_grid(cam, params);
				break;
			default:
					world = kiss(cam, params);
		}

		if (params.image_width) cam.image_width = params.image_width;
		if (params.samples_per_pixel) cam.samples_per_pixel = params.samples_per_pixel;
		if (params.max_depth) cam.max_depth = params.max_depth;

		point3 lookfrom = cam.lookfrom;
		point3 lookat = cam.lookat;

    omp_set_num_threads(params.num_threads);
		cam.initialize();
		vec3 up = cam.vup;

		const int IWIDTH = cam.image_width;
		const int IHEIGHT = cam.image_height;
		const int HEIGHT = (WIDTH / cam.aspect_ratio);
		std::vector<color> image;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cout << "SDL Initialization Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create SDL window
    SDL_Window *window = SDL_CreateWindow("15418 demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);

    if (window == NULL) {
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create SDL renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        std::cout << "Could not create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main loop
    SDL_Event event;
    bool running = true;


		int frames = 0;
		const auto compute_start = std::chrono::steady_clock::now();
    while (running) {
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          running = false;
        }
        if ( SDL_KEYDOWN == event.type ) {
          if ( SDLK_a == event.key.keysym.sym ) {
						vec3 forward = unit_vector(lookat - lookfrom);
						vec3 right = unit_vector(cross(forward, up));
						lookfrom += right * -1 * distance;
						lookat += right * -1 * distance;
          }
					else if ( SDLK_d == event.key.keysym.sym ) {
						vec3 forward = unit_vector(lookat - lookfrom);
						vec3 right = unit_vector(cross(forward, up));
						lookfrom += right * distance;
						lookat += right * distance;
          }
					else if ( SDLK_w == event.key.keysym.sym ) {
						vec3 forward = unit_vector(lookat - lookfrom);
						lookfrom += forward * distance;
						lookat += forward * distance;
          }
					else if ( SDLK_s == event.key.keysym.sym ) {
            vec3 forward = unit_vector(lookat - lookfrom);
						lookfrom += forward * -1 * distance;
						lookat += forward * -1 * distance;
          }
					else if ( SDLK_q == event.key.keysym.sym ) {
						lookfrom += vec3(0, -1 * distance, 0);
						lookat += vec3(0,  -1 * distance, 0);
          }
					else if ( SDLK_e == event.key.keysym.sym ) {
						lookfrom += vec3(0, 1 * distance, 0);
						lookat += vec3(0, 1 * distance, 0);
          }
					else if ( SDLK_LEFT == event.key.keysym.sym ) {
						rotate(lookfrom, lookat, up, -1);
          }
					else if ( SDLK_RIGHT == event.key.keysym.sym ) {
						rotate(lookfrom, lookat, up, 1);
          }
					else if ( SDLK_DOWN == event.key.keysym.sym ) {
						rotate_y(lookfrom, lookat, up, -1);
          }
					else if ( SDLK_UP == event.key.keysym.sym ) {
						rotate_y(lookfrom, lookat, up, 1);
          }
					else if ( SDLK_ESCAPE == event.key.keysym.sym ) {
						running = false;
						break;
          }
        }
      }
			cam.lookfrom = lookfrom;
			cam.lookat   = lookat;
			image = cam.render(world);

      // Clear screen
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black
      SDL_RenderClear(renderer);

      int ir, ig, ib;
			int ii, ij;
			const int scalew = WIDTH / IWIDTH;
			const int scaleh = HEIGHT / IHEIGHT;
      for (int j = 0; j < HEIGHT; j++) {
        for (int i = 0; i < WIDTH; i++) {
					ii = std::min(i / scalew, IWIDTH - 1);
					ij = std::min(j / scaleh, IHEIGHT - 1);
          color c = image[(IHEIGHT-ij-1)*IWIDTH+ii];
          ptr_color(c, &ir, &ig, &ib);
          SDL_SetRenderDrawColor(renderer, ir, ig, ib, 255);
          SDL_RenderDrawPoint(renderer, i, j);

        }
      }

      // Present the renderer to update the screen
      SDL_RenderPresent(renderer);
			frames++;
			std::clog << "\rframe: " << frames << ' ' << std::flush;
    }
		const double compute_time = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - compute_start).count();
		const double fps = double(frames) / compute_time;

		std::clog << "\rframe: " << frames << '\n';
		std::clog << "fps: " << std::fixed << std::setprecision(10) << fps << '\n';

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}

