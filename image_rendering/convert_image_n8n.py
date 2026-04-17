#!/usr/bin/env python3
"""
Image to LED Frame Converter for n8n Workflow
Converts arbitrary images to 64x64 RGB hex arrays for LED matrix display
"""

import cv2
import numpy as np
import json
import sys
import argparse
from pathlib import Path


def convert_image_to_frame(input_path, output_path, preserve_aspect=True, zigzag_wiring=True):
    """
    Convert image to 64x64 LED frame array
    
    Args:
        input_path (str): Path to input image
        output_path (str): Path to output JSON file
        preserve_aspect (bool): Preserve aspect ratio with letterboxing
        zigzag_wiring (bool): Apply serpentine pattern for LED matrix wiring
    
    Returns:
        dict: Frame data with metadata
    """
    
    try:
        # Load image
        img = cv2.imread(input_path)
        if img is None:
            raise FileNotFoundError(f"Could not load image: {input_path}")
        
        # Convert BGR to RGB
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        h, w, _ = img.shape
        
        # Resize to 64x64
        if preserve_aspect:
            scale = min(64 / w, 64 / h)
            new_w = int(w * scale)
            new_h = int(h * scale)
            
            resized = cv2.resize(img, (new_w, new_h), interpolation=cv2.INTER_AREA)
            
            # Letterbox on black background
            canvas = np.zeros((64, 64, 3), dtype=np.uint8)
            x_offset = (64 - new_w) // 2
            y_offset = (64 - new_h) // 2
            canvas[y_offset:y_offset+new_h, x_offset:x_offset+new_w] = resized
            
            img64 = canvas
        else:
            img64 = cv2.resize(img, (64, 64), interpolation=cv2.INTER_AREA)
        
        # Convert to hex array
        frame_array = []
        for y in range(64):
            # Reverse every other row for zig-zag wiring
            x_range = range(63, -1, -1) if (zigzag_wiring and y % 2 == 1) else range(64)
            
            for x in x_range:
                r, g, b = map(int, img64[y, x])
                hex_value = (r << 16) | (g << 8) | b
                frame_array.append(f"0x{hex_value:06X}")
        
        # Create output structure
        output_data = {
            "frame_data": frame_array,
            "width": 64,
            "height": 64,
            "total_pixels": 4096,
            "format": "hex_rgb",
            "preserve_aspect": preserve_aspect,
            "zigzag_wiring": zigzag_wiring,
            "source_file": Path(input_path).name
        }
        
        # Write JSON output
        with open(output_path, 'w') as f:
            json.dump(output_data, f, indent=2)
        
        return output_data
    
    except Exception as e:
        print(f"ERROR: {str(e)}", file=sys.stderr)
        sys.exit(1)


def generate_cpp_header(frame_data, array_name="frame_converted"):
    """
    Generate C++ header content from frame data
    
    Args:
        frame_data (list): List of hex color strings
        array_name (str): Name of C++ array variable
    
    Returns:
        str: C++ header content
    """
    lines = ["#include <stdint.h>\n\n"]
    lines.append(f"static const uint32_t {array_name}[] = {{\n\n")
    
    # Add pixels in rows
    for i in range(0, 4096, 64):
        row_pixels = frame_data[i:i+64]
        lines.append("  " + ",".join(row_pixels) + ",\n")
    
    lines.append("};\n")
    return "".join(lines)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Convert image to 64x64 LED frame array"
    )
    parser.add_argument('--input', required=True, help='Input image path')
    parser.add_argument('--output', required=True, help='Output JSON path')
    parser.add_argument('--output-cpp', help='Optional: Output C++ header file')
    parser.add_argument('--array-name', default='frame_converted', help='C++ array variable name')
    parser.add_argument('--no-aspect', action='store_true', help='Stretch image instead of letterbox')
    parser.add_argument('--no-zigzag', action='store_true', help='Disable zig-zag wiring pattern')
    
    args = parser.parse_args()
    
    # Convert image
    result = convert_image_to_frame(
        args.input,
        args.output,
        preserve_aspect=not args.no_aspect,
        zigzag_wiring=not args.no_zigzag
    )
    
    # Optionally generate C++ header
    if args.output_cpp:
        cpp_content = generate_cpp_header(result["frame_data"], args.array_name)
        with open(args.output_cpp, 'w') as f:
            f.write(cpp_content)
        print(f"✓ C++ header written: {args.output_cpp}")
    
    # Output result
    print(json.dumps(result))
    print(f"\n✓ Frame data written: {args.output}")
