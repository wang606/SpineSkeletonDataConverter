# Spine Skeleton Data Converter

> ⚠️ **Development Warning**  
>
> This repository is under active development and features may be unstable.

A powerful command-line tool for converting Spine skeleton data between different formats, with automatic version detection and support for multiple Spine runtime versions.

## ✨ Features

- **Automatic Version Detection**: Intelligently detects Spine version from `.skel` or `.json` files
- **Multi-Version Support**: Compatible with Spine versions 3.7, 3.8, 4.0, 4.1, and 4.2
- **Cross-Platform**: Built with C++20 and CMake for maximum portability
- **Comprehensive Testing**: Includes test suites for all supported versions

## 📋 Format Support Matrix

|           | 3.7 | 3.8 | 4.0 | 4.1 | 4.2 |
| --------- | --- | --- | --- | --- | --- |
| JSON Reader | 🚧 | 🚧 | 🚧 | 🚧 | ✅  |
| JSON Writer | ✅ | ✅ | ✅ | ✅ | ✅  |
| Binary Reader | ✅ | ✅ | ✅ | ✅ | ✅ |
| Binary Writer | 🚧 | 🚧 | 🚧 | 🚧 | ✅ |
| Convert to the next version | 🚧 | 🚧 | 🚧 | 🚧 | ❌ |
| Convert to the previous version | ❌ | 🚧 | 🚧 | 🚧 | 🚧 |

**Legend**: ✅ Supported | 🚧 In Development | ❌ Not Supported

## 🚀 Quick Start

### Building from Source

```bash
# Clone the repository
git clone https://github.com/wang606/SpineSkeletonDataConverter.git
cd SpineSkeletonDataConverter

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build . --config Release
```

### Usage

```bash
# Basic usage
./build/Release/SpineSkeletonDataConverter.exe input.skel output.json --in-skel --out-json

# The tool auto-detects file formats from extensions
./build/Release/SpineSkeletonDataConverter.exe input.json output.skel

# Version detection is automatic based on file content
```

## 🧪 Testing

The project includes comprehensive testing tools to validate conversion accuracy and data integrity across all supported Spine versions.

For each Spine version, we recommend running three rounds of tests to ensure complete functionality:

#### Round 1: JSON Round-trip Test (JSON → JSON)
Tests JSON reading and writing functionality. The generated `.json.json` files should be identical to the original `.json` files.

```bash
# Step 1: Convert all JSON files to JSON (tests JSON reader/writer)
$ python ./tools/TestSpineSkeletonDataConverter.py --exe ./build/Debug/SpineSkeletonDataConverter.exe ./data/42 .json .json

# Step 2: Compare all original vs converted JSON files
$ python ./tools/json_diff_all.py ./data/42 .json .json.json

# Step 3: Detailed comparison of specific different files (if any)
$ python ./tools/json_diff.py ./data/42/chibi-stickers/export/chibi-stickers.json ./data/42/chibi-stickers/export/chibi-stickers.json.json
```

#### Round 2: SKEL to JSON Test (SKEL → JSON)
Tests SKEL reading functionality. The generated `.skel.json` files don't need to be identical to original JSON files, as SKEL and JSON formats have inherent differences.

```bash
# Step 1: Convert all SKEL files to JSON (tests SKEL reader)
$ python ./tools/TestSpineSkeletonDataConverter.py --exe ./build/Debug/SpineSkeletonDataConverter.exe ./data/42 .skel .json

# Step 2: Compare results (for reference, differences are expected)
$ python ./tools/json_diff_all.py ./data/42 .json .skel.json
```

#### Round 3: Full Round-trip Test (JSON → SKEL → JSON)
Tests complete functionality: JSON reader, SKEL writer, SKEL reader, and JSON writer. The final `.json.skel.json` files should be identical to the original `.json` files.

```bash
# Step 1: Convert JSON → SKEL → JSON (tests all 4 functions)
$ python ./tools/TestSpineSkeletonDataConverter.py --exe ./build/Debug/SpineSkeletonDataConverter.exe ./data/42 .json .skel.json

# Step 2: Compare original vs final converted files
$ python ./tools/json_diff_all.py ./data/42 .json .json.skel.json

# Step 3: Detailed analysis of any differences
$ python ./tools/json_diff.py ./data/42/example/export/example.json ./data/42/example/export/example.json.skel.json
```

## 🙏 Acknowledgments

- [Spine Runtime](http://esotericsoftware.com/) - For the excellent 2D animation software
- [nlohmann/json](https://github.com/nlohmann/json) - For the JSON library
- Contributors and testers who helped improve this tool

---

**Made with ❤️ for the Spine animation community**

