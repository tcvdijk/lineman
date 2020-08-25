#include "load_png.h"

using std::make_unique;

#include <vector>
using std::vector;

#include <string>
using std::string;

#include "Logging.h"
#include "xtiffio.h"
#include "geotiffio.h"

#include "Image.h"

extern "C" void tiff_error_handler(const char *module, const char *fmt, va_list args) {
	string mod = module ? module : "";
	const int N = 1024;
	char buffer[N];
	int end = vsnprintf(buffer, N, fmt, args);
	buffer[end] = '\0';
	console->error("({}) - {}", mod, buffer);
}

extern "C" void tiff_warning_handler(const char *module, const char *fmt, va_list args) {
	string mod = module ? module : "";
	const int N = 1024;
	char buffer[N];
	int end = vsnprintf(buffer, N, fmt, args);
	buffer[end] = '\0';
	console->warn("({}) - {}", mod, buffer);
}

std::unique_ptr<Image> load_tiff(const std::filesystem::path &path) {
	uint32_t W, H;
	string filename = path.filename().string();
	console->info("Loading image \"{}\" as TIFF ...", filename);

	// Load pixel data from TIFF
	TIFFSetErrorHandler(&tiff_error_handler);
	TIFFSetWarningHandler(&tiff_warning_handler);
	TIFF *tif;
	tif = XTIFFOpen(filename.c_str(), "r");
	if (tif == nullptr) {
		return nullptr;
	}
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &W);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &H);
	uint32_t npixels = W * H;
	uint32_t *raster = (uint32_t *)_TIFFmalloc(npixels * sizeof(uint32_t));
	int result = TIFFReadRGBAImage(tif, W, H, raster, 0);
	if (result == 0) {
		console->error("Some error occured in TIFFReadRGBAImage.");
		_TIFFfree(raster);
		XTIFFClose(tif);
		return nullptr;
	}
	console->info("Image dimensions : {} x {}", W, H);
	console->info("Constructing my image data structure ...");
	auto img = make_unique<Image>(W, H, reinterpret_cast<unsigned char*>(raster));
	_TIFFfree(raster);
	img->flip_y();

	// Load GeoTIFF headers and set up coordinate transformation if appropriate
	GTIF *gtif = GTIFNew(tif);
	if (gtif == nullptr) {
		console->info("Failed to open TIFF as GeoTIFF.");
	}
	else {
		int versions[3] = { -1,-1,-1 }, keycount = -1;
		GTIFDirectoryInfo(gtif, versions, &keycount);
		console->info("GeoTIFF version {}.{}.{}", versions[0], versions[1], versions[2]);
		console->info("GeoTIFF directory has {} keys.", keycount);
		img->to_pixel_coords = [gtif](double *x, double *y) {
			GTIFPCSToImage(gtif, x, y);
		};
		img->from_pixel_coords = [gtif](double *x, double *y) {
			GTIFImageToPCS(gtif, x, y);
		};
	}

	// Set up cleanup
	img->cleanup = [tif, gtif]() {
		GTIFFree(gtif);
		XTIFFClose(tif);
	};

	return img;
}
