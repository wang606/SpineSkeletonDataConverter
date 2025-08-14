#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
JSON Difference Comparison Tool
比较两个JSON文件之间的差异，支持数值容差比较和Spine默认值忽略

Usage:
    python json_diff.py file1.json file2.json
    python json_diff.py file1.json file2.json --output diff_report.txt
    python json_diff.py file1.json file2.json --format detailed
    python json_diff.py file1.json file2.json --tolerance 0.1 --abs-tolerance 1e-6
    python json_diff.py file1.json file2.json --no-ignore-defaults  # 不忽略默认值差异
    python json_diff.py file1.json file2.json -t 0.05 -a 1e-9 -f detailed
"""

import json
import sys
import argparse
import re
from typing import Any, Dict, List, Union, Tuple
from pathlib import Path


class SpineDefaultValues:
    """Spine JSON默认值数据库 - 基于键名的智能匹配"""
    
    # 数值类型的默认值键名（值接近0时视为默认）
    ZERO_VALUE_KEYS = {
        'x', 'y', 'rotation', 'shearX', 'shearY', 'length', 'time', 'value',
        'float', 'int', 'offset', 'position', 'spacing', 'rotate', 'wind', 'gravity',
        'order', 'bendPositive', 'local', 'relative', 'limit', 'inertia', 'strength',
        'damping', 'mass', 'angle'
    }
    
    # 数值类型的默认值键名（值接近1时视为默认）
    ONE_VALUE_KEYS = {
        'scaleX', 'scaleY', 'mix', 'mixRotate', 'mixX', 'mixY', 'mixScaleX', 'mixScaleY', 
        'mixShearY', 'fps', 'referenceScale'
    }
    
    # 字符串类型的默认值键名（空字符串视为默认）
    EMPTY_STRING_KEYS = {
        'string', 'name', 'path', 'attachmentName', 'attachment', 'images', 'audio',
        'icon', 'dark'
    }
    
    # 布尔类型的默认值键名（false视为默认）
    FALSE_BOOLEAN_KEYS = {
        'skin', 'compress', 'stretch', 'uniform', 'inertiaGlobal', 'strengthGlobal',
        'dampingGlobal', 'massGlobal', 'windGlobal', 'gravityGlobal', 'mixGlobal'
    }
    
    # 布尔类型的默认值键名（true视为默认）
    TRUE_BOOLEAN_KEYS = {
        'visible'
    }
    
    # 颜色类型的默认值键名和对应的默认值
    COLOR_DEFAULTS = {
        'color': ['ffffffff', 'ffffff', 'FFFFFFFF', 'FFFFFF', '9b9b9bff', '9b9b9bff'],  # 白色和骨骼默认色
        'dark': ['', '000000', '000000ff', '000000FF']  # 黑色/空
    }
    
    # 枚举类型的默认值键名和对应的默认值
    ENUM_DEFAULTS = {
        'blend': ['normal'],
        'inherit': ['normal'],
        'type': ['region'],
        'positionMode': ['fixed'],
        'spacingMode': ['length'],
        'rotateMode': ['tangent']
    }
    
    # 特殊的固定默认值
    SPECIAL_DEFAULTS = {
        'fps': [30.0, 60.0],  # 常见的FPS值
        'referenceScale': [100.0],
        'color': ['9b9b9bff']  # 骨骼的默认颜色
    }

    @classmethod
    def is_default_value(cls, path: str, value: Any, numeric_tolerance: float = 0.10, absolute_tolerance: float = 0.01) -> bool:
        """检查给定路径的值是否是默认值（基于键名智能匹配）"""
        # 从路径中提取键名
        key_name = cls._extract_key_name(path)
        
        # 特殊处理：皮肤附件的name和path
        if cls._is_attachment_name_or_path_default(path, value):
            return True
        
        # 特殊处理：hash键总是忽略
        if key_name == 'hash':
            return True
        
        # 布尔类型检查（必须放在数值检查之前，因为bool是int的子类）
        if isinstance(value, bool):
            return cls._is_boolean_default(key_name, value)
        
        # 数值类型检查
        if isinstance(value, (int, float)):
            return cls._is_numeric_default(key_name, value, numeric_tolerance, absolute_tolerance)
        
        # 字符串类型检查
        if isinstance(value, str):
            return cls._is_string_default(key_name, value)
        
        return False
    
    @classmethod
    def _extract_key_name(cls, path: str) -> str:
        """从路径中提取键名"""
        # 移除数组索引，如 scale[0] -> scale
        import re
        path_clean = re.sub(r'\[\d+\]', '', path)
        
        # 获取最后一个部分作为键名
        if '.' in path_clean:
            return path_clean.split('.')[-1]
        return path_clean
    
    @classmethod
    def _is_numeric_default(cls, key_name: str, value: float, tolerance: float, abs_tolerance: float) -> bool:
        """检查数值是否为默认值"""
        # 检查接近0的默认值
        if key_name in cls.ZERO_VALUE_KEYS:
            return cls._is_numeric_close_to(value, 0.0, tolerance, abs_tolerance)
        
        # 检查接近1的默认值
        if key_name in cls.ONE_VALUE_KEYS:
            return cls._is_numeric_close_to(value, 1.0, tolerance, abs_tolerance)
        
        # 检查特殊的固定默认值
        if key_name in cls.SPECIAL_DEFAULTS:
            for default_val in cls.SPECIAL_DEFAULTS[key_name]:
                if cls._is_numeric_close_to(value, default_val, tolerance, abs_tolerance):
                    return True
        
        return False
    
    @classmethod
    def _is_string_default(cls, key_name: str, value: str) -> bool:
        """检查字符串是否为默认值"""
        # 检查空字符串默认值
        if key_name in cls.EMPTY_STRING_KEYS and value == '':
            return True
        
        # 检查颜色默认值
        if key_name in cls.COLOR_DEFAULTS:
            return value in cls.COLOR_DEFAULTS[key_name]
        
        # 检查枚举默认值
        if key_name in cls.ENUM_DEFAULTS:
            return value in cls.ENUM_DEFAULTS[key_name]
        
        return False
    
    @classmethod
    def _is_boolean_default(cls, key_name: str, value: bool) -> bool:
        """检查布尔值是否为默认值"""
        if key_name in cls.FALSE_BOOLEAN_KEYS and value is False:
            return True
        
        if key_name in cls.TRUE_BOOLEAN_KEYS and value is True:
            return True
        
        return False
    
    @classmethod
    def _is_numeric_close_to(cls, val1: float, val2: float, tolerance: float, abs_tolerance: float) -> bool:
        """检查两个数值是否接近"""
        if val1 == val2:
            return True
        
        diff = abs(val1 - val2)
        
        # 绝对容差检查
        if diff <= abs_tolerance:
            return True
            
        # 相对容差检查
        if val2 != 0:
            relative_diff = diff / abs(val2)
            if relative_diff <= tolerance:
                return True
        
        return False
    
    @classmethod
    def _is_attachment_name_or_path_default(cls, path: str, value: Any) -> bool:
        """检查附件的name或path是否等于附件名（这种情况下视为默认值）"""
        # 只对字符串类型进行检查
        if not isinstance(value, str):
            return False
            
        import re
        
        # 匹配 skins[0].attachments.slotName.attachmentName.name
        name_pattern = r"^skins\[\d+\]\.attachments\.([^.]+)\.([^.]+)\.name$"
        name_match = re.match(name_pattern, path)
        if name_match:
            attachment_name = name_match.group(2)
            return value == attachment_name
            
        # 匹配 skins[0].attachments.slotName.attachmentName.path  
        path_pattern = r"^skins\[\d+\]\.attachments\.([^.]+)\.([^.]+)\.path$"
        path_match = re.match(path_pattern, path)
        if path_match:
            attachment_name = path_match.group(2)
            return value == attachment_name
            
        return False


class JSONDiffer:
    """JSON文件差异比较器"""
    
    def __init__(self, numeric_tolerance: float = 0.10, absolute_tolerance: float = 0.01, ignore_defaults: bool = True):
        """
        初始化比较器
        
        Args:
            numeric_tolerance: 相对容差（默认10%）
            absolute_tolerance: 绝对容差（用于接近零的数值，默认0.01）
            ignore_defaults: 是否忽略默认值差异
        """
        self.differences = []
        self.numeric_tolerance = numeric_tolerance
        self.absolute_tolerance = absolute_tolerance
        self.ignore_defaults = ignore_defaults
        self.ignored_defaults_count = 0
    
    def _is_numeric_equal(self, val1: Any, val2: Any) -> bool:
        """
        检查两个数值是否在容差范围内相等（满足任一容差即可）
        
        Args:
            val1: 第一个值
            val2: 第二个值
            
        Returns:
            bool: 如果在容差范围内则返回True
        """
        # 检查是否都是数值类型
        if not (isinstance(val1, (int, float)) and isinstance(val2, (int, float))):
            return False
        
        # 转换为浮点数
        f1, f2 = float(val1), float(val2)
        
        # 处理特殊情况
        if f1 == f2:
            return True
        
        diff = abs(f1 - f2)
        
        # 绝对容差检查
        if diff <= self.absolute_tolerance:
            return True
        
        # 相对容差检查
        if f1 != 0:
            relative_diff = diff / abs(f1)
            if relative_diff <= self.numeric_tolerance:
                return True
        if f2 != 0:
            relative_diff = diff / abs(f2)
            if relative_diff <= self.numeric_tolerance:
                return True
        
        return False

    def load_json(self, file_path: str) -> Dict[str, Any]:
        """加载JSON文件"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        except FileNotFoundError:
            raise FileNotFoundError(f"文件未找到: {file_path}")
        except json.JSONDecodeError as e:
            raise ValueError(f"JSON格式错误 in {file_path}: {e}")
    
    def compare(self, obj1: Any, obj2: Any, path: str = "") -> List[Dict[str, Any]]:
        """比较两个JSON对象的差异"""
        self.differences = []
        self.ignored_defaults_count = 0
        self._compare_recursive(obj1, obj2, path)
        return self.differences
    
    def _compare_recursive(self, obj1: Any, obj2: Any, path: str):
        """递归比较对象"""
        # 类型不同的特殊处理
        if type(obj1) != type(obj2):
            # 特殊处理：int vs float的情况，自动转换为float进行数值比较
            if (isinstance(obj1, int) and isinstance(obj2, float)) or (isinstance(obj1, float) and isinstance(obj2, int)):
                # 转换为float进行数值比较
                float1, float2 = float(obj1), float(obj2)
                if not self._is_numeric_equal(float1, float2):
                    # 计算差异百分比
                    if float1 != 0:
                        diff_percent = abs(float1 - float2) / abs(float1) * 100
                    elif float2 != 0:
                        diff_percent = abs(float1 - float2) / abs(float2) * 100
                    else:
                        diff_percent = 0
                    
                    self.differences.append({
                        'type': 'numeric_changed',
                        'path': path,
                        'value1': obj1,
                        'value2': obj2,
                        'abs_diff': abs(float1 - float2),
                        'diff_percent': diff_percent
                    })
                # 如果数值相等（在容差范围内），则不记录差异
                return
            else:
                # 其他类型不匹配的情况
                self.differences.append({
                    'type': 'type_mismatch',
                    'path': path,
                    'value1': obj1,
                    'value2': obj2,
                    'type1': type(obj1).__name__,
                    'type2': type(obj2).__name__
                })
                return
        
        # 字典比较
        if isinstance(obj1, dict):
            self._compare_dicts(obj1, obj2, path)
        
        # 列表比较
        elif isinstance(obj1, list):
            self._compare_lists(obj1, obj2, path)
        
        # 基本类型比较
        else:
            # 对于数值类型，使用容差比较
            if isinstance(obj1, (int, float)) and isinstance(obj2, (int, float)):
                if not self._is_numeric_equal(obj1, obj2):
                    # 计算差异百分比以便在报告中显示
                    if obj1 != 0:
                        diff_percent = abs(obj1 - obj2) / abs(obj1) * 100
                    elif obj2 != 0:
                        diff_percent = abs(obj1 - obj2) / abs(obj2) * 100
                    else:
                        diff_percent = 0
                    
                    self.differences.append({
                        'type': 'numeric_changed',
                        'path': path,
                        'value1': obj1,
                        'value2': obj2,
                        'diff_percent': diff_percent,
                        'abs_diff': abs(obj1 - obj2)
                    })
            # 对于其他类型，使用严格比较
            elif obj1 != obj2:
                self.differences.append({
                    'type': 'value_changed',
                    'path': path,
                    'value1': obj1,
                    'value2': obj2
                })
    
    def _compare_dicts(self, dict1: dict, dict2: dict, path: str):
        """比较字典"""
        # 找出只在dict1中存在的键
        only_in_1 = set(dict1.keys()) - set(dict2.keys())
        for key in only_in_1:
            new_path = f"{path}.{key}" if path else key
            
            # 忽略 hash 键（skeleton.hash 通常会变化但不影响功能）
            if key == "hash" and (path == "skeleton" or path == ""):
                continue
            
            # 检查是否为默认值差异
            if self.ignore_defaults and SpineDefaultValues.is_default_value(new_path, dict1[key], self.numeric_tolerance, self.absolute_tolerance):
                self.ignored_defaults_count += 1
                continue  # 忽略此差异
            
            self.differences.append({
                'type': 'key_removed',
                'path': new_path,
                'value1': dict1[key],
                'value2': None
            })
        
        # 找出只在dict2中存在的键
        only_in_2 = set(dict2.keys()) - set(dict1.keys())
        for key in only_in_2:
            new_path = f"{path}.{key}" if path else key
            
            # 忽略 hash 键（skeleton.hash 通常会变化但不影响功能）
            if key == "hash" and (path == "skeleton" or path == ""):
                continue
            
            # 检查是否为默认值差异
            if self.ignore_defaults and SpineDefaultValues.is_default_value(new_path, dict2[key], self.numeric_tolerance, self.absolute_tolerance):
                self.ignored_defaults_count += 1
                continue  # 忽略此差异
            
            self.differences.append({
                'type': 'key_added',
                'path': new_path,
                'value1': None,
                'value2': dict2[key]
            })
        
        # 比较共同存在的键
        common_keys = set(dict1.keys()) & set(dict2.keys())
        for key in common_keys:
            new_path = f"{path}.{key}" if path else key
            
            # 忽略 hash 键的值比较（skeleton.hash 通常会变化但不影响功能）
            if key == "hash" and (path == "skeleton" or path == ""):
                continue
            
            self._compare_recursive(dict1[key], dict2[key], new_path)
    
    def _compare_lists(self, list1: list, list2: list, path: str):
        """比较列表"""
        len1, len2 = len(list1), len(list2)
        
        # 长度不同
        if len1 != len2:
            self.differences.append({
                'type': 'length_mismatch',
                'path': path,
                'length1': len1,
                'length2': len2
            })
        
        # 比较共同长度部分
        min_len = min(len1, len2)
        for i in range(min_len):
            new_path = f"{path}[{i}]"
            self._compare_recursive(list1[i], list2[i], new_path)
        
        # 处理额外元素
        if len1 > len2:
            for i in range(len2, len1):
                new_path = f"{path}[{i}]"
                self.differences.append({
                    'type': 'item_removed',
                    'path': new_path,
                    'value1': list1[i],
                    'value2': None
                })
        elif len2 > len1:
            for i in range(len1, len2):
                new_path = f"{path}[{i}]"
                self.differences.append({
                    'type': 'item_added',
                    'path': new_path,
                    'value1': None,
                    'value2': list2[i]
                })


