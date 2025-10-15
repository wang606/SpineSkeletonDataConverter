// Spine Atlas 4.x to 3.x Downgrader implemented in C++

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

namespace fs = std::filesystem;

struct AtlasRegion {
	std::string name;
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	int offsetX = 0;
	int offsetY = 0;
	int originalWidth = 0;
	int originalHeight = 0;
	int degrees = 0;
	int index = -1;
	std::vector<int> splits;
	std::vector<int> pads;
	std::vector<std::string> names;
	std::vector<int> values;
};

struct AtlasPage {
	std::string name;
	int width = 0;
	int height = 0;
	std::string format = "RGBA8888";
	std::string minFilter = "Nearest";
	std::string magFilter = "Nearest";
	std::string repeat = "none";
	bool pma = false;
	double scale = 1.0;
	std::vector<AtlasRegion> regions;
};

struct AtlasData {
	std::vector<AtlasPage> pages;
};

namespace {

std::string trim(const std::string& input) {
	size_t start = 0;
	size_t end = input.size();
	while (start < end && std::isspace(static_cast<unsigned char>(input[start]))) {
		++start;
	}
	while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
		--end;
	}
	return input.substr(start, end - start);
}

bool parseInt(const std::string& text, int& value) {
	try {
		size_t idx = 0;
		int v = std::stoi(text, &idx, 10);
		if (idx != text.size()) {
			return false;
		}
		value = v;
		return true;
	} catch (...) {
		return false;
	}
}

std::pair<std::string, std::vector<std::string>> parseEntry(const std::string& line) {
	auto colonPos = line.find(':');
	if (colonPos == std::string::npos) {
		return {"", {}};
	}

	std::string key = trim(line.substr(0, colonPos));
	std::string valuesStr = trim(line.substr(colonPos + 1));

	std::vector<std::string> values;
	std::string current;
	std::istringstream ss(valuesStr);
	while (std::getline(ss, current, ',')) {
		values.push_back(trim(current));
	}

	return {key, values};
}

