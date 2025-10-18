#!/usr/bin/env python3
"""
Spine Atlas 4.x to 3.x Downgrader

This script converts a Spine 4.x atlas file to a 3.x compatible format.
It performs two main tasks:
1. Converts the .atlas file, applying the 'scale' property to all relevant metrics.
2. Scales the associated PNG texture images according to the 'scale' property.

Usage:
    python SpineAtlasDowngrade.py input.atlas output_dir
"""

import os
import sys
import shutil
import argparse
from pathlib import Path
from typing import List
from PIL import Image

# ============================================================================
# Atlas Data Structures
# ============================================================================

class AtlasRegion:
    """Represents a region in an atlas page."""
    def __init__(self):
        self.name = ""
        self.x = 0
        self.y = 0
        self.width = 0
        self.height = 0
        self.offset_x = 0
        self.offset_y = 0
        self.original_width = 0
        self.original_height = 0
        self.degrees = 0
        self.index = -1
        self.splits = []
        self.pads = []
        # Support for custom properties
        self.names = []
        self.values = []

class AtlasPage:
    """Represents a page in an atlas, typically a single texture file."""
    def __init__(self):
        self.name = ""
        self.width = 0
        self.height = 0
        self.format = "RGBA8888"
        self.min_filter = "Nearest"
        self.mag_filter = "Nearest"
        self.repeat = "none"
        self.pma = False
        self.scale = 1.0  # 4.x scale property
        self.regions: List[AtlasRegion] = []

class AtlasData:
    """Represents the entire atlas data, containing multiple pages."""
    def __init__(self):
        self.pages: List[AtlasPage] = []

# ============================================================================
# Atlas Parsing and Writing
# ============================================================================

def parse_entry(line: str) -> tuple[str, list[str]]:
    """Parses a key-value pair line from the atlas file."""
    if ':' not in line:
        return "", []
    
    key, value_str = line.split(':', 1)
    key = key.strip()
    value_str = value_str.strip()
    
    values = [v.strip() for v in value_str.split(',')]
    return key, values

def read_atlas_data_4x(content: str) -> AtlasData:
    """Parses the content of a 4.x atlas file into an AtlasData object."""
    atlas_data = AtlasData()
    lines = content.split('\n')
    current_page = None
    
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        
        if not line:
            i += 1
            continue
        
        # Check if the line is a page name (no colon)
        if ':' not in line:
            # Look ahead to see if the next line is a page property (e.g., "size:")
            is_page_start = False
            if i + 1 < len(lines):
                next_line = lines[i + 1].strip()
                if next_line.startswith("size:"):
                    is_page_start = True
            
            if is_page_start:
                # New page
                page = AtlasPage()
                page.name = line
                atlas_data.pages.append(page)
                current_page = atlas_data.pages[-1]
            elif current_page:
                # New region
                region = AtlasRegion()
                region.name = line
                current_page.regions.append(region)
            
            i += 1
            continue
        
        # Parse key-value pairs
        key, values = parse_entry(line)
        if not key:
            i += 1
            continue
        
        # Page properties
        if (current_page is not None and not current_page.regions and
            key in ["size", "format", "filter", "repeat", "pma", "scale"]):
            if key == "size" and len(values) >= 2:
                current_page.width = int(values[0])
                current_page.height = int(values[1])
            elif key == "format" and values:
                current_page.format = values[0]
            elif key == "filter" and len(values) >= 2:
                current_page.min_filter = values[0]
                current_page.mag_filter = values[1]
            elif key == "repeat" and values:
                current_page.repeat = values[0]
            elif key == "pma" and values:
                current_page.pma = (values[0].lower() == "true")
            elif key == "scale" and values:
                current_page.scale = float(values[0])
        
        # Region properties
        elif current_page and current_page.regions:
            region = current_page.regions[-1]
            
            if key == "bounds" and len(values) >= 4:
                region.x, region.y, region.width, region.height = map(int, values)
            elif key == "xy" and len(values) >= 2:
                region.x, region.y = map(int, values)
            elif key == "size" and len(values) >= 2:
                region.width, region.height = map(int, values)
            elif key == "offset" and len(values) >= 2:
                region.offset_x, region.offset_y = map(int, values)
            elif key == "offsets" and len(values) >= 4:
                region.offset_x = int(values[0])
                region.offset_y = int(values[1])
                region.original_width = int(values[2])
                region.original_height = int(values[3])
            elif key == "orig" and len(values) >= 2:
                region.original_width, region.original_height = map(int, values)
            elif key == "rotate" and values:
                if values[0].lower() == "true":
                    region.degrees = 90
                elif values[0].lower() != "false":
                    region.degrees = int(values[0])
            elif key == "index" and values:
                region.index = int(values[0])
            elif key == "split" and len(values) >= 4:
                region.splits = [int(val) for val in values]
            elif key == "pad" and len(values) >= 4:
                region.pads = [int(val) for val in values]
            else:
                region.names.append(key)
                region.values.extend(map(int, values))
        
        i += 1
    
    return atlas_data