class DiffReporter:
    """差异报告生成器"""
    
    @staticmethod
    def format_simple(differences: List[Dict[str, Any]], ignored_count: int = 0) -> str:
        """简单格式输出"""
        if not differences:
            if ignored_count > 0:
                return f"✅ 两个JSON文件在意义上相同\n💡 已忽略 {ignored_count} 个默认值差异\n"
            else:
                return "✅ 两个JSON文件完全相同\n"
        
        report = f"发现 {len(differences)} 个差异"
        if ignored_count > 0:
            report += f" (已忽略 {ignored_count} 个默认值差异)"
        report += ":\n\n"
        
        for i, diff in enumerate(differences, 1):
            diff_type = diff['type']
            path = diff['path'] or 'root'
            
            if diff_type == 'value_changed':
                report += f"{i}. 值变更 at {path}:\n"
                report += f"   文件1: {diff['value1']}\n"
                report += f"   文件2: {diff['value2']}\n\n"
            
            elif diff_type == 'numeric_changed':
                report += f"{i}. 数值差异超出容差 at {path}:\n"
                report += f"   文件1: {diff['value1']}\n"
                report += f"   文件2: {diff['value2']}\n"
                report += f"   绝对差异: {diff['abs_diff']:.6f}\n"
                report += f"   相对差异: {diff['diff_percent']:.2f}%\n\n"
            
            elif diff_type == 'type_mismatch':
                report += f"{i}. 类型不匹配 at {path}:\n"
                report += f"   文件1: {diff['type1']} = {diff['value1']}\n"
                report += f"   文件2: {diff['type2']} = {diff['value2']}\n\n"
            
            elif diff_type == 'key_added':
                report += f"{i}. 新增键 at {path}:\n"
                report += f"   文件2中新增: {diff['value2']}\n\n"
            
            elif diff_type == 'key_removed':
                report += f"{i}. 删除键 at {path}:\n"
                report += f"   文件1中存在: {diff['value1']}\n\n"
            
            elif diff_type == 'length_mismatch':
                report += f"{i}. 数组长度不匹配 at {path}:\n"
                report += f"   文件1长度: {diff['length1']}\n"
                report += f"   文件2长度: {diff['length2']}\n\n"
            
            elif diff_type == 'item_added':
                report += f"{i}. 数组新增元素 at {path}:\n"
                report += f"   文件2中新增: {diff['value2']}\n\n"
            
            elif diff_type == 'item_removed':
                report += f"{i}. 数组删除元素 at {path}:\n"
                report += f"   文件1中存在: {diff['value1']}\n\n"
        
        return report
    
    @staticmethod
    def format_detailed(differences: List[Dict[str, Any]], ignored_count: int = 0) -> str:
        """详细格式输出"""
        if not differences:
            if ignored_count > 0:
                return f"✅ JSON文件比较结果: 在意义上相同\n💡 已忽略 {ignored_count} 个默认值差异\n"
            else:
                return "✅ JSON文件比较结果: 完全相同\n"
        
        # 按类型分组统计
        stats = {}
        for diff in differences:
            diff_type = diff['type']
            stats[diff_type] = stats.get(diff_type, 0) + 1
        
        report = "📊 JSON差异分析报告\n"
        report += "=" * 50 + "\n\n"
        
        report += "📈 差异统计:\n"
        for diff_type, count in stats.items():
            type_names = {
                'value_changed': '值变更',
                'numeric_changed': '数值差异超出容差',
                'type_mismatch': '类型不匹配',
                'key_added': '新增键',
                'key_removed': '删除键',
                'length_mismatch': '长度不匹配',
                'item_added': '新增数组元素',
                'item_removed': '删除数组元素'
            }
            report += f"  • {type_names.get(diff_type, diff_type)}: {count}\n"
        
        if ignored_count > 0:
            report += f"  • 已忽略默认值差异: {ignored_count}\n"
        
        report += f"\n总计: {len(differences)} 个差异"
        if ignored_count > 0:
            report += f" (忽略了 {ignored_count} 个默认值)"
        report += "\n\n"
        
        report += "📋 详细差异列表:\n"
        report += "-" * 50 + "\n"
        
        return report + DiffReporter.format_simple(differences, ignored_count)
    
    @staticmethod
    def format_json(differences: List[Dict[str, Any]], ignored_count: int = 0) -> str:
        """JSON格式输出"""
        return json.dumps({
            'total_differences': len(differences),
            'ignored_defaults': ignored_count,
            'differences': differences
        }, ensure_ascii=False, indent=2)


