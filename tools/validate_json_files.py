#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
JSON Validation Tool
验证生成的 .skel.json 文件是否能被 Spine 运行时正确加载

Usage:
    python validate_json_files.py
    python validate_json_files.py --data-dir custom_data --target-dir custom_target
"""

import os
import sys
import subprocess
import time
import argparse
import json
from pathlib import Path
from typing import List, Tuple, Optional


class JsonValidator:
    def __init__(self, data_dir: str = "../data", target_dir: str = "../target"):
        self.data_dir = Path(data_dir).resolve()
        self.target_dir = Path(target_dir).resolve()
        self.test_exe = self.target_dir / "test_skel2json.exe"
        
        # 统计信息
        self.stats = {
            'total_files': 0,
            'validated_files': 0,
            'failed_files': 0,
            'missing_atlas_files': 0,
            'total_time': 0.0,
            'failed_list': []
        }
    
    def check_prerequisites(self) -> bool:
        """检查必要的文件和目录是否存在"""
        if not self.data_dir.exists():
            print(f"❌ 错误: data目录不存在: {self.data_dir}")
            return False
        
        if not self.test_exe.exists():
            print(f"❌ 错误: test_skel2json.exe不存在: {self.test_exe}")
            return False
        
        return True
    
    def find_skel_json_files(self) -> List[Path]:
        """递归查找所有 .skel.json 文件"""
        json_files = []
        for json_file in self.data_dir.rglob("*.skel.json"):
            if json_file.is_file():
                json_files.append(json_file)
        return sorted(json_files)
    
    def detect_spine_version_from_json(self, json_path: Path) -> Optional[str]:
        """从 JSON 文件中检测 Spine 版本"""
        try:
            with open(json_path, 'r', encoding='utf-8') as f:
                data = json.load(f)
            
            # 查找版本信息
            skeleton = data.get('skeleton', {})
            spine_version = skeleton.get('spine', '')
            
            if spine_version:
                # 提取主要版本号
                if spine_version.startswith('3.7'):
                    return '3.7'
                elif spine_version.startswith('3.8'):
                    return '3.8'
                elif spine_version.startswith('4.0'):
                    return '4.0'
                elif spine_version.startswith('4.1'):
                    return '4.1'
                elif spine_version.startswith('4.2'):
                    return '4.2'
            
            # 如果无法从 JSON 中检测，尝试从目录名推断
            parent_name = json_path.parent.parent.name
            version_map = {
                '37': '3.7',
                '38': '3.8',
                '40': '4.0',
                '41': '4.1',
                '42': '4.2'
            }
            return version_map.get(parent_name)
            
        except Exception:
            return None
    
    def find_atlas_file(self, json_path: Path) -> Optional[Path]:
        """查找对应的 .atlas 文件"""
        # 首先尝试同名的 atlas 文件
        base_name = json_path.stem.replace('.skel', '')  # 移除 .skel 部分
        atlas_file = json_path.parent / f"{base_name}.atlas"
        
        if atlas_file.exists():
            return atlas_file
        
        # 查找同目录下的所有 .atlas 文件
        for atlas_file in json_path.parent.glob("*.atlas"):
            atlas_name = atlas_file.stem
            # 检查是否有匹配的前缀
            if base_name.startswith(atlas_name) or atlas_name.startswith(base_name):
                return atlas_file
        
        return None
    
    def validate_json_structure(self, json_path: Path) -> Tuple[bool, str]:
        """验证 JSON 文件的基本结构"""
        try:
            with open(json_path, 'r', encoding='utf-8') as f:
                data = json.load(f)
            
            # 检查必要的字段
            required_fields = ['skeleton', 'bones', 'slots']
            missing_fields = []
            
            for field in required_fields:
                if field not in data:
                    missing_fields.append(field)
            
            if missing_fields:
                return False, f"缺少必要字段: {', '.join(missing_fields)}"
            
            return True, ""
            
        except json.JSONDecodeError as e:
            return False, f"JSON 格式错误: {str(e)}"
        except Exception as e:
            return False, f"读取文件错误: {str(e)}"
    
    def validate_with_spine_runtime(self, json_path: Path, atlas_path: Path) -> Tuple[bool, float, str]:
        """
        使用 Spine 运行时验证 JSON 文件
        
        Returns:
            (success, elapsed_time, error_message)
        """
        start_time = time.time()
        
        try:
            cmd = [str(self.test_exe), str(json_path), str(atlas_path)]
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=30,
                cwd=self.target_dir.parent
            )
            
            elapsed = time.time() - start_time
            
            if result.returncode == 0:
                return True, elapsed, ""
            else:
                error_msg = result.stderr.strip() if result.stderr else "验证失败，未知错误"
                return False, elapsed, error_msg
                
        except subprocess.TimeoutExpired:
            elapsed = time.time() - start_time
            return False, elapsed, "验证超时 (30秒)"
        except Exception as e:
            elapsed = time.time() - start_time
            return False, elapsed, f"执行错误: {str(e)}"
    
    def validate_json_file(self, json_path: Path) -> Tuple[bool, float, str]:
        """验证单个 JSON 文件"""
        # 1. 检测 Spine 版本
        version = self.detect_spine_version_from_json(json_path)
        if not version:
            return False, 0.0, "无法检测 Spine 版本"
        
        # 2. 验证 JSON 结构
        structure_valid, structure_error = self.validate_json_structure(json_path)
        if not structure_valid:
            return False, 0.0, f"JSON 结构无效: {structure_error}"
        
        # 3. 查找 Atlas 文件
        atlas_path = self.find_atlas_file(json_path)
        if not atlas_path:
            return False, 0.0, "找不到对应的 .atlas 文件"
        
        # 4. 使用 Spine 运行时验证
        success, elapsed, error = self.validate_with_spine_runtime(json_path, atlas_path)
        
        return success, elapsed, error
    
    def validate_all(self) -> None:
        """验证所有 .skel.json 文件"""
        print("=== Spine JSON 文件验证器 ===")
        print(f"数据目录: {self.data_dir}")
        print(f"工具目录: {self.target_dir}")
        print(f"验证工具: {self.test_exe}")
        print("=" * 60)
        
        if not self.check_prerequisites():
            return
        
        json_files = self.find_skel_json_files()
        if not json_files:
            print("❌ 未找到任何 .skel.json 文件")
            print("💡 请先运行 convert_skel_to_json.py 生成 JSON 文件")
            return
        
        self.stats['total_files'] = len(json_files)
        print(f"找到 {len(json_files)} 个 .skel.json 文件")
        print("-" * 60)
        
        overall_start = time.time()
        
        for i, json_path in enumerate(json_files, 1):
            # 获取相对路径用于显示
            rel_path = json_path.relative_to(self.data_dir)
            print(f"[{i:3d}/{len(json_files)}] {rel_path}")
            
            success, elapsed, error = self.validate_json_file(json_path)
            
            if success:
                print(f"           ✅ 验证成功 ({elapsed:.3f}s)")
                self.stats['validated_files'] += 1
            else:
                print(f"           ❌ 验证失败 ({elapsed:.3f}s)")
                print(f"           错误: {error}")
                self.stats['failed_files'] += 1
                
                if "找不到对应的 .atlas 文件" in error:
                    self.stats['missing_atlas_files'] += 1
                
                self.stats['failed_list'].append(f"{rel_path}: {error}")
        
        self.stats['total_time'] = time.time() - overall_start
        self.print_summary()
    
    def print_summary(self) -> None:
        """打印验证统计摘要"""
        print("=" * 60)
        print("=== 验证统计摘要 ===")
        print(f"总文件数: {self.stats['total_files']}")
        print(f"验证成功: {self.stats['validated_files']}")
        print(f"验证失败: {self.stats['failed_files']}")
        print(f"缺少Atlas: {self.stats['missing_atlas_files']}")
        
        if self.stats['total_files'] > 0:
            success_rate = self.stats['validated_files'] / self.stats['total_files'] * 100
            print(f"成功率: {success_rate:.1f}%")
        
        if self.stats['validated_files'] > 0:
            avg_time = self.stats['total_time'] / self.stats['validated_files']
            print(f"平均验证时间: {avg_time:.3f}s")
        
        print(f"总耗时: {self.stats['total_time']:.3f}s")
        
        # 显示失败的文件列表
        if self.stats['failed_list']:
            print("\n=== 失败文件列表 ===")
            for failed_file in self.stats['failed_list']:
                print(f"❌ {failed_file}")
        
        # 最终状态
        if self.stats['failed_files'] == 0:
            print("\n🎉 所有文件验证成功!")
        elif self.stats['validated_files'] > 0:
            print(f"\n⚠️  部分文件验证失败，请检查上述错误信息")
        else:
            print(f"\n💥 所有文件验证失败，请检查工具和文件")


def main():
    parser = argparse.ArgumentParser(description='验证 .skel.json 文件是否能被 Spine 运行时正确加载')
    parser.add_argument('--data-dir', default='../data',
                        help='包含 .skel.json 文件的数据目录 (默认: ../data)')
    parser.add_argument('--target-dir', default='../target',
                        help='包含 test_skel2json.exe 的目录 (默认: ../target)')
    
    args = parser.parse_args()
    
    try:
        validator = JsonValidator(args.data_dir, args.target_dir)
        validator.validate_all()
        
        # 设置退出码
        if validator.stats['failed_files'] > 0:
            sys.exit(1)
        else:
            sys.exit(0)
            
    except KeyboardInterrupt:
        print("\n\n⚠️  用户中断验证过程")
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ 验证过程中发生错误: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