def write_atlas_data_3x(atlas_data: AtlasData) -> str:
    """Generates a 3.x compatible atlas string from an AtlasData object."""
    output_lines = []
    
    for page in atlas_data.pages:
        output_lines.append(page.name)
        
        # Apply scale to page size
        width = int(page.width / page.scale) if page.scale != 1.0 else page.width
        height = int(page.height / page.scale) if page.scale != 1.0 else page.height
        output_lines.append(f"size: {width}, {height}")
        
        output_lines.append(f"format: {page.format}")
        output_lines.append(f"filter: {page.min_filter}, {page.mag_filter}")
        output_lines.append(f"repeat: {page.repeat}")
        
        for region in page.regions:
            output_lines.append(region.name)
            
            # rotate (support numeric angles)
            if region.degrees == 90:
                output_lines.append("  rotate: true")
            elif region.degrees == 0:
                output_lines.append("  rotate: false")
            else:
                output_lines.append(f"  rotate: {region.degrees}")
            
            # Apply scale to all region metrics
            scale = page.scale
            x = int(region.x / scale) if scale != 1.0 else region.x
            y = int(region.y / scale) if scale != 1.0 else region.y
            output_lines.append(f"  xy: {x}, {y}")
            
            r_width = int(region.width / scale) if scale != 1.0 else region.width
            r_height = int(region.height / scale) if scale != 1.0 else region.height
            output_lines.append(f"  size: {r_width}, {r_height}")
            
            if region.splits and len(region.splits) >= 4:
                splits_source = region.splits[:4]
                splits = [str(int(s / scale)) if scale != 1.0 else str(s) for s in splits_source]
                output_lines.append(f"  split: {', '.join(splits)}")

            if region.pads and len(region.pads) >= 4:
                pads_source = region.pads[:4]
                pads = [str(int(p / scale)) if scale != 1.0 else str(p) for p in pads_source]
                output_lines.append(f"  pad: {', '.join(pads)}")
            
            orig_width = region.original_width if region.original_width > 0 else region.width
            orig_height = region.original_height if region.original_height > 0 else region.height
            if scale != 1.0:
                orig_width = int(orig_width / scale)
                orig_height = int(orig_height / scale)
            output_lines.append(f"  orig: {orig_width}, {orig_height}")
            
            offset_x = int(region.offset_x / scale) if scale != 1.0 else region.offset_x
            offset_y = int(region.offset_y / scale) if scale != 1.0 else region.offset_y
            output_lines.append(f"  offset: {offset_x}, {offset_y}")
            
            output_lines.append(f"  index: {region.index}")
        
        output_lines.append("")
    
    return '\n'.join(output_lines)

# ============================================================================
# Image Scaling
# ============================================================================

def scale_png_images(atlas_data: AtlasData, atlas_dir: Path, output_dir: Path):
    """Scales PNG texture images based on the scale property in the atlas data."""
    print("Processing PNG images:")
    for page in atlas_data.pages:
        png_name = os.path.basename(page.name)
        input_path = atlas_dir / png_name
        output_path = output_dir / png_name

        if not input_path.exists():
            print(f"  ✗ PNG file not found: {input_path}")
            continue
        
        if page.scale == 1.0:
            shutil.copy2(input_path, output_path)
            print(f"  ✓ Copied {png_name} (scale=1.0, no scaling needed)")
        else:
            try:
                with Image.open(input_path) as img:
                    new_width = int(img.width / page.scale)
                    new_height = int(img.height / page.scale)
                    
                    scaled_img = img.resize((new_width, new_height), Image.Resampling.LANCZOS)
                    scaled_img.save(output_path)
                    print(f"  ✓ Scaled {png_name}: {img.width}x{img.height} → {new_width}x{new_height} (scale={page.scale})")
            except Exception as e:
                print(f"  ✗ Error scaling {png_name}: {e}")

# ============================================================================
# Main Function
# ============================================================================

def main():
    parser = argparse.ArgumentParser(description='Convert Spine 4.x atlas to 3.x format.')
    parser.add_argument('input_atlas', help='Input 4.x atlas file')
    parser.add_argument('output_dir', help='Output directory for converted files')
    
    args = parser.parse_args()
    
    input_path = Path(args.input_atlas)
    output_dir = Path(args.output_dir)
    
    if not input_path.exists():
        print(f"Error: Input atlas file '{input_path}' not found")
        sys.exit(1)
    
    output_dir.mkdir(exist_ok=True)
    
    print(f"Converting Spine 4.x atlas: {input_path.name}")
    print(f"Output directory: {output_dir}")
    print("-" * 50)
    
    # Read 4.x atlas content
    with open(input_path, 'r', encoding='utf-8') as f:
        atlas_4x_content = f.read()
    
    # Parse the atlas data
    atlas_data = read_atlas_data_4x(atlas_4x_content)
    
    # Generate 3.x atlas content
    atlas_3x_content = write_atlas_data_3x(atlas_data)
    
    # Save the converted atlas file
    output_atlas_path = output_dir / input_path.name
    with open(output_atlas_path, 'w', encoding='utf-8') as f:
        f.write(atlas_3x_content)
    print(f"✓ Atlas file converted successfully: {output_atlas_path}")
    
    # Scale the associated PNG images
    atlas_dir = input_path.parent
    scale_png_images(atlas_data, atlas_dir, output_dir)
    
    print("-" * 50)
    print("✓ Atlas downgrade completed successfully!")

if __name__ == "__main__":
    main()
