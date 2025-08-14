#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
JSON Difference Analysis Tool
分析生成的 .skel.json 文件与原始 .json 文件之间的差异

Usage:
    python analyze_json_differences.py
    python analyze_json_differences.py --data-dir custom_data
    python analyze_json_differences.py --tolerance 0.05 --abs-tolerance 0.001
    python analyze_json_differences.py --output diff_report.txt
"""

import os
import sys
import argparse
import time
from pathlib import Path
from typing import List, Tuple, Dict, Any, Optional
import subprocess
import json


class JsonDifferenceAnalyzer:
    def __init__(self, data_dir: str = "../data", tolerance: float = 0.10, 
                 abs_tolerance: float = 0.01, ignore_defaults: bool = True):
        self.data_dir = Path(data_dir).resolve()
        self.tolerance = tolerance
        self.abs_tolerance = abs_tolerance
        self.ignore_defaults = ignore_defaults
        
        # 确保 json_diff.py 路径正确
        self.json_diff_script = Path(__file__).parent / "json_diff.py"
        
        # 统计信息
        self.stats = {
            'total_pairs': 0,
            'identical_files': 0,
            'different_files': 0,
            'missing_original_files': 0,
            'missing_generated_files': 0,
            'error_files': 0,
            'total_time': 0.0,
            'analysis_results': []
        }
    
    def check_prerequisites(self) -> bool:
        """检查必要的文件和目录是否存在"""
        if not self.data_dir.exists():
            print(f"❌ 错误: data目录不存在: {self.data_dir}")
            return False
        
        if not self.json_diff_script.exists():
            print(f"❌ 错误: json_diff.py不存在: {self.json_diff_script}")
            return False
        
        return True
    
    def find_json_file_pairs(self) -> List[Tuple[Path, Path]]:
        """查找 .skel.json 和对应的 .json 文件对"""
        pairs = []
        
        # 查找所有 .skel.json 文件
        for skel_json_file in self.data_dir.rglob("*.skel.json"):
            if skel_json_file.is_file():
                # 推断对应的原始 .json 文件路径
                # 例如: data/42/cloud-pot/export/cloud-pot.skel.json 
                #   -> data/42/cloud-pot/export/cloud-pot.json
                original_json_file = skel_json_file.with_suffix('').with_suffix('.json')
                
                pairs.append((skel_json_file, original_json_file))
        
        return sorted(pairs)
    
    def analyze_file_pair(self, skel_json_path: Path, original_json_path: Path) -> Dict[str, Any]:
        """分析单个文件对的差异"""
        result = {
            'skel_json_path': skel_json_path,
            'original_json_path': original_json_path,
            'status': 'unknown',
            'differences_count': 0,
            'ignored_defaults_count': 0,
            'analysis_time': 0.0,
            'error_message': '',
            'diff_output': ''
        }
        
        start_time = time.time()
        
        try:
            # 检查文件是否存在
            if not skel_json_path.exists():
                result['status'] = 'missing_generated'
                result['error_message'] = f"生成的文件不存在: {skel_json_path}"
                return result
            
            if not original_json_path.exists():
                result['status'] = 'missing_original'
                result['error_message'] = f"原始文件不存在: {original_json_path}"
                return result
            
            # 调用 json_diff.py 进行比较
            cmd = [
                sys.executable,
                str(self.json_diff_script),
                str(skel_json_path),
                str(original_json_path),
                '--tolerance', str(self.tolerance),
                '--abs-tolerance', str(self.abs_tolerance),
                '--format', 'simple'
            ]
            
            if not self.ignore_defaults:
                cmd.append('--no-ignore-defaults')
            
            process = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=60,
                cwd=self.json_diff_script.parent
            )
            
            result['analysis_time'] = time.time() - start_time
            result['diff_output'] = process.stdout
            
            # 解析输出结果
            if process.returncode == 0:
                # 文件相同或在意义上相同
                result['status'] = 'identical'
                result['differences_count'] = 0
                
                # 尝试提取忽略的默认值数量
                if "已忽略" in process.stdout:
                    import re
                    match = re.search(r'已忽略 (\d+) 个默认值差异', process.stdout)
                    if match:
                        result['ignored_defaults_count'] = int(match.group(1))
            else:
                # 文件有差异
                result['status'] = 'different'
                
                # 尝试提取差异数量
                import re
                match = re.search(r'发现 (\d+) 个差异', process.stdout)
                if match:
                    result['differences_count'] = int(match.group(1))
                
                # 尝试提取忽略的默认值数量
                match = re.search(r'已忽略 (\d+) 个默认值差异', process.stdout)
                if match:
                    result['ignored_defaults_count'] = int(match.group(1))
                
        except subprocess.TimeoutExpired:
            result['status'] = 'error'
            result['error_message'] = "分析超时 (60秒)"
            result['analysis_time'] = time.time() - start_time
        except Exception as e:
            result['status'] = 'error'
            result['error_message'] = f"分析错误: {str(e)}"
            result['analysis_time'] = time.time() - start_time
        
        return result
    
    def analyze_all(self) -> None:
        """分析所有文件对"""
        print("=== Spine JSON 差异分析器 ===")
        print(f"数据目录: {self.data_dir}")
        print(f"数值容差: {self.tolerance*100:.1f}%")
        print(f"绝对容差: {self.abs_tolerance}")
        print(f"忽略默认值: {'是' if self.ignore_defaults else '否'}")
        print("=" * 60)
        
        if not self.check_prerequisites():
            return
        
        file_pairs = self.find_json_file_pairs()
        if not file_pairs:
            print("❌ 未找到任何 .skel.json 文件")
            print("💡 请先运行 convert_skel_to_json.py 生成 JSON 文件")
            return
        
        self.stats['total_pairs'] = len(file_pairs)
        print(f"找到 {len(file_pairs)} 个文件对")
        print("-" * 60)
        
        overall_start = time.time()
        
        for i, (skel_json_path, original_json_path) in enumerate(file_pairs, 1):
            # 获取相对路径用于显示
            rel_path = skel_json_path.relative_to(self.data_dir)
            print(f"[{i:3d}/{len(file_pairs)}] {rel_path}")
            
            result = self.analyze_file_pair(skel_json_path, original_json_path)
            self.stats['analysis_results'].append(result)
            
            # 更新统计
            if result['status'] == 'identical':
                print(f"           ✅ 文件相同 ({result['analysis_time']:.3f}s)")
                if result['ignored_defaults_count'] > 0:
                    print(f"           💡 忽略了 {result['ignored_defaults_count']} 个默认值差异")
                self.stats['identical_files'] += 1
                
            elif result['status'] == 'different':
                print(f"           ⚠️  有差异 ({result['analysis_time']:.3f}s)")
                print(f"           发现 {result['differences_count']} 个差异")
                if result['ignored_defaults_count'] > 0:
                    print(f"           忽略了 {result['ignored_defaults_count']} 个默认值差异")
                self.stats['different_files'] += 1
                
            elif result['status'] == 'missing_original':
                print(f"           ❌ 缺少原始文件")
                self.stats['missing_original_files'] += 1
                
            elif result['status'] == 'missing_generated':
                print(f"           ❌ 缺少生成文件")
                self.stats['missing_generated_files'] += 1
                
            else:  # error
                print(f"           💥 分析错误 ({result['analysis_time']:.3f}s)")
                print(f"           错误: {result['error_message']}")
                self.stats['error_files'] += 1
        
        self.stats['total_time'] = time.time() - overall_start
        self.print_summary()
    
    def print_summary(self) -> None:
        """打印分析统计摘要"""
        print("=" * 60)
        print("=== 差异分析摘要 ===")
        print(f"总文件对数: {self.stats['total_pairs']}")
        print(f"完全相同: {self.stats['identical_files']}")
        print(f"存在差异: {self.stats['different_files']}")
        print(f"缺少原始文件: {self.stats['missing_original_files']}")
        print(f"缺少生成文件: {self.stats['missing_generated_files']}")
        print(f"分析错误: {self.stats['error_files']}")
        
        if self.stats['total_pairs'] > 0:
            identical_rate = self.stats['identical_files'] / self.stats['total_pairs'] * 100
            print(f"相同率: {identical_rate:.1f}%")
        
        print(f"总耗时: {self.stats['total_time']:.3f}s")
        
        # 显示有差异的文件列表
        different_files = [r for r in self.stats['analysis_results'] if r['status'] == 'different']
        if different_files:
            print("\n=== 存在差异的文件 ===")
            for result in different_files:
                rel_path = result['skel_json_path'].relative_to(self.data_dir)
                print(f"⚠️  {rel_path} ({result['differences_count']} 个差异)")
        
        # 显示错误文件列表
        error_files = [r for r in self.stats['analysis_results'] if r['status'] in ['error', 'missing_original', 'missing_generated']]
        if error_files:
            print("\n=== 错误文件列表 ===")
            for result in error_files:
                rel_path = result['skel_json_path'].relative_to(self.data_dir)
                print(f"❌ {rel_path}: {result['error_message']}")
        
        # 最终状态
        if self.stats['different_files'] == 0 and self.stats['error_files'] == 0:
            print("\n🎉 所有文件分析完成，转换质量良好!")
        elif self.stats['identical_files'] > 0:
            print(f"\n⚠️  部分文件存在差异或错误，请检查上述信息")
        else:
            print(f"\n💥 转换质量不佳，请检查转换过程和工具")
    
    def save_detailed_report(self, output_path: str) -> None:
        """保存详细的差异报告"""
        try:
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write("Spine JSON 差异分析详细报告\n")
                f.write("=" * 60 + "\n\n")
                
                f.write(f"分析时间: {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
                f.write(f"数据目录: {self.data_dir}\n")
                f.write(f"数值容差: {self.tolerance*100:.1f}%\n")
                f.write(f"绝对容差: {self.abs_tolerance}\n")
                f.write(f"忽略默认值: {'是' if self.ignore_defaults else '否'}\n\n")
                
                # 统计摘要
                f.write("统计摘要:\n")
                f.write("-" * 30 + "\n")
                f.write(f"总文件对数: {self.stats['total_pairs']}\n")
                f.write(f"完全相同: {self.stats['identical_files']}\n")
                f.write(f"存在差异: {self.stats['different_files']}\n")
                f.write(f"缺少原始文件: {self.stats['missing_original_files']}\n")
                f.write(f"缺少生成文件: {self.stats['missing_generated_files']}\n")
                f.write(f"分析错误: {self.stats['error_files']}\n\n")
                
                # 详细结果
                f.write("详细分析结果:\n")
                f.write("-" * 30 + "\n")
                
                for result in self.stats['analysis_results']:
                    rel_path = result['skel_json_path'].relative_to(self.data_dir)
                    f.write(f"\n文件: {rel_path}\n")
                    f.write(f"状态: {result['status']}\n")
                    f.write(f"分析时间: {result['analysis_time']:.3f}s\n")
                    
                    if result['status'] == 'different':
                        f.write(f"差异数量: {result['differences_count']}\n")
                        f.write(f"忽略默认值: {result['ignored_defaults_count']}\n")
                        f.write("差异详情:\n")
                        f.write(result['diff_output'])
                        f.write("\n")
                    elif result['status'] == 'identical':
                        if result['ignored_defaults_count'] > 0:
                            f.write(f"忽略默认值: {result['ignored_defaults_count']}\n")
                    elif result['status'] in ['error', 'missing_original', 'missing_generated']:
                        f.write(f"错误信息: {result['error_message']}\n")
                    
                    f.write("-" * 50 + "\n")
            
            print(f"\n📄 详细报告已保存到: {output_path}")
        except Exception as e:
            print(f"\n❌ 保存报告失败: {e}")


def main():
    parser = argparse.ArgumentParser(description='分析生成的 .skel.json 文件与原始 .json 文件之间的差异')
    parser.add_argument('--data-dir', default='../data',
                        help='包含 JSON 文件的数据目录 (默认: ../data)')
    parser.add_argument('--tolerance', '-t', type=float, default=0.10,
                        help='数值比较相对容差 (默认: 0.10，即10%%)')
    parser.add_argument('--abs-tolerance', '-a', type=float, default=0.01,
                        help='数值比较绝对容差 (默认: 0.01)')
    parser.add_argument('--no-ignore-defaults', action='store_true',
                        help='不忽略默认值差异')
    parser.add_argument('--output', '-o',
                        help='保存详细报告到文件')
    
    args = parser.parse_args()
    
    try:
        analyzer = JsonDifferenceAnalyzer(
            data_dir=args.data_dir,
            tolerance=args.tolerance,
            abs_tolerance=args.abs_tolerance,
            ignore_defaults=not args.no_ignore_defaults
        )
        
        analyzer.analyze_all()
        
        # 保存详细报告
        if args.output:
            analyzer.save_detailed_report(args.output)
        
        # 设置退出码
        if analyzer.stats['different_files'] > 0 or analyzer.stats['error_files'] > 0:
            sys.exit(1)
        else:
            sys.exit(0)
            
    except KeyboardInterrupt:
        print("\n\n⚠️  用户中断分析过程")
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ 分析过程中发生错误: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
