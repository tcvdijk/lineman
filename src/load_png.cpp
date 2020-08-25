#include "load_png.h"

using std::make_unique;

#include <vector>
using std::vector;

#include <string>
using std::string;

#include "Logging.h"

#include "lodepng.h"

#include "Image.h"
#include "Logging.h"

std::unique_ptr<Image> load_png(const std::filesystem::path &path) {
	vector<unsigned char> pixels;
	unsigned int W, H;
	string filename = path.filename().string();
	console->info("Loading image \"{}\" a PNG ...", filename);

	unsigned int error = lodepng::decode(pixels, W, H, filename);
	if (error) {
		console->error("lodepng error: {0}", lodepng_error_text(error));
		return nullptr;
	}

	console->info("Image dimensions : {} x {}", W, H);
	console->info("Constructing my image data structure ...");
	return make_unique<Image>(W, H, pixels.data());
}
