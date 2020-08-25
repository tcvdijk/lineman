#include "Image.h"
using std::vector;

#include <utility>
using std::swap;

#include "geotiff.h"

#include "Logging.h"

Image::Image(int W, int H, const unsigned char *srcpixel) : W(W), H(H) {
	pixel.reserve(W*H);
	for (int i = 0; i < W*H; ++i) {
		// ITU BT.709 brightness = 0.2126 R + 0.7152 G + 0.0722 B
		// divided by 255
		double score =
			0.000833725 * srcpixel[4 * i]
			+ 0.00280471 * srcpixel[4 * i + 1]
			+ 0.000283137 * srcpixel[4 * i + 2];
		// score = srcpixel[4 * i] / 255.0
		pixel.push_back(score);
	}
}
Image::~Image() {
	if (cleanup) cleanup();
}

void Image::flip_y() {
	int half_H = H / 2;
	for (int y = 0; y < half_H; ++y) {
		for (int x = 0; x < W; ++x) {
			swap(get(x, y), get(x, H - y - 1));
		}
	}
}

void Image::addPolyline(const vector<Point<Space::World>> &polyline, unsigned char r, unsigned char g, unsigned char b) {
	bool first = true;
	Point<Space::World> previous{ -1,-1 };
	for (Point<Space::World> current : polyline) {
		if (first) { first = false; previous = current; continue; }
		strokes.push_back({ { previous,current}, r, g, b });
		previous = current;
	}
}

double Image::avg(Point<Space::World> a, Point<Space::World> b) {
	SegmentStat stat;
	bresenham(stat, a, b);
	return stat.sum / double(stat.count);
}

void Image::save(const std::string &fname) {
	std::vector<unsigned char> png(W * H * 4);
	for (int i = 0; i < W*H; ++i) {
		unsigned char value = static_cast<unsigned char>(floor(pixel[i] * 255));
		png[4 * i + 0] = value;
		png[4 * i + 1] = value;
		png[4 * i + 2] = value;
		png[4 * i + 3] = 255;
	}
	for (Stroke s : strokes) {
		DrawRGBA visitor(png, W, s.r, s.g, s.b, 255);
		bresenham(visitor, s.seg.a, s.seg.b);
	}
	int error = lodepng::encode(fname, png, W, H);
	if (error) {
		console->error("{}", lodepng_error_text(error));
	}
	else {}
}

void Image::Draw::visit(int, int, double &value) {
	value = myValue;
}

void Image::DrawRGBA::visit(int x, int y, double&) {
	int i = 4 * (W*y + x);
	rgba[i + 0] = r;
	rgba[i + 1] = g;
	rgba[i + 2] = b;
	rgba[i + 3] = a;
}

void Image::SegmentStat::visit(int, int, double &value) {
	++count;
	sum += value;
}
