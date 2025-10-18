#!/usr/bin/env python3
"""
Spine 4.x to 3.8 Converter
Complete converter for Spine skeleton data from 4.x to 3.8 format

This tool implements 3 main tasks:
1. Convert skeleton data using SpineSkeletonDataConverter executable
2. Convert 4.x atlas to 3.8 format and extract page->scale mapping
3. Scale PNG images according to the scale mapping

Usage:
    python Spine4xTo38Converter.py input.skel output_dir --converter path/to/SpineSkeletonDataConverter.exe
    python Spine4xTo38Converter.py input.json output_dir --converter path/to/SpineSkeletonDataConverter.exe
"""

import os
import sys
import shutil
import argparse
import subprocess
from pathlib import Path
from typing import Optional, List
from PIL import Image


# ============================================================================
# Task 1: Skeleton Data Conversion
# ============================================================================

def convert_skeleton_data(input_file: str, output_file: str, converter_path: str) -> bool:
    """任务1：使用SpineSkeletonDataConverter转换skeleton数据"""
    try:
        cmd = [
            converter_path,
            input_file,
            output_file,
            '-v', '3.8.75'
        ]
        
        print(f"Converting skeleton: {os.path.basename(input_file)}")
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"✓ Skeleton conversion successful")
            return True
        else:
            print(f"✗ Skeleton conversion failed:")
            print(f"  stdout: {result.stdout}")
            print(f"  stderr: {result.stderr}")
            return False
    
    except Exception as e:
        print(f"✗ Error running converter: {e}")
        return False


# ============================================================================
# Task 2: Atlas Format Conversion (4.x -> 3.8)
# ============================================================================

# Atlas数据结构类
class AtlasRegion:
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
        # 自定义属性支持
        self.names = []
        self.values = []


class AtlasPage:
    def __init__(self):
        self.name = ""
        self.width = 0
        self.height = 0
        self.format = "RGBA8888"
        self.min_filter = "Nearest"
        self.mag_filter = "Nearest"
        self.repeat = "none"
        self.pma = False
        self.scale = 1.0  # 4.x的scale属性
        self.regions: List[AtlasRegion] = []


class AtlasData:
    def __init__(self):
        self.pages: List[AtlasPage] = []


def parse_entry(line: str) -> tuple[str, list[str]]:
    """解析键值对"""
    if ':' not in line:
        return "", []
    
    key, value_str = line.split(':', 1)
    key = key.strip()
    value_str = value_str.strip()
    
    values = [v.strip() for v in value_str.split(',')]
    return key, values


def read_atlas_data_4x(content: str) -> AtlasData:
    """子任务2.1：读取4.x atlas数据，解析包括scale信息"""
    atlas_data = AtlasData()
    lines = content.split('\n')
    current_page = None
    
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        
        # 跳过空行
        if not line:
            i += 1
            continue
        
        # 检查是否是页面名称（不包含冒号的行）
        if ':' not in line:
            # 检查下一行是否包含页面属性（如size:）
            is_page_start = False
            if i + 1 < len(lines):
                next_line = lines[i + 1].strip()
                if next_line.startswith("size:"):
                    is_page_start = True
            
            if is_page_start:
                # 新页面
                page = AtlasPage()
                page.name = line
                atlas_data.pages.append(page)
                current_page = atlas_data.pages[-1]
            elif current_page:
                # 新区域
                region = AtlasRegion()
                region.name = line
                current_page.regions.append(region)
            
            i += 1
            continue
        
        # 解析键值对
        key, values = parse_entry(line)
        if not key:
            i += 1
            continue
        
        if (current_page is not None and not current_page.regions and
            key in ["size", "format", "filter", "repeat", "pma", "scale"]):
            # 页面属性
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
                current_page.scale = float(values[0])  # 保存scale信息
        
        elif current_page and current_page.regions:
            # 区域属性
            region = current_page.regions[-1]
            
            if key == "bounds" and len(values) >= 4:
                region.x = int(values[0])
                region.y = int(values[1])
                region.width = int(values[2])
                region.height = int(values[3])
            elif key == "xy" and len(values) >= 2:
                region.x = int(values[0])
                region.y = int(values[1])
            elif key == "size" and len(values) >= 2:
                region.width = int(values[0])
                region.height = int(values[1])
            elif key == "offset" and len(values) >= 2:
                region.offset_x = int(values[0])
                region.offset_y = int(values[1])
            elif key == "offsets" and len(values) >= 4:
                region.offset_x = int(values[0])
                region.offset_y = int(values[1])
                region.original_width = int(values[2])
                region.original_height = int(values[3])
            elif key == "orig" and len(values) >= 2:
                region.original_width = int(values[0])
                region.original_height = int(values[1])
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
                for val in values:
                    region.values.append(int(val))
        
        i += 1
    
    return atlas_data