AtlasData readAtlasData4x(const std::string& content) {
	AtlasData atlas;
	std::vector<std::string> lines;
	std::istringstream stream(content);
	std::string line;
	while (std::getline(stream, line)) {
		lines.push_back(line);
	}

	AtlasPage* currentPage = nullptr;
	size_t i = 0;
	while (i < lines.size()) {
		std::string trimmed = trim(lines[i]);
		if (trimmed.empty()) {
			++i;
			continue;
		}

		if (trimmed.find(':') == std::string::npos) {
			bool isPageStart = false;
			if (i + 1 < lines.size()) {
				std::string nextTrimmed = trim(lines[i + 1]);
				if (!nextTrimmed.empty() && nextTrimmed.rfind("size:", 0) == 0) {
					isPageStart = true;
				}
			}

			if (isPageStart) {
				atlas.pages.emplace_back();
				currentPage = &atlas.pages.back();
				currentPage->name = trimmed;
			} else if (currentPage) {
				currentPage->regions.emplace_back();
				currentPage->regions.back().name = trimmed;
			}

			++i;
			continue;
		}

		auto [key, values] = parseEntry(trimmed);
		if (key.empty()) {
			++i;
			continue;
		}

		if (currentPage && currentPage->regions.empty() &&
			(key == "size" || key == "format" || key == "filter" ||
			 key == "repeat" || key == "pma" || key == "scale")) {
			if (key == "size" && values.size() >= 2) {
				int w = 0;
				int h = 0;
				if (parseInt(values[0], w)) currentPage->width = w;
				if (parseInt(values[1], h)) currentPage->height = h;
			} else if (key == "format" && !values.empty()) {
				currentPage->format = values[0];
			} else if (key == "filter" && values.size() >= 2) {
				currentPage->minFilter = values[0];
				currentPage->magFilter = values[1];
			} else if (key == "repeat" && !values.empty()) {
				currentPage->repeat = values[0];
			} else if (key == "pma" && !values.empty()) {
				std::string lowered = values[0];
				std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				currentPage->pma = (lowered == "true");
			} else if (key == "scale" && !values.empty()) {
				try {
					currentPage->scale = std::stod(values[0]);
				} catch (...) {
					currentPage->scale = 1.0;
				}
			}
		} else if (currentPage && !currentPage->regions.empty()) {
			AtlasRegion& region = currentPage->regions.back();

			if (key == "bounds" && values.size() >= 4) {
				parseInt(values[0], region.x);
				parseInt(values[1], region.y);
				parseInt(values[2], region.width);
				parseInt(values[3], region.height);
			} else if (key == "xy" && values.size() >= 2) {
				parseInt(values[0], region.x);
				parseInt(values[1], region.y);
			} else if (key == "size" && values.size() >= 2) {
				parseInt(values[0], region.width);
				parseInt(values[1], region.height);
			} else if (key == "offset" && values.size() >= 2) {
				parseInt(values[0], region.offsetX);
				parseInt(values[1], region.offsetY);
			} else if (key == "orig" && values.size() >= 2) {
				parseInt(values[0], region.originalWidth);
				parseInt(values[1], region.originalHeight);
			} else if (key == "rotate" && !values.empty()) {
				std::string lowered = values[0];
				std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				if (lowered == "true") {
					region.degrees = 90;
				} else if (lowered == "false") {
					region.degrees = 0;
				} else {
					parseInt(values[0], region.degrees);
				}
			} else if (key == "index" && !values.empty()) {
				parseInt(values[0], region.index);
			} else if (key == "split" && values.size() >= 4) {
				region.splits.clear();
				for (const auto& entry : values) {
					int val = 0;
					if (parseInt(entry, val)) {
						region.splits.push_back(val);
					}
				}
			} else if (key == "pad" && values.size() >= 4) {
				region.pads.clear();
				for (const auto& entry : values) {
					int val = 0;
					if (parseInt(entry, val)) {
						region.pads.push_back(val);
					}
				}
			} else {
				region.names.push_back(key);
				for (const auto& entry : values) {
					int val = 0;
					if (parseInt(entry, val)) {
						region.values.push_back(val);
					}
				}
			}
		}

		++i;
	}

	return atlas;
}

std::string writeAtlasData3x(const AtlasData& atlas) {
	std::ostringstream output;

	for (const auto& page : atlas.pages) {
		output << page.name << '\n';

		double pageScale = page.scale != 0.0 ? page.scale : 1.0;
		int width = page.scale != 1.0 ? static_cast<int>(page.width / pageScale) : page.width;
		int height = page.scale != 1.0 ? static_cast<int>(page.height / pageScale) : page.height;
		output << "size: " << width << ", " << height << '\n';

		output << "format: " << page.format << '\n';
		output << "filter: " << page.minFilter << ", " << page.magFilter << '\n';
		output << "repeat: " << page.repeat << '\n';

		for (const auto& region : page.regions) {
			output << region.name << '\n';

			if (region.degrees == 90) {
				output << "  rotate: true\n";
			} else if (region.degrees == 0) {
				output << "  rotate: false\n";
			} else {
				output << "  rotate: " << region.degrees << '\n';
			}

			double scale = page.scale != 0.0 ? page.scale : 1.0;
			int x = scale != 1.0 ? static_cast<int>(region.x / scale) : region.x;
			int y = scale != 1.0 ? static_cast<int>(region.y / scale) : region.y;
			output << "  xy: " << x << ", " << y << '\n';

			int regionWidth = scale != 1.0 ? static_cast<int>(region.width / scale) : region.width;
			int regionHeight = scale != 1.0 ? static_cast<int>(region.height / scale) : region.height;
			output << "  size: " << regionWidth << ", " << regionHeight << '\n';

			if (!region.splits.empty() && region.splits.size() >= 4) {
				output << "  split: ";
				for (size_t idx = 0; idx < 4; ++idx) {
					int val = region.splits[idx];
					if (scale != 1.0) {
						val = static_cast<int>(val / scale);
					}
					output << val;
					if (idx < 3) {
						output << ", ";
					}
				}
				output << '\n';
			}

			if (!region.pads.empty() && region.pads.size() >= 4) {
				output << "  pad: ";
				for (size_t idx = 0; idx < 4; ++idx) {
					int val = region.pads[idx];
					if (scale != 1.0) {
						val = static_cast<int>(val / scale);
					}
					output << val;
					if (idx < 3) {
						output << ", ";
					}
				}
				output << '\n';
			}

			int origWidth = region.originalWidth > 0 ? region.originalWidth : region.width;
			int origHeight = region.originalHeight > 0 ? region.originalHeight : region.height;
			if (scale != 1.0) {
				origWidth = static_cast<int>(origWidth / scale);
				origHeight = static_cast<int>(origHeight / scale);
			}
			output << "  orig: " << origWidth << ", " << origHeight << '\n';

			int offsetX = scale != 1.0 ? static_cast<int>(region.offsetX / scale) : region.offsetX;
			int offsetY = scale != 1.0 ? static_cast<int>(region.offsetY / scale) : region.offsetY;
			output << "  offset: " << offsetX << ", " << offsetY << '\n';

			output << "  index: " << region.index << '\n';
		}

		output << '\n';
	}

	return output.str();
}

