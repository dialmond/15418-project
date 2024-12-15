import argparse
import os
from PIL import Image

parser = argparse.ArgumentParser(description="ppm dir -> gif creator")
parser.add_argument("directory", help="ppm dir")
parser.add_argument("-o", "--output", required=False, help="gif file", default=None)

args = parser.parse_args()

ppm_directory = args.directory
output_gif_path = args.output
if output_gif_path is None:
    output_gif_path = ppm_directory.replace("/", "") + ".gif"

frame_files = sorted(os.listdir(ppm_directory))

# load images as frames
frames = []
for frame in frame_files:
    file_path = os.path.join(ppm_directory, frame)
    if os.path.exists(file_path):
        frames.append(Image.open(file_path))

# save as animated gif
if frames:
    frames[0].save(output_gif_path, save_all=True, append_images=frames[1:], duration=100, loop=1)

print(f"gif saved to {output_gif_path}")

