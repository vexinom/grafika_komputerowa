from PIL import Image
import numpy as np

# === USTAW TU NAZWĘ WEJŚCIOWEGO PLIKU ===
INPUT_FILE = "worldmap.png"
OUTPUT_FILE = "output.png"

def rgb_to_heightmap_16bit(input_path, output_path):
    img = Image.open(input_path).convert("RGB")
    arr = np.asarray(img).astype(np.float32)

    # luminancja (grayscale)
    gray = (
        0.2126 * arr[..., 0] +
        0.7152 * arr[..., 1] +
        0.0722 * arr[..., 2]
    )

    # normalizacja do 0–65535
    gray_min = gray.min()
    gray_max = gray.max()

    if gray_max > gray_min:
        gray = (gray - gray_min) / (gray_max - gray_min)
    else:
        gray = np.zeros_like(gray)

    heightmap = (gray * 65535).astype(np.uint16)

    # zapis 16-bit PNG
    out = Image.fromarray(heightmap, mode="I;16")
    out.save(output_path)

    print(f"Zapisano: {output_path}")

if __name__ == "__main__":
    rgb_to_heightmap_16bit(INPUT_FILE, OUTPUT_FILE)