import cv2
import numpy as np

# --------- EDIT THESE ---------
image_path = "input_image.png"    # input image (any size)
output_cpp = "frame1.cpp"         # output C++ file (contains 64x64 RGB array)
array_name = "frame1"             # C array name

PRESERVE_ASPECT = True            # True = letterbox, False = stretch
ZIGZAG_WIRING  = True             # True = serpentine rows (might be needed for led matrix)
BACKGROUND_COLOR = (0, 0, 0)      # RGB background for letterboxing
# ------------------------------

# Load image (BGR)
img = cv2.imread(image_path)
if img is None:
    raise FileNotFoundError(f"Could not load {image_path}")

# Convert to RGB
img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

h, w, _ = img.shape

# Resize to 64x64
if PRESERVE_ASPECT:
    scale = min(64 / w, 64 / h)
    new_w = int(w * scale)
    new_h = int(h * scale)

    resized = cv2.resize(img, (new_w, new_h), interpolation=cv2.INTER_AREA)

    canvas = np.zeros((64, 64, 3), dtype=np.uint8)
    canvas[:] = BACKGROUND_COLOR

    x_offset = (64 - new_w) // 2
    y_offset = (64 - new_h) // 2
    canvas[y_offset:y_offset+new_h, x_offset:x_offset+new_w] = resized

    img64 = canvas
else:
    img64 = cv2.resize(img, (64, 64), interpolation=cv2.INTER_AREA)

# ---- Generate C++ array ----
lines = []
lines.append("#include <stdint.h>\n\n")
# use static const and a 32-bit type since values are 0xRRGGBB
lines.append(f"static const uint32_t {array_name}[] = {{\n\n")

for y in range(64):
    row_pixels = []

    # Reverse every other row for zig-zag wiring
    x_range = range(63, -1, -1) if (ZIGZAG_WIRING and y % 2 == 1) else range(64)

    for x in x_range:
        r, g, b = map(int, img64[y, x])
        value = (r << 16) | (g << 8) | b   # 0xRRGGBB
        row_pixels.append(f"0x{value:06X}")

    lines.append("  " + ",".join(row_pixels) + ",\n")

lines.append("};\n")

# Write output file
with open(output_cpp, "w") as f:
    f.writelines(lines)

print(f"Saved {array_name} to {output_cpp}")
