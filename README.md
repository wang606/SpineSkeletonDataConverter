# Spine Skeleton Data Converter

A powerful command-line tool for converting Spine skeleton data between different formats and different versions, with automatic version detection and support for multiple Spine runtime versions.

## ✨ Features

- **Automatic Version Detection**: Intelligently detects Spine version from `.skel` or `.json` files
- **Multi-Version Support**: Compatible with Spine versions 3.5, 3.6, 3.7, 3.8, 4.0, 4.1, and 4.2
- **Smart Format Detection**: Automatically detects file formats based on file extensions
- **Cross-Platform**: Built with C++20 and CMake for maximum portability
- **Comprehensive Testing**: Includes test suites for all supported versions

## 📋 Format Support Matrix

|           | 3.5 | 3.6 | 3.7 | 3.8 | 4.0 | 4.1 | 4.2 |
| --------- | --- | --- | --- | --- | --- | --- | --- |
| JSON Reader | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅  |
| JSON Writer | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅  |
| Binary Reader | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Binary Writer | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |

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
# Basic skeleton format conversion (same version)
SpineSkeletonDataConverter.exe input.skel output.json

# The tool auto-detects file formats from extensions
SpineSkeletonDataConverter.exe input.json output.skel

# Cross-version conversion (convert 3.7 file to 4.2 format)
SpineSkeletonDataConverter.exe input37.json output42.json -v 4.2.11

# Convert new binary format to old version
SpineSkeletonDataConverter.exe new.skel old.json -v 3.8.99

# Supported file formats:
#   .json       Spine JSON format
#   .skel       Spine binary (SKEL) format

# Options:
#   -v              Output version (must be complete: x.y.z format)
#   --remove-curve  Strip animation curves instead of converting when crossing 3.x/4.x
#   --help          Show this help message

# Supported Spine versions: 3.5.x, 3.6.x, 3.7.x, 3.8.x, 4.0.x, 4.1.x, 4.2.x
# Note: Version must be specified in complete x.y.z format (e.g., 4.2.11, not 4.2)
# Input version detection is automatic based on file content.
# Output version defaults to input version unless specified with -v.
```

## 🛠️ Spine Atlas 4.x to 3.x Downgrade

A dedicated Python script for converting Spine 4.x atlas files to a 3.x compatible format.

**Features:**

- Converts `.atlas` file format, applying the `scale` property to all relevant metrics.
- Scales the associated PNG texture images according to the `scale` property.

**Usage:**
```bash
python SpineAtlasDowngrade.py input.atlas output_dir
```

The repository now also provides a native executable `SpineAtlasDowngrade` that mirrors the Python script.
Build it via CMake and run:

```
SpineAtlasDowngrade.exe input.atlas output_dir
```

When downgrading, textures are resized using stb; currently the native converter only supports PNG texture pages, so convert other formats in advance.

## 🧰 Batch Conversion Script

For large directories of Spine assets, use the bundled helper script `tools/SpineConverter.py` to drive both `SpineSkeletonDataConverter.exe` and `SpineAtlasDowngrade.exe` automatically.

```bash
python tools/SpineConverter.py <input_dir> <output_dir> \
	[-v x.y.z] \
	[--format same|json|skel|other] \
	[--remove-curve]
```

- Recursively processes `.json`, `.skel`, and `.atlas` files, preserving the folder structure in the output directory.
- `-v` overrides the target skeleton version (defaults to each file's original version).
- `--format` controls output formats for `.json`/`.skel` files (`same` keeps the original format, `json`/`skel` force a specific format, and `other` swaps between the two).
- `--remove-curve` forwards to the native converter, stripping animation curves instead of translating them when converting between 3.x and 4.x.
- `.atlas` files are always downgraded through `SpineAtlasDowngrade.exe`.

## 🧪 Testing

The project includes comprehensive testing tools to validate conversion accuracy and data integrity across all supported Spine versions.

For each Spine version, we recommend running three rounds of tests to ensure complete functionality:

### Round 1: JSON Round-trip Test (JSON → JSON)
Tests JSON reading and writing functionality. The generated `.json.json` files should be identical to the original `.json` files.

```bash
# Step 1: Convert all JSON files to JSON (tests JSON reader/writer)
$ python ./tools/TestSpineSkeletonDataConverter.py --exe ./build/Debug/SpineSkeletonDataConverter.exe ./data/42 .json .json

# Step 2: Compare all original vs converted JSON files
$ python ./tools/json_diff_all.py ./data/42 .json .json.json

# Step 3: Detailed comparison of specific different files (if any)
$ python ./tools/json_diff.py ./data/42/chibi-stickers/export/chibi-stickers.json ./data/42/chibi-stickers/export/chibi-stickers.json.json
```

### Round 2: SKEL to JSON Test (SKEL → JSON)
Tests SKEL reading functionality. The generated `.skel.json` files don't need to be identical to original JSON files, as SKEL and JSON formats have inherent differences.

```bash
# Step 1: Convert all SKEL files to JSON (tests SKEL reader)
$ python ./tools/TestSpineSkeletonDataConverter.py --exe ./build/Debug/SpineSkeletonDataConverter.exe ./data/42 .skel .json

# Step 2: Compare results (for reference, differences are expected)
$ python ./tools/json_diff_all.py ./data/42 .json .skel.json
```

### Round 3: Full Round-trip Test (JSON → SKEL → JSON)
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
- [stb libraries](https://github.com/nothings/stb) - For lightweight image loading, resizing, and writing
- Contributors and testers who helped improve this tool

## ⚖️License
This project is licensed under the [PolyForm Noncommercial License 1.0.0](https://polyformproject.org/licenses/noncommercial/1.0.0/).

You may use, modify, and share this project for noncommercial purposes only, and must retain attribution to the original author.
