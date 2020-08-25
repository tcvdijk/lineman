static const char USAGE[] =
R"(
Usage:
   align [--config=<file>]... [--png|--tiff] [options] [<image>] [<linestring>]

Options:
   -h --help                         Show this screen.
   -v --verbose                      Verbose output.
   -q --quiet                        Do not output effective settings and stats to json.
   --config=<file>...                Get settings from json file. Can set any commandline
                                     options; gets applied before commandline.
   
   image                             Filename of background image (png or tiff).
   --png                             Open image as PNG despite file extension.
   --tiff                            Open image as TIFF despite file extension.
   
   linestring                        Filename of linestring (geojson).
                                     Points are interpreted as image-space in pixels:
                                     * top-left is (0,0),
                                     * positive x goes right into image,
                                     * negative y goes down into image.
   -p <string>, --pointer=<string>   JSON pointer to where to find the LineString feature.
                                     Default is document root.
   -o <file>, --output=<file>        Write result to file rather than standard out.
   
   --output-full-dom                 Replicate full DOM from the linestring file in the
                                     output, rather than just the LineString features
                                     that was processed.
   -t <text>, --tag=<text>           Add an arbitrary string to the config object in the
                                     output json.
   -d <file>, --debug-image=<file>   Write debug image to file.
   
   -m <px>, --max-displace=<px>      Maximum vertex displacement in pixels.
   -k <px>, --kernel-sigma=<px>      Sigma in pixels for the geometric term.
   -s <n>, --stride=<n>              Use only 1 out of every n vertices of the input.
                                     That is, stride 1 uses every vertex; stride 2 uses
                                     vertex 0, 2, 4, et cetera. Default 1.
   --subdivide=<n>                   Place an additional n vertices along every segment of
                                     the line string. Applies after stride. Default 0.
)";

#include "Timer.h"
#include "Image.h"
#include "Site.h"
#include "LineString.h"
#include "Config.h"
#include "Stats.h"

#include "load_png.h"
#include "load_tiff.h"

#include "load_text.h"
#include "load_geojson.h"

#include "write_geojson.h"

#include "proj_api.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <iterator>
#include <cstdio>
using namespace std;

#include "Logging.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "docopt.h"

#include "lodepng.h"


