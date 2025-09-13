#!/usr/bin/env python3
"""
JSON Diff Tool - 比较两个JSON文件的差异，支持数值容差比较

使用方法:
    python json_diff.py file1.json file2.json [--tolerance 0.05] [--verbose]

参数:
    file1.json          第一个JSON文件
    file2.json          第二个JSON文件
    --tolerance, -t     数值比较容差，默认5% (0.05)
    --verbose, -v       显示详细差异信息
    --help, -h          显示帮助信息
"""

import json
import sys
import argparse
import os
from typing import Any, Dict, List, Tuple, Union

class JsonDiffTool:
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
            print(f"错误: 文件 '{file_path}' 不存在")
            sys.exit(1)
        except json.JSONDecodeError as e:
            print(f"错误: 文件 '{file_path}' 不是有效的JSON格式: {e}")
            sys.exit(1)
        except Exception as e:
            print(f"错误: 无法读取文件 '{file_path}': {e}")
            sys.exit(1)
    
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
    
    def compare_json_files(self, file1: str, file2: str) -> bool:
        """
        比较两个JSON文件
        
        Args:
            file1: 第一个JSON文件路径
            file2: 第二个JSON文件路径
            
        Returns:
            True如果文件相同，否则False
        """
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
        if are_equal:
            print("✅ 文件相同 (在指定容差范围内)")
            return True
        else:
            print(f"❌ 发现 {len(self.differences)} 处差异:")
            self.print_differences()
            return False
    
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


def main():
    parser = argparse.ArgumentParser(
        description="比较两个JSON文件的差异，支持数值容差比较",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
    python json_diff.py file1.json file2.json
    python json_diff.py file1.json file2.json --tolerance 0.1
    python json_diff.py file1.json file2.json -t 0.01 -v
        """
    )
    
    parser.add_argument('file1', help='第一个JSON文件')
    parser.add_argument('file2', help='第二个JSON文件')
    parser.add_argument('-t', '--tolerance', type=float, default=0.05,
                        help='数值比较容差 (默认: 0.05 = 5%%)')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='显示详细信息')
    
    args = parser.parse_args()
    
    # 验证文件存在
    if not os.path.exists(args.file1):
        print(f"错误: 文件 '{args.file1}' 不存在")
        sys.exit(1)
    
    if not os.path.exists(args.file2):
        print(f"错误: 文件 '{args.file2}' 不存在")
        sys.exit(1)
    
    # 验证容差值
    if args.tolerance < 0:
        print("错误: 容差值不能为负数")
        sys.exit(1)
    
    # 创建比较工具并执行比较
    diff_tool = JsonDiffTool(tolerance=args.tolerance, verbose=args.verbose)
    
    try:
        are_equal = diff_tool.compare_json_files(args.file1, args.file2)
        sys.exit(0 if are_equal else 1)
    except KeyboardInterrupt:
        print("\n\n操作被用户中断")
        sys.exit(130)
    except Exception as e:
        print(f"未预期的错误: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