def write_atlas_data_38(atlas_data: AtlasData) -> str:
    """子任务2.2：写入3.8格式atlas，应用scale到各个数值"""
    output_lines = []
    
    for page in atlas_data.pages:
        # 页面名称
        output_lines.append(page.name)
        
        # size (必需) - 应用scale
        width = int(page.width / page.scale) if page.scale != 1.0 else page.width
        height = int(page.height / page.scale) if page.scale != 1.0 else page.height
        output_lines.append(f"size: {width}, {height}")
        
        # format (必需)
        output_lines.append(f"format: {page.format}")
        
        # filter (必需)
        output_lines.append(f"filter: {page.min_filter}, {page.mag_filter}")
        
        # repeat (必需)
        output_lines.append(f"repeat: {page.repeat}")
        
        # 输出此页面的所有区域
        for region in page.regions:
            # 区域名称
            output_lines.append(region.name)
            
            # rotate (必需)
            if region.degrees == 90:
                output_lines.append("  rotate: true")
            elif region.degrees == 0:
                output_lines.append("  rotate: false")
            else:
                output_lines.append(f"  rotate: {region.degrees}")
            
            # xy (必需) - 应用scale
            x = int(region.x / page.scale) if page.scale != 1.0 else region.x
            y = int(region.y / page.scale) if page.scale != 1.0 else region.y
            output_lines.append(f"  xy: {x}, {y}")
            
            # size (必需) - 应用scale
            width = int(region.width / page.scale) if page.scale != 1.0 else region.width
            height = int(region.height / page.scale) if page.scale != 1.0 else region.height
            output_lines.append(f"  size: {width}, {height}")
            
            # split (可选，仅当存在时) - 应用scale
            if region.splits and len(region.splits) >= 4:
                if page.scale != 1.0:
                    splits = [str(int(s / page.scale)) for s in region.splits[:4]]
                else:
                    splits = [str(s) for s in region.splits[:4]]
                output_lines.append(f"  split: {', '.join(splits)}")
            
            # pad (可选，仅当存在split时) - 应用scale
            if region.pads and len(region.pads) >= 4:
                if page.scale != 1.0:
                    pads = [str(int(p / page.scale)) for p in region.pads[:4]]
                else:
                    pads = [str(p) for p in region.pads[:4]]
                output_lines.append(f"  pad: {', '.join(pads)}")
            
            # orig (必需) - 应用scale
            orig_width = region.original_width if region.original_width > 0 else region.width
            orig_height = region.original_height if region.original_height > 0 else region.height
            if page.scale != 1.0:
                orig_width = int(orig_width / page.scale)
                orig_height = int(orig_height / page.scale)
            output_lines.append(f"  orig: {orig_width}, {orig_height}")
            
            # offset (必需) - 应用scale
            offset_x = int(region.offset_x / page.scale) if page.scale != 1.0 else region.offset_x
            offset_y = int(region.offset_y / page.scale) if page.scale != 1.0 else region.offset_y
            output_lines.append(f"  offset: {offset_x}, {offset_y}")
            
            # index (必需)
            output_lines.append(f"  index: {region.index}")
        
        # 页面之间有空行
        output_lines.append("")
    
    return '\n'.join(output_lines)


def convert_atlas_4x_to_38(atlas_content: str) -> tuple[str, AtlasData]:
    """任务2：转换4.x atlas到3.8格式
    
    Returns:
        tuple[str, AtlasData]: (转换后的atlas内容, atlas数据对象)
    """
    # 子任务2.1：读取4.x atlas数据
    atlas_data = read_atlas_data_4x(atlas_content)
    
    # 子任务2.2：写入3.8格式，应用scale
    converted_content = write_atlas_data_38(atlas_data)
    
    return converted_content, atlas_data


# ============================================================================
# Task 3: Image Scaling - 使用atlasData中的scale信息
# ============================================================================

