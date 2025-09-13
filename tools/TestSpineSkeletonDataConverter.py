#!/usr/bin/env python3
"""
测试命令行工具
递归处理指定目录下的文件，使用 SpineSkeletonDataConverter.exe 进行格式转换
"""

import os
import sys
import argparse
import subprocess
import tempfile
import shutil
from pathlib import Path
from typing import List, Tuple

class SpineConverter:
    def __init__(self, exe_path: str):
        """
        初始化转换器
        
        Args:
            exe_path: SpineSkeletonDataConverter.exe 的路径
        """
        self.exe_path = exe_path
        if not os.path.exists(exe_path):
            raise FileNotFoundError(f"SpineSkeletonDataConverter.exe not found at: {exe_path}")
    
    def get_format_from_extension(self, ext: str) -> str:
        """
        根据文件扩展名确定格式
        
        Args:
            ext: 文件扩展名
            
        Returns:
            格式字符串 ('json' 或 'skel')
        """
        ext = ext.lower()
        if ext == '.json':
            return 'json'
        elif ext == '.skel':
            return 'skel'
        else:
            raise ValueError(f"Unsupported file extension: {ext}")
    
    def parse_conversion_chain(self, output_suffix: str) -> List[str]:
        """
        解析转换链
        
        Args:
            output_suffix: 输出后缀，如 '.json.skel.json'
            
        Returns:
            转换步骤列表，如 ['json', 'skel', 'json']
        """
        # 移除开头的点并分割
        if output_suffix.startswith('.'):
            output_suffix = output_suffix[1:]
        
        parts = output_suffix.split('.')
        formats = []
        
        for part in parts:
            if part.lower() in ['json', 'skel']:
                formats.append(part.lower())
            else:
                raise ValueError(f"Unsupported format in output suffix: {part}")
        
        return formats
    
    def convert_file(self, input_file: str, input_format: str, output_format: str, output_file: str) -> bool:
        """
        转换单个文件
        
        Args:
            input_file: 输入文件路径
            input_format: 输入格式 ('json' 或 'skel')
            output_format: 输出格式 ('json' 或 'skel')
            output_file: 输出文件路径
            
        Returns:
            转换是否成功
        """
        try:
            cmd = [
                self.exe_path,
                input_file,
                output_file,
                f'--in-{input_format}',
                f'--out-{output_format}'
            ]
            
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            
            if result.returncode == 0:
                return True
            else:
                print(f"Error converting {input_file}: {result.stderr}")
                return False
                
        except subprocess.TimeoutExpired:
            print(f"Timeout converting {input_file}")
            return False
        except Exception as e:
            print(f"Exception converting {input_file}: {e}")
            return False
    
    def convert_with_chain(self, input_file: str, input_format: str, conversion_chain: List[str], input_suffix: str, output_suffix: str) -> bool:
        """
        按转换链进行多步转换
        
        Args:
            input_file: 输入文件路径
            input_format: 输入格式
            conversion_chain: 转换链
            input_suffix: 输入文件后缀
            output_suffix: 输出后缀
            
        Returns:
            转换是否成功
        """
        if not conversion_chain:
            return True
        
        # 创建临时目录用于中间文件
        with tempfile.TemporaryDirectory() as temp_dir:
            current_file = input_file
            current_format = input_format
            
            for i, target_format in enumerate(conversion_chain):
                # 确定输出文件
                if i == len(conversion_chain) - 1:
                    # 最后一步，输出到最终文件
                    output_file = self.get_final_output_path(input_file, input_suffix, output_suffix)
                else:
                    # 中间步骤，输出到临时文件
                    temp_name = f"temp_{i}.{target_format}"
                    output_file = os.path.join(temp_dir, temp_name)
                
                # 执行转换
                if not self.convert_file(current_file, current_format, target_format, output_file):
                    return False
                
                # 更新当前文件和格式
                current_file = output_file
                current_format = target_format
        
        return True
    
    def get_final_output_path(self, input_file: str, input_suffix: str, output_suffix: str) -> str:
        """
        生成最终输出文件路径
        
        Args:
            input_file: 输入文件路径
            input_suffix: 输入文件后缀
            output_suffix: 输出后缀（转换路径）
            
        Returns:
            输出文件路径
        """
        input_path = Path(input_file)
        
        # 获取文件名，去掉输入后缀
        filename = input_path.name
        if filename.endswith(input_suffix):
            base_name = filename[:-len(input_suffix)]
        else:
            base_name = input_path.stem
        
        parent_dir = input_path.parent
        
        # 构造输出文件名：base_name + input_suffix + output_suffix
        output_name = f"{base_name}{input_suffix}{output_suffix}"
        
        return str(parent_dir / output_name)