bool writeFile(const fs::path& path, const std::string& content) {
	std::ofstream ofs(path, std::ios::trunc);
	if (!ofs) {
		return false;
	}
	ofs << content;
	return true;
}

bool readFile(const fs::path& path, std::string& out) {
	std::ifstream ifs(path, std::ios::binary);
	if (!ifs) {
		return false;
	}
	std::ostringstream ss;
	ss << ifs.rdbuf();
	out = ss.str();
	return true;
}

bool scaleImageWithStb(const fs::path& inputPath, const fs::path& outputPath, double scaleFactor) {
	int width = 0;
	int height = 0;
	int channels = 0;
	stbi_uc* data = stbi_load(inputPath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!data) {
		const char* reason = stbi_failure_reason();
		if (reason) {
			std::cout << "  [ERROR] stb_image failed to load '" << inputPath.string() << "': " << reason << std::endl;
		}
		return false;
	}
	if (scaleFactor == 0.0) {
		stbi_image_free(data);
		return false;
	}

	channels = 4;
	int outWidth = static_cast<int>(std::lround(static_cast<double>(width) / scaleFactor));
	int outHeight = static_cast<int>(std::lround(static_cast<double>(height) / scaleFactor));
	outWidth = std::max(1, outWidth);
	outHeight = std::max(1, outHeight);

	std::vector<unsigned char> resized(static_cast<size_t>(outWidth) * outHeight * channels);
	if (outWidth == width && outHeight == height) {
		std::copy(data, data + static_cast<size_t>(width) * height * channels, resized.begin());
	} else {
		stbir_pixel_layout layout = STBIR_RGBA;
		if (!stbir_resize_uint8_linear(data, width, height, 0, resized.data(), outWidth, outHeight, 0, layout)) {
			stbi_image_free(data);
			return false;
		}
	}

	stbi_image_free(data);

	const int stride = outWidth * channels;
	return stbi_write_png(outputPath.string().c_str(), outWidth, outHeight, channels, resized.data(), stride) != 0;
}

