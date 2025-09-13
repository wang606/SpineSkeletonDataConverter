#!/usr/bin/env python3
"""
批量JSON文件差异比较工具 - 递归比较目录下符合条件的成对文件

使用方法:
    python json_diff_all.py <directory> <suffix1> <suffix2> [--tolerance 0.05] [--verbose] [--summary-only]

参数:
    directory           要搜索的根目录
    suffix1            第一个文件后缀 (如 .json)
    suffix2            第二个文件后缀 (如 .json.skel.json)
    --tolerance, -t     数值比较容差，默认5% (0.05)
    --verbose, -v       显示详细差异信息
    --summary-only, -s  仅显示汇总信息，不显示详细差异
    --help, -h          显示帮助信息

示例:
    python json_diff_all.py ./data .json .json.skel.json
    python json_diff_all.py ./data .json .json.skel.json --tolerance 0.1 --verbose
    python json_diff_all.py ./data .json .json.skel.json -s
"""

import os
import sys
import argparse
import json
from pathlib import Path
from typing import List, Tuple, Dict, Any, Union


class JsonDiffTool:
    """JSON文件差异比较工具"""
    
    def __init__(self, tolerance: float = 0.05, verbose: bool = False):
        """
        初始化JSON差异比较工具
        
        Args:
            tolerance: 数值比较容差 (默认5%)
            verbose: 是否显示详细信息
        """
        self.tolerance = tolerance
        self.verbose = verbose
        self.differences = []
        
    def load_json_file(self, file_path: str) -> Dict[str, Any]:
        """加载JSON文件"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        except FileNotFoundError:
            raise FileNotFoundError(f"文件 '{file_path}' 不存在")
        except json.JSONDecodeError as e:
            raise ValueError(f"文件 '{file_path}' 不是有效的JSON格式: {e}")
        except Exception as e:
            raise RuntimeError(f"无法读取文件 '{file_path}': {e}")
    
    def is_number(self, value: Any) -> bool:
        """检查值是否为数字"""
        return isinstance(value, (int, float))
    
    def numbers_equal_with_tolerance(self, val1: Union[int, float], val2: Union[int, float]) -> bool:
        """
        比较两个数字是否在容差范围内相等
        
        Args:
            val1, val2: 要比较的两个数值
            
        Returns:
            True如果在容差范围内相等，否则False
        """
        # 转换为float以支持int/float混合比较
        float_val1 = float(val1)
        float_val2 = float(val2)
        
        if float_val1 == float_val2:
            return True
        
        # 当两个值都很接近0时（绝对值都小于0.001），直接判为相同
        if abs(float_val1) < 0.001 and abs(float_val2) < 0.001:
            return True
        
        # 如果其中一个为0，使用绝对差值比较
        if float_val1 == 0 or float_val2 == 0:
            return abs(float_val1 - float_val2) <= self.tolerance
        
        # 使用相对误差比较
        relative_error = abs(float_val1 - float_val2) / max(abs(float_val1), abs(float_val2))
        return relative_error <= self.tolerance
    
    def format_path(self, path: List[str]) -> str:
        """格式化路径字符串"""
        if not path:
            return "根"
        
        formatted_path = ""
        for i, key in enumerate(path):
            if key.isdigit():
                formatted_path += f"[{key}]"
            else:
                if i > 0:
                    formatted_path += "."
                formatted_path += key
        
        return formatted_path
    
    def compare_values(self, val1: Any, val2: Any, path: List[str]) -> bool:
        """
        比较两个值是否相等（考虑数值容差）
        
        Args:
            val1, val2: 要比较的值
            path: 当前路径
            
        Returns:
            True如果相等，否则False
        """
        # 数字比较（支持int/float混合比较）
        if self.is_number(val1) and self.is_number(val2):
            if not self.numbers_equal_with_tolerance(val1, val2):
                float_val1 = float(val1)
                float_val2 = float(val2)
                relative_diff = abs(float_val1 - float_val2) / max(abs(float_val1), abs(float_val2)) if max(abs(float_val1), abs(float_val2)) != 0 else abs(float_val1 - float_val2)
                self.differences.append({
                    'path': self.format_path(path),
                    'type': 'value_mismatch',
                    'value1': val1,
                    'value2': val2,
                    'difference': abs(float_val1 - float_val2),
                    'relative_difference': relative_diff,
                    'tolerance': self.tolerance
                })
                return False
            return True
        
        # 类型不同（但排除int/float混合的情况）
        if type(val1) != type(val2):
            # 如果一个是int一个是float，前面已经处理了
            if not (self.is_number(val1) and self.is_number(val2)):
                self.differences.append({
                    'path': self.format_path(path),
                    'type': 'type_mismatch',
                    'value1': val1,
                    'value2': val2,
                    'value1_type': type(val1).__name__,
                    'value2_type': type(val2).__name__
                })
                return False
        
        # 字符串、布尔值、None比较
        if val1 != val2:
            self.differences.append({
                'path': self.format_path(path),
                'type': 'value_mismatch',
                'value1': val1,
                'value2': val2
            })
            return False
        
        return True
    
    def compare_dicts(self, dict1: Dict[str, Any], dict2: Dict[str, Any], path: List[str]) -> bool:
        """比较两个字典"""
        all_equal = True
        
        # 检查键的差异
        keys1 = set(dict1.keys())
        keys2 = set(dict2.keys())
        
        # 只在dict1中的键
        only_in_dict1 = keys1 - keys2
        for key in only_in_dict1:
            self.differences.append({
                'path': self.format_path(path + [key]),
                'type': 'missing_in_second',
                'value1': dict1[key],
                'value2': None
            })
            all_equal = False
        
        # 只在dict2中的键
        only_in_dict2 = keys2 - keys1
        for key in only_in_dict2:
            self.differences.append({
                'path': self.format_path(path + [key]),
                'type': 'missing_in_first',
                'value1': None,
                'value2': dict2[key]
            })
            all_equal = False
        
        # 比较共同的键
        common_keys = keys1 & keys2
        for key in sorted(common_keys):
            if not self.compare_recursive(dict1[key], dict2[key], path + [key]):
                all_equal = False
        
        return all_equal
    
    def compare_lists(self, list1: List[Any], list2: List[Any], path: List[str]) -> bool:
        """比较两个列表"""
        all_equal = True
        
        # 长度不同
        if len(list1) != len(list2):
            self.differences.append({
                'path': self.format_path(path),
                'type': 'length_mismatch',
                'value1_length': len(list1),
                'value2_length': len(list2)
            })
            all_equal = False
        
        # 比较相同索引的元素
        min_length = min(len(list1), len(list2))
        for i in range(min_length):
            if not self.compare_recursive(list1[i], list2[i], path + [str(i)]):
                all_equal = False
        
        # 处理长度不同的情况
        if len(list1) > len(list2):
            for i in range(len(list2), len(list1)):
                self.differences.append({
                    'path': self.format_path(path + [str(i)]),
                    'type': 'missing_in_second',
                    'value1': list1[i],
                    'value2': None
                })
                all_equal = False
        elif len(list2) > len(list1):
            for i in range(len(list1), len(list2)):
                self.differences.append({
                    'path': self.format_path(path + [str(i)]),
                    'type': 'missing_in_first',
                    'value1': None,
                    'value2': list2[i]
                })
                all_equal = False
        
        return all_equal
    
    def compare_recursive(self, obj1: Any, obj2: Any, path: List[str]) -> bool:
        """递归比较两个对象"""
        if isinstance(obj1, dict) and isinstance(obj2, dict):
            return self.compare_dicts(obj1, obj2, path)
        elif isinstance(obj1, list) and isinstance(obj2, list):
            return self.compare_lists(obj1, obj2, path)
        else:
            return self.compare_values(obj1, obj2, path)
    
    def compare_json_files(self, file1: str, file2: str, silent: bool = False) -> bool:
        """
        比较两个JSON文件
        
        Args:
            file1: 第一个JSON文件路径
            file2: 第二个JSON文件路径
            silent: 是否静默模式（不打印过程信息）
            
        Returns:
            True如果文件相同，否则False
        """
        if not silent:
            print(f"正在比较文件:")
            print(f"  文件1: {file1}")
            print(f"  文件2: {file2}")
            print(f"  数值容差: {self.tolerance * 100:.1f}%")
            print("-" * 60)
        
        # 加载JSON文件
        json1 = self.load_json_file(file1)
        json2 = self.load_json_file(file2)
        
        # 清空之前的差异记录
        self.differences = []
        
        # 比较JSON
        are_equal = self.compare_recursive(json1, json2, [])
        
        # 输出结果
        if not silent:
            if are_equal:
                print("✅ 文件相同 (在指定容差范围内)")
            else:
                print(f"❌ 发现 {len(self.differences)} 处差异")
                if self.verbose:
                    self.print_differences()
        
        return are_equal
    
    def print_differences(self):
        """打印差异信息"""
        for i, diff in enumerate(self.differences, 1):
            print(f"\n{i}. 路径: {diff['path']}")
            
            if diff['type'] == 'type_mismatch':
                print(f"   类型不匹配:")
                print(f"   文件1: {diff['value1']} ({diff['value1_type']})")
                print(f"   文件2: {diff['value2']} ({diff['value2_type']})")
                
            elif diff['type'] == 'value_mismatch':
                if 'relative_difference' in diff:
                    # 数值差异
                    print(f"   数值差异 (超出容差 {diff['tolerance']*100:.1f}%):")
                    print(f"   文件1: {diff['value1']}")
                    print(f"   文件2: {diff['value2']}")
                    print(f"   绝对差异: {diff['difference']:.6f}")
                    print(f"   相对差异: {diff['relative_difference']*100:.2f}%")
                else:
                    # 其他值差异
                    print(f"   值不同:")
                    print(f"   文件1: {diff['value1']}")
                    print(f"   文件2: {diff['value2']}")
                    
            elif diff['type'] == 'missing_in_second':
                print(f"   仅在文件1中存在:")
                print(f"   值: {diff['value1']}")
                
            elif diff['type'] == 'missing_in_first':
                print(f"   仅在文件2中存在:")
                print(f"   值: {diff['value2']}")
                
            elif diff['type'] == 'length_mismatch':
                print(f"   数组长度不同:")
                print(f"   文件1长度: {diff['value1_length']}")
                print(f"   文件2长度: {diff['value2_length']}")

class JsonDiffAllTool:
    def __init__(self, tolerance: float = 0.05, verbose: bool = False, summary_only: bool = False):
        """
        初始化批量JSON差异比较工具
        
        Args:
            tolerance: 数值比较容差 (默认5%)
            verbose: 是否显示详细信息
            summary_only: 是否仅显示汇总信息
        """
        self.tolerance = tolerance
        self.verbose = verbose
        self.summary_only = summary_only
        
        # 创建JsonDiffTool实例
        self.diff_tool = JsonDiffTool(tolerance=tolerance, verbose=verbose)
        
        # 统计信息
        self.total_pairs = 0
        self.equal_pairs = 0
        self.different_pairs = 0
        self.error_pairs = 0
        self.results = []  # 存储所有比较结果
    
    def find_file_pairs(self, directory: str, suffix1: str, suffix2: str) -> List[Tuple[str, str]]:
        """
        在指定目录中递归查找符合条件的文件对
        
        Args:
            directory: 搜索目录
            suffix1: 第一个文件后缀
            suffix2: 第二个文件后缀
            
        Returns:
            文件对列表，每个元素是(file1_path, file2_path)的元组
        """
        file_pairs = []
        directory_path = Path(directory)
        
        if not directory_path.exists():
            print(f"错误: 目录 '{directory}' 不存在")
            return file_pairs
        
        if not directory_path.is_dir():
            print(f"错误: '{directory}' 不是一个目录")
            return file_pairs
        
        print(f"正在搜索目录: {directory}")
        print(f"查找文件对: *{suffix1} 和 *{suffix2}")
        print(f"注意: {suffix1} 文件必须是严格以该后缀结尾，不包含其他 .*.json 文件")
        print("-" * 60)
        
        # 递归遍历目录
        for root, dirs, files in os.walk(directory):
            root_path = Path(root)
            
            # 创建文件名到完整路径的映射
            files_in_dir = {}
            for file in files:
                files_in_dir[file] = root_path / file
            
            # 查找符合条件的文件对
            for filename in files:
                # 检查是否以suffix1结尾
                if filename.endswith(suffix1):
                    # 确保这是严格的suffix1结尾，不是其他形式的.json文件
                    # 例如：如果suffix1是.json，则不匹配.skel.json或.atlas.json等
                    base_name = filename[:-len(suffix1)]
                    
                    # 检查是否存在其他形式的.json文件（如果suffix1是.json）
                    if suffix1 == '.json':
                        # 检查该文件是否真的只是.json结尾，而不是.something.json
                        potential_other_suffixes = ['.skel.json', '.atlas.json', '.png.json']
                        is_pure_json = True
                        for other_suffix in potential_other_suffixes:
                            if filename.endswith(other_suffix):
                                is_pure_json = False
                                break
                        
                        if not is_pure_json:
                            continue
                    
                    # 构造对应的suffix2文件名
                    corresponding_file = base_name + suffix2
                    
                    # 检查对应文件是否存在
                    if corresponding_file in files_in_dir:
                        file1_path = files_in_dir[filename]
                        file2_path = files_in_dir[corresponding_file]
                        file_pairs.append((str(file1_path), str(file2_path)))
        
        print(f"找到 {len(file_pairs)} 个文件对")
        return file_pairs
    
    def compare_file_pair(self, file1: str, file2: str) -> Dict[str, Any]:
        """
        比较一对文件
        
        Args:
            file1: 第一个文件路径
            file2: 第二个文件路径
            
        Returns:
            比较结果字典
        """
        result = {
            'file1': file1,
            'file2': file2,
            'equal': False,
            'error': None,
            'differences_count': 0,
            'differences': []
        }
        
        try:
            # 使用json_diff工具进行比较
            # 如果是summary_only模式，使用静默模式
            silent_mode = self.summary_only
            are_equal = self.diff_tool.compare_json_files(file1, file2, silent=silent_mode)
            
            result['equal'] = are_equal
            result['differences_count'] = len(self.diff_tool.differences)
            result['differences'] = self.diff_tool.differences.copy()
            
        except Exception as e:
            result['error'] = str(e)
            if not self.summary_only:
                print(f"❌ 比较文件时出错: {e}")
        
        return result
    
    def compare_all_pairs(self, directory: str, suffix1: str, suffix2: str) -> bool:
        """
        比较所有找到的文件对
        
        Args:
            directory: 搜索目录
            suffix1: 第一个文件后缀
            suffix2: 第二个文件后缀
            
        Returns:
            True如果所有文件对都相同，否则False
        """
        # 查找文件对
        file_pairs = self.find_file_pairs(directory, suffix1, suffix2)
        
        if not file_pairs:
            print("未找到符合条件的文件对")
            return True
        
        # 重置统计
        self.total_pairs = len(file_pairs)
        self.equal_pairs = 0
        self.different_pairs = 0
        self.error_pairs = 0
        self.results = []
        
        print(f"\n开始比较 {self.total_pairs} 个文件对...")
        print("=" * 60)
        
        # 比较每一对文件
        for i, (file1, file2) in enumerate(file_pairs, 1):
            if self.summary_only:
                # 在summary模式下显示进度
                print(f"进度: {i}/{self.total_pairs} - {os.path.basename(file1)}", end='\r')
            
            result = self.compare_file_pair(file1, file2)
            self.results.append(result)
            
            # 更新统计
            if result['error']:
                self.error_pairs += 1
            elif result['equal']:
                self.equal_pairs += 1
            else:
                self.different_pairs += 1
            
            if not self.summary_only:
                print("-" * 60)
        
        if self.summary_only:
            print()  # 清除进度显示行
        
        # 显示汇总结果
        self.print_summary()
        
        # 如果有差异，显示详细信息
        if not self.summary_only and self.different_pairs > 0:
            self.print_detailed_results()
        
        return self.different_pairs == 0 and self.error_pairs == 0
    
    def print_summary(self):
        """打印汇总信息"""
        print("\n" + "=" * 60)
        print("汇总结果:")
        print(f"总文件对数: {self.total_pairs}")
        print(f"✅ 相同文件对: {self.equal_pairs}")
        print(f"❌ 不同文件对: {self.different_pairs}")
        print(f"⚠️  错误文件对: {self.error_pairs}")
        
        if self.total_pairs > 0:
            success_rate = (self.equal_pairs / self.total_pairs) * 100
            print(f"成功率: {success_rate:.1f}%")
        
        print("=" * 60)
    
    def print_detailed_results(self):
        """打印详细结果"""
        print("\n详细结果:")
        print("-" * 60)
        
        # 显示有差异的文件对
        if self.different_pairs > 0:
            print(f"\n❌ 有差异的文件对 ({self.different_pairs}):")
            for i, result in enumerate(self.results, 1):
                if not result['equal'] and not result['error']:
                    rel_path1 = os.path.relpath(result['file1'])
                    rel_path2 = os.path.relpath(result['file2'])
                    print(f"  {i}. {rel_path1} <-> {rel_path2}")
                    print(f"     差异数量: {result['differences_count']}")
        
        # 显示有错误的文件对
        if self.error_pairs > 0:
            print(f"\n⚠️  有错误的文件对 ({self.error_pairs}):")
            for i, result in enumerate(self.results, 1):
                if result['error']:
                    rel_path1 = os.path.relpath(result['file1'])
                    rel_path2 = os.path.relpath(result['file2'])
                    print(f"  {i}. {rel_path1} <-> {rel_path2}")
                    print(f"     错误: {result['error']}")


def main():
    parser = argparse.ArgumentParser(
        description="递归比较目录下符合条件的成对JSON文件",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
    python json_diff_all.py ./data .json .json.skel.json
    python json_diff_all.py ./data .json .json.skel.json --tolerance 0.1
    python json_diff_all.py ./data .json .json.skel.json -t 0.01 -v
    python json_diff_all.py ./data .json .json.skel.json -s
        """
    )
    
    parser.add_argument('directory', help='要搜索的根目录')
    parser.add_argument('suffix1', help='第一个文件后缀 (如 .json)')
    parser.add_argument('suffix2', help='第二个文件后缀 (如 .json.skel.json)')
    parser.add_argument('-t', '--tolerance', type=float, default=0.05,
                        help='数值比较容差 (默认: 0.05 = 5%%)')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='显示详细差异信息')
    parser.add_argument('-s', '--summary-only', action='store_true',
                        help='仅显示汇总信息，不显示详细差异')
    
    args = parser.parse_args()
    
    # 验证目录存在
    if not os.path.exists(args.directory):
        print(f"错误: 目录 '{args.directory}' 不存在")
        sys.exit(1)
    
    if not os.path.isdir(args.directory):
        print(f"错误: '{args.directory}' 不是一个目录")
        sys.exit(1)
    
    # 验证容差值
    if args.tolerance < 0:
        print("错误: 容差值不能为负数")
        sys.exit(1)
    
    # 验证后缀格式
    if not args.suffix1.startswith('.') or not args.suffix2.startswith('.'):
        print("错误: 文件后缀必须以'.'开头")
        sys.exit(1)
    
    # 创建比较工具并执行比较
    diff_tool = JsonDiffAllTool(
        tolerance=args.tolerance, 
        verbose=args.verbose,
        summary_only=args.summary_only
    )
    
    try:
        print(f"批量JSON文件差异比较工具")
        print(f"搜索目录: {os.path.abspath(args.directory)}")
        print(f"文件后缀: {args.suffix1} <-> {args.suffix2}")
        print(f"数值容差: {args.tolerance * 100:.1f}%")
        if args.summary_only:
            print("模式: 仅显示汇总")
        elif args.verbose:
            print("模式: 详细信息")
        else:
            print("模式: 标准")
        print("=" * 60)
        
        all_equal = diff_tool.compare_all_pairs(args.directory, args.suffix1, args.suffix2)
        sys.exit(0 if all_equal else 1)
        
    except KeyboardInterrupt:
        print("\n\n操作被用户中断")
        sys.exit(130)
    except Exception as e:
        print(f"未预期的错误: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