def find_files_with_suffix(directory: str, suffix: str) -> List[str]:
    """
    递归查找指定后缀的文件
    
    Args:
        directory: 搜索目录
        suffix: 文件后缀（如 '.json'）- 完整后缀，前面不能有额外的点
        
    Returns:
        匹配的文件路径列表
    """
    files = []
    suffix = suffix.lower()
    
    for root, dirs, filenames in os.walk(directory):
        for filename in filenames:
            # 检查文件是否以指定后缀结尾
            if filename.lower().endswith(suffix):
                # 检查是否是完整后缀文件（前面不能有额外的点）
                name_without_suffix = filename[:-len(suffix)]
                # 确保去掉后缀后的文件名中不包含点，或者是我们想要的完整匹配
                if '.' not in name_without_suffix or name_without_suffix.endswith('.'):
                    # 如果 name_without_suffix 以 '.' 结尾，说明原文件名是 "name..json" 这种形式，跳过
                    if not name_without_suffix.endswith('.'):
                        files.append(os.path.join(root, filename))
    
    return files

def main():
    parser = argparse.ArgumentParser(
        description='递归测试 SpineSkeletonDataConverter 格式转换',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例用法:
  %(prog)s /path/to/files .json .skel
    将所有 .json 文件转换为 .skel，生成 .json.skel 文件
    
  %(prog)s /path/to/files .json .skel.json
    将所有 .json 文件先转为 .skel 再转为 .json，生成 .json.skel.json 文件
    
  %(prog)s /path/to/files .skel .json --exe ./build/Debug/SpineSkeletonDataConverter.exe
    将所有 .skel 文件转换为 .json，生成 .skel.json 文件
        """
    )
    
    parser.add_argument('directory', help='要处理的目录路径')
    parser.add_argument('input_suffix', help='输入文件后缀 (如 .json) - 完整后缀，前面不能有额外的点')
    parser.add_argument('conversion_path', help='转换路径 (如 .skel 或 .skel.json)')
    parser.add_argument('--exe', default='./build/Debug/SpineSkeletonDataConverter.exe', 
                       help='SpineSkeletonDataConverter.exe 的路径')
    parser.add_argument('--dry-run', action='store_true', 
                       help='只显示将要处理的文件，不执行实际转换')
    
    args = parser.parse_args()
    
    # 验证参数
    if not os.path.exists(args.directory):
        print(f"错误：目录不存在: {args.directory}")
        sys.exit(1)
    
    if not args.input_suffix.startswith('.'):
        args.input_suffix = '.' + args.input_suffix
    
    if not args.conversion_path.startswith('.'):
        args.conversion_path = '.' + args.conversion_path
    
    try:
        # 初始化转换器
        converter = SpineConverter(args.exe)
        
        # 解析转换链
        conversion_chain = converter.parse_conversion_chain(args.conversion_path)
        print(f"转换链: {' -> '.join(conversion_chain)}")
        
        # 确定输入格式
        input_format = converter.get_format_from_extension(args.input_suffix)
        
        # 查找文件
        files = find_files_with_suffix(args.directory, args.input_suffix)
        
        if not files:
            print(f"在目录 {args.directory} 中没有找到后缀为 {args.input_suffix} 的文件")
            sys.exit(0)
        
        print(f"找到 {len(files)} 个文件")
        
        if args.dry_run:
            print("\n将要处理的文件:")
            for file in files:
                output_file = converter.get_final_output_path(file, args.input_suffix, args.conversion_path)
                print(f"  {file} -> {output_file}")
            sys.exit(0)
        
        # 处理文件
        success_count = 0
        total_count = len(files)
        
        for i, file in enumerate(files, 1):
            print(f"[{i}/{total_count}] 处理: {file}")
            
            if converter.convert_with_chain(file, input_format, conversion_chain, args.input_suffix, args.conversion_path):
                success_count += 1
                output_file = converter.get_final_output_path(file, args.input_suffix, args.conversion_path)
                print(f"  -> 成功: {output_file}")
            else:
                print(f"  -> 失败")
        
        print(f"\n转换完成: {success_count}/{total_count} 成功")
        
    except Exception as e:
        print(f"错误: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()