def scale_png_images(atlas_data: AtlasData, atlas_dir: str, output_dir: str):
    """任务3：根据atlasData中的scale信息缩放PNG图片文件"""
    for page in atlas_data.pages:
        # 处理页面名称中可能包含的路径
        png_name = os.path.basename(page.name)
        if png_name.endswith('.png'):
            png_name = png_name[:-4]
        
        input_path = os.path.join(atlas_dir, png_name + '.png')
        output_path = os.path.join(output_dir, png_name + '.png')
        
        if not os.path.exists(input_path):
            print(f"  ✗ PNG file not found: {png_name}.png")
            continue
        
        if page.scale == 1.0:
            # 不需要缩放，直接复制
            shutil.copy2(input_path, output_path)
            print(f"  ✓ Copied {png_name}.png (scale=1.0, no scaling needed)")
        else:
            # 需要缩放
            try:
                with Image.open(input_path) as img:
                    new_width = int(img.width / page.scale)
                    new_height = int(img.height / page.scale)
                    
                    scaled_img = img.resize((new_width, new_height), Image.Resampling.LANCZOS)
                    scaled_img.save(output_path)
                    print(f"  ✓ Scaled {png_name}.png: {img.width}x{img.height} → {new_width}x{new_height} (scale={page.scale})")
                    
            except Exception as e:
                print(f"  ✗ Error scaling {png_name}.png: {e}")


# ============================================================================
# Utility Functions
# ============================================================================

def find_atlas_file(skeleton_file: str) -> Optional[str]:
    """根据skeleton文件查找同名的atlas文件"""
    skeleton_path = Path(skeleton_file)
    atlas_path = skeleton_path.with_suffix('.atlas')
    
    if atlas_path.exists():
        return str(atlas_path)
    return None


# ============================================================================
# Main Function
# ============================================================================

def main():
    parser = argparse.ArgumentParser(description='Convert Spine 4.x project to 3.8 format')
    parser.add_argument('input_file', help='Input skeleton file (.skel or .json)')
    parser.add_argument('output_dir', help='Output directory for converted files')
    parser.add_argument('--converter', required=True, help='Path to SpineSkeletonDataConverter executable')
    
    args = parser.parse_args()
    
    # 验证输入文件
    if not os.path.exists(args.input_file):
        print(f"Error: Input file '{args.input_file}' not found")
        sys.exit(1)
    
    # 验证转换器
    if not os.path.exists(args.converter):
        print(f"Error: Converter executable '{args.converter}' not found")
        sys.exit(1)
    
    # 创建输出目录
    os.makedirs(args.output_dir, exist_ok=True)
    
    input_path = Path(args.input_file)
    base_name = input_path.stem
    atlas_dir = os.path.dirname(args.input_file)
    
    print(f"Converting Spine 4.x project: {input_path.name}")
    print(f"Output directory: {args.output_dir}")
    print("-" * 50)
    
    success = True
    
    # ============================================================================
    # 任务1：转换skeleton数据
    # ============================================================================
    output_skeleton = os.path.join(args.output_dir, f"{base_name}.json")
    if not convert_skeleton_data(args.input_file, output_skeleton, args.converter):
        success = False
    
    # ============================================================================
    # 任务2：转换atlas文件
    # ============================================================================
    atlas_file = find_atlas_file(args.input_file)
    if atlas_file:
        print(f"Found atlas file: {os.path.basename(atlas_file)}")
        
        # 读取4.x atlas内容
        with open(atlas_file, 'r', encoding='utf-8') as f:
            atlas_4x_content = f.read()
        
        # 转换到3.8格式并获取atlas数据
        atlas_38_content, atlas_data = convert_atlas_4x_to_38(atlas_4x_content)
        
        # 保存转换后的atlas文件
        output_atlas = os.path.join(args.output_dir, f"{base_name}.atlas")
        with open(output_atlas, 'w', encoding='utf-8') as f:
            f.write(atlas_38_content)
        print(f"✓ Atlas conversion successful")
        
        # 打印scale信息
        print(f"Found {len(atlas_data.pages)} pages:")
        for page in atlas_data.pages:
            print(f"  {page.name}: scale={page.scale}")
        
        # ============================================================================
        # 任务3：根据atlasData缩放PNG图片
        # ============================================================================
        print("Processing PNG images:")
        scale_png_images(atlas_data, atlas_dir, args.output_dir)
        
    else:
        print("Warning: No atlas file found")
    
    print("-" * 50)
    if success:
        print("✓ Conversion completed successfully!")
        print(f"Output files saved to: {args.output_dir}")
    else:
        print("✗ Conversion completed with errors")
        sys.exit(1)


if __name__ == "__main__":
    main()