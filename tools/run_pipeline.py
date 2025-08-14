#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Spine Data Processing Pipeline
Spine 数据处理管道 - 一键执行完整的转换、验证和差异分析流程

Usage:
    python run_pipeline.py
    python run_pipeline.py --data-dir custom_data --target-dir custom_target
    python run_pipeline.py --skip-convert --skip-validate  # 只执行差异分析
"""

import os
import sys
import argparse
import subprocess
import time
from typing import List, Optional
from pathlib import Path


class SpinePipeline:
    def __init__(self, data_dir: str = "../data", target_dir: str = "../target"):
        self.data_dir = data_dir
        self.target_dir = target_dir
        self.tools_dir = Path(__file__).parent
        
        # 脚本路径
        self.convert_script = self.tools_dir / "convert_skel_to_json.py"
        self.validate_script = self.tools_dir / "validate_json_files.py"
        self.analyze_script = self.tools_dir / "analyze_json_differences.py"
    
    def run_step(self, script_path: Path, description: str, args: Optional[List[str]] = None) -> bool:
        """运行单个步骤"""
        print(f"\n🚀 开始 {description}...")
        print("=" * 60)
        
        cmd = [sys.executable, str(script_path)]
        if args:
            cmd.extend(args)
        
        try:
            result = subprocess.run(cmd, cwd=self.tools_dir)
            if result.returncode == 0:
                print(f"\n✅ {description} 完成")
                return True
            else:
                print(f"\n❌ {description} 失败 (退出码: {result.returncode})")
                return False
        except Exception as e:
            print(f"\n💥 {description} 执行错误: {e}")
            return False
    
    def run_pipeline(self, skip_convert: bool = False, skip_validate: bool = False, 
                    skip_analyze: bool = False, output_report: Optional[str] = None) -> None:
        """运行完整的处理管道"""
        print("🎯 Spine 数据处理管道")
        print(f"📁 数据目录: {self.data_dir}")
        print(f"🔧 工具目录: {self.target_dir}")
        print("=" * 60)
        
        start_time = time.time()
        steps_completed = 0
        total_steps = sum([not skip_convert, not skip_validate, not skip_analyze])
        
        # 准备通用参数
        common_args = [
            '--data-dir', self.data_dir,
            '--target-dir', self.target_dir
        ]
        
        # 步骤1: 转换 .skel 文件为 .json
        if not skip_convert:
            success = self.run_step(
                self.convert_script,
                "步骤1: 转换 .skel 文件为 .json",
                common_args
            )
            if success:
                steps_completed += 1
            else:
                print(f"\n💥 管道在步骤1失败，停止执行")
                return
        
        # 步骤2: 验证生成的 JSON 文件
        if not skip_validate:
            success = self.run_step(
                self.validate_script,
                "步骤2: 验证生成的 JSON 文件",
                common_args
            )
            if success:
                steps_completed += 1
            else:
                print(f"\n⚠️  步骤2失败，但继续执行后续步骤...")
        
        # 步骤3: 分析 JSON 文件差异
        if not skip_analyze:
            analyze_args = common_args.copy()
            if output_report:
                analyze_args.extend(['--output', output_report])
            
            success = self.run_step(
                self.analyze_script,
                "步骤3: 分析 JSON 文件差异",
                analyze_args
            )
            if success:
                steps_completed += 1
        
        # 最终总结
        total_time = time.time() - start_time
        print("\n" + "=" * 60)
        print("🏁 管道执行完成")
        print(f"⏱️  总耗时: {total_time:.2f}s")
        print(f"✅ 完成步骤: {steps_completed}/{total_steps}")
        
        if steps_completed == total_steps:
            print("🎉 所有步骤执行成功!")
        else:
            print("⚠️  部分步骤失败，请检查上述输出")


def main():
    parser = argparse.ArgumentParser(description='运行完整的 Spine 数据处理管道')
    parser.add_argument('--data-dir', default='../data',
                        help='包含 .skel 文件的数据目录 (默认: ../data)')
    parser.add_argument('--target-dir', default='../target',
                        help='包含可执行文件的目录 (默认: ../target)')
    parser.add_argument('--skip-convert', action='store_true',
                        help='跳过转换步骤')
    parser.add_argument('--skip-validate', action='store_true',
                        help='跳过验证步骤')
    parser.add_argument('--skip-analyze', action='store_true',
                        help='跳过差异分析步骤')
    parser.add_argument('--output-report',
                        help='差异分析详细报告输出文件')
    
    args = parser.parse_args()
    
    try:
        pipeline = SpinePipeline(args.data_dir, args.target_dir)
        pipeline.run_pipeline(
            skip_convert=args.skip_convert,
            skip_validate=args.skip_validate,
            skip_analyze=args.skip_analyze,
            output_report=args.output_report
        )
    except KeyboardInterrupt:
        print("\n\n⚠️  用户中断管道执行")
        sys.exit(1)
    except Exception as e:
        print(f"\n❌ 管道执行错误: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
