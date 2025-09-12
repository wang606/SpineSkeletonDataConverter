# Spine Skeleton Data Converter

> ⚠️ **Development Warning**  
> This repository is under active development and features may be unstable.

A powerful command-line tool for converting Spine skeleton data between different formats, with automatic version detection and support for multiple Spine runtime versions.

## ✨ Features

- **Automatic Version Detection**: Intelligently detects Spine version from `.skel` files
- **Multi-Version Support**: Compatible with Spine versions 3.7, 3.8, 4.0, 4.1, and 4.2
- **High-Performance Conversion**: Fast and reliable conversion between `.skel` and `.json` formats
- **Cross-Platform**: Built with C++20 and CMake for maximum portability
- **Comprehensive Testing**: Includes test suites for all supported versions

## 📋 Format Support Matrix

|           | 3.7 | 3.8 | 4.0 | 4.1 | 4.2 |
| --------- | --- | --- | --- | --- | --- |
| skel2json | ✅  | ✅  | ✅  | ✅  | ✅  |
| json2skel | 🚧  | 🚧  | 🚧  | 🚧  | ✅  |

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

## 🙏 Acknowledgments

- [Spine Runtime](http://esotericsoftware.com/) - For the excellent 2D animation software
- [nlohmann/json](https://github.com/nlohmann/json) - For the JSON library
- Contributors and testers who helped improve this tool

---

**Made with ❤️ for the Spine animation community**