def main():
    parser = argparse.ArgumentParser(description='比较两个JSON文件的差异')
    parser.add_argument('file1', help='第一个JSON文件路径')
    parser.add_argument('file2', help='第二个JSON文件路径')
    parser.add_argument('--output', '-o', help='输出报告到文件')
    parser.add_argument('--format', '-f', 
                       choices=['simple', 'detailed', 'json'], 
                       default='simple',
                       help='输出格式 (默认: simple)')
    parser.add_argument('--tolerance', '-t', 
                       type=float, 
                       default=0.10,
                       help='数值比较相对容差 (默认: 0.10，即10%%)')
    parser.add_argument('--abs-tolerance', '-a',
                       type=float,
                       default=0.01,
                       help='数值比较绝对容差 (默认: 0.01)')
    parser.add_argument('--no-ignore-defaults', 
                       action='store_true',
                       help='不忽略默认值差异')
    
    args = parser.parse_args()
    
    try:
        # 检查文件是否存在
        if not Path(args.file1).exists():
            print(f"错误: 文件不存在 - {args.file1}")
            sys.exit(1)
        
        if not Path(args.file2).exists():
            print(f"错误: 文件不存在 - {args.file2}")
            sys.exit(1)
        
        # 创建比较器并执行比较
        differ = JSONDiffer(
            numeric_tolerance=args.tolerance, 
            absolute_tolerance=args.abs_tolerance,
            ignore_defaults=not args.no_ignore_defaults
        )
        
        print(f"正在比较文件:")
        print(f"  文件1: {args.file1}")
        print(f"  文件2: {args.file2}")
        print(f"  相对容差: {args.tolerance*100:.1f}%")
        print(f"  绝对容差: {args.abs_tolerance}")
        if args.no_ignore_defaults:
            print(f"  忽略默认值: 否")
        else:
            print(f"  忽略默认值: 是")
        print()
        
        json1 = differ.load_json(args.file1)
        json2 = differ.load_json(args.file2)
        
        differences = differ.compare(json1, json2)
        
        # 生成报告
        if args.format == 'simple':
            report = DiffReporter.format_simple(differences, differ.ignored_defaults_count)
        elif args.format == 'detailed':
            report = DiffReporter.format_detailed(differences, differ.ignored_defaults_count)
        elif args.format == 'json':
            report = DiffReporter.format_json(differences, differ.ignored_defaults_count)
        
        # 输出报告
        if args.output:
            with open(args.output, 'w', encoding='utf-8') as f:
                f.write(report)
            print(f"报告已保存到: {args.output}")
        else:
            print(report)
        
        # 设置退出码
        sys.exit(0 if not differences else 1)
        
    except Exception as e:
        print(f"错误: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
