#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>

struct Pixel {
  int r, g, b;
};

// normalized color difference
double calculate_color_difference(const Pixel& p1, const Pixel& p2) {
  double diff_r = std::abs(p1.r - p2.r) / 255.0;
  double diff_g = std::abs(p1.g - p2.g) / 255.0;
  double diff_b = std::abs(p1.b - p2.b) / 255.0;

  return (diff_r + diff_g + diff_b) / 3.0;
}

std::vector<Pixel> read(const std::string& filename, int& width, int& height) {
  std::ifstream file(filename);

  std::string format;
  file >> format;

  file >> width >> height;
  int max_val;
  file >> max_val;

  std::vector<Pixel> pixels(width * height);
  for (int i = 0; i < width * height; ++i) {
    int r, g, b;
    file >> r >> g >> b;
    pixels[i] = {r, g, b};
  }

  return pixels;
}

double compare_ppm(const std::string& file1, const std::string& file2) {
  int width1, height1, width2, height2;
  auto pixels1 = read(file1, width1, height1);
  auto pixels2 = read(file2, width2, height2);

  if (width1 != width2 || height1 != height2) {
    throw std::runtime_error("images different sizes");
  }

  double total_diff = 0.0;
  for (size_t i = 0; i < pixels1.size(); ++i) {
    total_diff += calculate_color_difference(pixels1[i], pixels2[i]);
  }

  return (total_diff / pixels1.size()) * 100.0;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <file1.ppm> <file2.ppm>" << std::endl;
    return 1;
  }

  std::string file1 = argv[1];
  std::string file2 = argv[2];

  try {
    double difference = compare_ppm(file1, file2);
    std::cout << "percentage difference between images: " << difference << " %" << std::endl;
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