int main(int argc, char **argv) {

	// Parse command line arguments.
	auto args = docopt::docopt(USAGE, { argv + 1, argv + argc }, true, "Align");

	// Setup logging console and handle verbose flag
	console = spdlog::stderr_color_mt("console");
	if (args["--verbose"].asBool()) console->set_level(spdlog::level::info);
	else console->set_level(spdlog::level::err);

	// Set up configuration
	Config config;
	for (auto filename : args["--config"].asStringList()) {
		config.ingest_json(filename);
	}
	config.ingest_docopt(args);
	bool showed_config = false;
	if (config.output_filename != "" && !args["--quiet"].asBool()) {
		config.json_to_stdout();
		showed_config = true;
	}

	// Show help and abort if image, linestring or output filenames are empty.
	if (config.image_filename == "" || config.linestring_filename == "") {
		console->error("Input image or input linestring not specified; aborting.");
		if (!showed_config) {
			config.json_to_stdout();
		}
		puts(USAGE);
		return 3;
	}


	ofstream output_filestream;
	if (config.output_filename != "") {
		// See if we can open the output file now, before we do any work:
		// bail if we cannot.
		output_filestream.open(config.output_filename, ios::binary);
		if (output_filestream.bad()) {
			console->error("Cannot open {} for output; aborting.", config.output_filename);
			return 4;
		}
	}
	ostream &output_stream = config.output_filename == "" ? cout : output_filestream;

	// Load image
	console->info("Loading image ...");
	filesystem::path image_path = config.image_filename;
	filesystem::path image_path_base = image_path.stem();
	unique_ptr<Image> img;
	{
		string ext = image_path.extension().string();
		if (config.force_png || (!config.force_tiff && ext == ".png")) {
			img = load_png(image_path);
		}
		else if (config.force_tiff || ext == ".tif" || ext == ".tiff") {
			img = load_tiff(image_path);
		}
		else {
			console->error("Did not recognise file extention (.png, .tif, .tiff). Use the --png or --tiff option to force.");
			return 6;
		}
	}
	if (img == nullptr) {
		console->error("Failed to load image. Hopefully you got an error message above.");
		return 7;
	}
	
	// Load polyline
	console->info("Loading polyline ...");
	filesystem::path polyline_path = config.linestring_filename;
	filesystem::path polyline_path_base = polyline_path.stem();
	rapidjson::Document dom;
	vector<Point<Space::World>> input_polyline = load_geojson(dom, polyline_path, config.geojson_pointer, config.linestring_stride, config.linestring_subdivide, img->to_pixel_coords);
	if (input_polyline.empty()) {
		console->error("No vertices loaded; aborting.");
		return 5;
	}
	console->info("Number of vertices : {}.", input_polyline.size());

	// Set up LineString
	Timer viterbiTime;
	LineString line_string;
	line_string.points.reserve(input_polyline.size());
	copy(input_polyline.begin(), input_polyline.end(), back_inserter(line_string.points));

	// Run Viterbi algorithm
	console->info("Running Viterbi alignment ...");
	line_string.align_viterbi(img.get(), config.window_size, config.global_sigma);

	// Log some statistics.
	chrono::duration<double> runtime = viterbiTime.elapsed();
	console->info("Total time    : {} s", runtime.count());
	chrono::duration<double, std::milli> ms_per_point = runtime / (input_polyline.size() - 1);
	console->info("Time per step : {} ms", ms_per_point.count());
	Stats stats;
	stats.time_total = runtime.count();
	stats.time_per_point = ms_per_point.count() / 1000.0;
	stats.log_score = line_string.log_score;


	// Write result json
	console->info("Writing output geojson ...");
	write_geojson(dom, config, stats, line_string, output_stream, img->from_pixel_coords);


	// Output debug image
	if (config.debug_image_filename != "") {
		console->info("Rendering debug image for {}...", config.debug_image_filename);
		vector<unsigned char> data(img->W*img->H * 4, 0);
		for (unique_ptr<Site> &site : line_string.viterbi) {
			double max_val = numeric_limits<double>::lowest();
			for( int dy = -config.window_size; dy <= config.window_size; ++dy ) {
				for (int dx = -config.window_size; dx <= config.window_size; ++dx) {
					Point<Space::Local> here{ dx, dy };
					Point<Space::World> world = site->translateToWorld(here);
					if (world.x < 0 || world.x >= img->W) continue;
					if (world.y < 0 || world.y >= img->H) continue;
					double val = site->get(here).value;
					max_val = max(val, max_val);
				}
			}
			max_val = exp(max_val);
			for (int dy = -config.window_size; dy <= config.window_size; ++dy) {
				for (int dx = -config.window_size; dx <= config.window_size; ++dx) {
					Point<Space::Local> here{ dx, dy };
					Point<Space::World> world = site->translateToWorld(here);
					if (world.x < 0 || world.x >= img->W) continue;
					if (world.y < 0 || world.y >= img->H) continue;
					unsigned char val = 255 * exp(site->get(here).value) / max_val;
					data[4 * (world.y*img->W + world.x) + 0] = val;
					data[4 * (world.y*img->W + world.x) + 1] = val;
					data[4 * (world.y*img->W + world.x) + 2] = val;
					data[4 * (world.y*img->W + world.x) + 3] = 255;
				}
			}
		}
		console->info("Writing debug image ...");
		vector<unsigned char> png_buffer;
		unsigned error = lodepng::encode(png_buffer, data, img->W, img->H);
		lodepng::save_file(png_buffer, config.debug_image_filename);
		if (error) {
			console->error("lodepng error: {0}", lodepng_error_text(error));
			return 16;
		}
	}

	console->info("Done.");

}
