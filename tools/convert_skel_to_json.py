#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Batch SKEL to JSON Converter
批量转换 .skel 文件为 .json 文件

Usage:
    python convert_skel_to_json.py
    python convert_skel_to_json.py --data-dir custom_data --target-dir custom_target
"""

import os
import sys
import subprocess
import time
import argparse
from pathlib import Path
from typing import List, Tuple


class SkelToJsonConverter:
    def __init__(self, data_dir: str = "../data", target_dir: str = "../target"):
        self.data_dir = Path(data_dir).resolve()
        self.target_dir = Path(target_dir).resolve()
        self.skel2json_exe = self.target_dir / "skel2json.exe"
        
        # 统计信息
        self.stats = {
            'total_files': 0,
            'converted_files': 0,
            'failed_files': 0,
            'total_time': 0.0,
            'failed_list': []
        }
    
    def check_prerequisites(self) -> bool:
        """检查必要的文件和目录是否存在"""
        if not self.data_dir.exists():
            print(f"❌ 错误: data目录不存在: {self.data_dir}")
            return False
        
        if not self.skel2json_exe.exists():
            print(f"❌ 错误: skel2json.exe不存在: {self.skel2json_exe}")
            return False
        
        return True
    
    def find_skel_files(self) -> List[Path]:
        """递归查找所有 .skel 文件"""
        skel_files = []
        for skel_file in self.data_dir.rglob("*.skel"):
            if skel_file.is_file():
                skel_files.append(skel_file)
        return sorted(skel_files)
    
    def convert_skel_file(self, skel_path: Path) -> Tuple[bool, float, str]:
        """
        转换单个 .skel 文件
        
        Returns:
            (success, elapsed_time, error_message)
        """
        # 输出文件路径：在原文件同目录下，添加 .json 后缀
        json_path = skel_path.with_suffix('.skel.json')
        
        start_time = time.time()
        
        try:
            # 调用 skel2json.exe
            cmd = [str(self.skel2json_exe), str(skel_path), str(json_path)]
            result = subprocess.run(
                cmd, 
                capture_output=True, 
                text=True, 
                timeout=30,
                cwd=self.target_dir.parent  # 设置工作目录
            )
            
            elapsed = time.time() - start_time
            
            if result.returncode == 0 and json_path.exists():
                return True, elapsed, ""
            else:
                error_msg = result.stderr.strip() if result.stderr else "转换失败，未知错误"
                return False, elapsed, error_msg
                
        except subprocess.TimeoutExpired:
            elapsed = time.time() - start_time
            return False, elapsed, "转换超时 (30秒)"
        except Exception as e:
            elapsed = time.time() - start_time
            return False, elapsed, f"执行错误: {str(e)}"
    
    def convert_all(self) -> None:
        """转换所有 .skel 文件"""
        print("=== Spine SKEL to JSON 批量转换器 ===")
        print(f"数据目录: {self.data_dir}")
        print(f"工具目录: {self.target_dir}")
        print(f"转换工具: {self.skel2json_exe}")
        print("=" * 60)
        
        if not self.check_prerequisites():
            return
        
        skel_files = self.find_skel_files()
        if not skel_files:
            print("❌ 未找到任何 .skel 文件")
            return
        
        self.stats['total_files'] = len(skel_files)
        print(f"找到 {len(skel_files)} 个 .skel 文件")
        print("-" * 60)
        
        overall_start = time.time()
        
        for i, skel_path in enumerate(skel_files, 1):
            # 获取相对路径用于显示
            rel_path = skel_path.relative_to(self.data_dir)
            print(f"[{i:3d}/{len(skel_files)}] {rel_path}")
            
            success, elapsed, error = self.convert_skel_file(skel_path)
            
            if success:
                print(f"           ✅ 转换成功 ({elapsed:.3f}s)")
                self.stats['converted_files'] += 1
            else:
                print(f"           ❌ 转换失败 ({elapsed:.3f}s)")
                print(f"           错误: {error}")
                self.stats['failed_files'] += 1
                self.stats['failed_list'].append(f"{rel_path}: {error}")
        
        self.stats['total_time'] = time.time() - overall_start
        self.print_summary()
    
    def print_summary(self) -> None:
        """打印转换统计摘要"""
        print("=" * 60)
        print("=== 转换统计摘要 ===")
        print(f"总文件数: {self.stats['total_files']}")
        print(f"成功转换: {self.stats['converted_files']}")
        print(f"转换失败: {self.stats['failed_files']}")
        
        if self.stats['total_files'] > 0:
            success_rate = self.stats['converted_files'] / self.stats['total_files'] * 100
            print(f"成功率: {success_rate:.1f}%")
        
        if self.stats['converted_files'] > 0:
            avg_time = self.stats['total_time'] / self.stats['converted_files']
            print(f"平均转换时间: {avg_time:.3f}s")
        
        print(f"总耗时: {self.stats['total_time']:.3f}s")
        
        # 显示失败的文件列表
        if self.stats['failed_list']:
            print("\n=== 失败文件列表 ===")
            for failed_file in self.stats['failed_list']:
                print(f"❌ {failed_file}")
        
        # 最终状态
        if self.stats['failed_files'] == 0:
            print("\n🎉 所有文件转换成功!")
        elif self.stats['converted_files'] > 0:
            print(f"\n⚠️  部分文件转换失败，请检查上述错误信息")
        else:
            print(f"\n💥 所有文件转换失败，请检查工具和文件")


def main():
    parser = argparse.ArgumentParser(description='批量转换 .skel 文件为 .json 文件')
    parser.add_argument('--data-dir', default='../data',
                        help='包含 .skel 文件的数据目录 (默认: ../data)')
    parser.add_argument('--target-dir', default='../target',
                        help='包含 skel2json.exe 的目录 (默认: ../target)')
    
    args = parser.parse_args()
    
    try:
        converter = SkelToJsonConverter(args.data_dir, args.target_dir)
        converter.convert_all()
        
        # 设置退出码
        if converter.stats['failed_files'] > 0:
            sys.exit(1)
        else:
            sys.exit(0)
            
    except KeyboardInterrupt:
        print("\n\n⚠️  用户中断转换过程")
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ 转换过程中发生错误: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