bool scaleTextureImages(AtlasData& atlas, const fs::path& atlasDir, const fs::path& outputDir) {
	std::cout << "Processing texture images:" << std::endl;
	bool overallSuccess = true;

	for (auto& page : atlas.pages) {
		fs::path originalPath(page.name);
		fs::path inputPath = atlasDir / originalPath;
		if (!fs::exists(inputPath)) {
			std::cout << "  [WARN] Texture file not found: " << inputPath.string() << std::endl;
			overallSuccess = false;
			continue;
		}

		if (originalPath.extension().string() != ".png" && originalPath.extension().string() != ".PNG") {
			std::cout << "  [ERROR] Unsupported texture format for " << originalPath.string()
					  << ". Only .png textures are currently supported." << std::endl;
			overallSuccess = false;
			continue;
		}

		fs::path outputPath = outputDir / originalPath;

		fs::path outputParent = outputPath.parent_path();
		std::error_code ec;
		if (!outputParent.empty()) {
			fs::create_directories(outputParent, ec);
		}
		if (ec) {
			std::cout << "  [ERROR] Failed to create directory for " << outputPath.string() << ": " << ec.message() << std::endl;
			overallSuccess = false;
			continue;
		}

		double scale = page.scale;
		if (scale <= 0.0) {
			std::cout << "  [ERROR] Invalid scale value " << scale << " for " << originalPath.string() << std::endl;
			overallSuccess = false;
			continue;
		}

		bool needsScaling = std::abs(scale - 1.0) >= 1e-6;
		if (!needsScaling) {
			try {
				fs::copy_file(inputPath, outputPath, fs::copy_options::overwrite_existing);
				std::cout << "  [OK] Copied " << originalPath.string() << " (scale≈1.0)" << std::endl;
			} catch (const fs::filesystem_error& copyError) {
				std::cout << "  [ERROR] Failed to copy " << originalPath.string() << ": " << copyError.what() << std::endl;
				overallSuccess = false;
			}
			continue;
		}

		if (!scaleImageWithStb(inputPath, outputPath, scale)) {
			std::cout << "  [ERROR] Failed to process " << originalPath.string() << std::endl;
			overallSuccess = false;
		} else {
			std::cout << "  [OK] Scaled " << originalPath.string() << std::endl;
		}
	}

	return overallSuccess;
}

void printUsage(const char* programName) {
	std::cout << "Usage: " << programName << " <input_atlas> <output_dir>" << std::endl;
}

} // namespace

int main(int argc, char* argv[]) {
	if (argc != 3) {
		printUsage(argv[0]);
		return 1;
	}

	fs::path inputAtlas = argv[1];
	fs::path outputDir = argv[2];

	if (!fs::exists(inputAtlas)) {
		std::cerr << "Error: Input atlas file not found: " << inputAtlas.string() << std::endl;
		return 1;
	}

	std::error_code ec;
	fs::create_directories(outputDir, ec);
	if (ec) {
		std::cerr << "Error: Failed to create output directory: " << outputDir.string() << std::endl;
		return 1;
	}

	std::cout << "Converting Spine 4.x atlas: " << inputAtlas.filename().string() << std::endl;
	std::cout << "Output directory: " << outputDir.string() << std::endl;
	std::cout << "--------------------------------------------------" << std::endl;

	std::string atlasContent;
	if (!readFile(inputAtlas, atlasContent)) {
		std::cerr << "Error: Failed to read atlas file." << std::endl;
		return 1;
	}

	AtlasData atlasData = readAtlasData4x(atlasContent);
	fs::path atlasDir = inputAtlas.parent_path();
	if (atlasDir.empty()) {
		atlasDir = fs::current_path();
	}

	bool textureSuccess = scaleTextureImages(atlasData, atlasDir, outputDir);

	std::string atlas3xContent = writeAtlasData3x(atlasData);
	fs::path outputAtlasPath = outputDir / inputAtlas.filename();
	if (!writeFile(outputAtlasPath, atlas3xContent)) {
		std::cerr << "Error: Failed to write converted atlas file." << std::endl;
		return 1;
	}
	std::cout << "[OK] Atlas file converted: " << outputAtlasPath.string() << std::endl;

	std::cout << "--------------------------------------------------" << std::endl;
	if (textureSuccess) {
		std::cout << "Conversion completed." << std::endl;
	} else {
		std::cout << "Conversion completed with warnings." << std::endl;
	}
	return 0;
}
