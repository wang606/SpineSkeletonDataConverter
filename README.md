# Spine Skeleton Data Converter

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
| json2skel | 🚧  | 🚧  | 🚧  | 🚧  | 🚧  |

**Legend**: ✅ Supported | 🚧 In Development | ❌ Not Supported

## 🚀 Quick Start

### Prerequisites

- CMake 3.10 or higher
- C++20 compatible compiler
- Visual Studio 2019+ (Windows) or GCC/Clang (Linux/macOS)

### Building from Source

```bash
# Clone the repository
git clone <repository-url>
cd SpineSkeletonDataConverter

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build . --config Release

# Install to target directory
cmake --install .
```

### Usage

The converter automatically detects the Spine version and performs the conversion:

```bash
# Basic usage
skel2json.exe <input_skel_path> <output_json_path>

# Examples
skel2json.exe skeleton.skel output.json
skel2json.exe "C:\spine\character.skel" "C:\output\character.json"

# Help information
skel2json.exe -h
```

#### Sample Output
```
Detecting Spine version...
Detected Spine version: 4.1
Converting skeleton.skel to output.json...
Conversion completed successfully!
```

## 🛠️ Development Tools

The project includes several Python utilities for development and testing:

- **`convert_skel_to_json.py`**: Batch conversion script
- **`validate_json_files.py`**: JSON validation utility
- **`analyze_json_differences.py`**: Compare JSON outputs
- **`json_diff.py`**: Detailed JSON difference analysis
- **`run_pipeline.py`**: Automated testing pipeline

## 📁 Project Structure

```bash
SpineSkeletonDataConverter/
├── src/                     # Core source code
│   ├── main.cpp            # Entry point and version detection
│   ├── skel2json.h         # Header definitions
│   ├── skel2json37.cpp     # Spine 3.7 converter
│   ├── skel2json38.cpp     # Spine 3.8 converter
│   ├── skel2json40.cpp     # Spine 4.0 converter
│   ├── skel2json41.cpp     # Spine 4.1 converter
│   ├── skel2json42.cpp     # Spine 4.2 converter
│   └── json.hpp            # JSON library
├── test/                   # Test files and Spine runtimes
│   ├── spine/              # Spine C++ runtime libraries
│   └── test_*.cpp          # Version-specific test cases
├── tools/                  # Development utilities
├── data/                   # Test data for different versions
├── target/                 # Output directory for built executables
└── build/                  # CMake build directory
```

## 🧪 Testing

Run the comprehensive test suite:

```bash
# Build and run tests
cd target
./test_skel2json.exe
```

## 🙏 Acknowledgments

- [Spine Runtime](http://esotericsoftware.com/) - For the excellent 2D animation software
- [nlohmann/json](https://github.com/nlohmann/json) - For the JSON library
- Contributors and testers who helped improve this tool

---

**Made with ❤️ for the Spine animation community**

