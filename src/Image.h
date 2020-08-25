#pragma once
#ifndef INCLUDED_IMAGE
#define INCLUDED_IMAGE

#include <vector>
#include <string>
#include <functional>

#include "Point.h"

#include "lodepng.h"

class Image {
public:
	struct Stroke {
		Stroke() {}
		Stroke(Segment seg, unsigned char r, unsigned char g, unsigned char b) : seg(seg), r(r), g(g), b(b) {}
		Segment seg;
		unsigned char r, g, b;
	};
	std::vector<Stroke> strokes;

	Image(int W, int H, const unsigned char *srcpixel);
	~Image();
	std::vector<double> pixel;
	int W, H;
	double &get(int x, int y) {
		return pixel[W*y + x];
	}

	void flip_y();

	std::function<void(double*, double*)> to_pixel_coords = nullptr;
	std::function<void(double*, double*)> from_pixel_coords = nullptr;
	std::function<void()> cleanup = nullptr;

	// This just adds the polyline to a buffer.
	// Polylines are only rendered on ::save.
	void addPolyline(const std::vector<Point<Space::World>> &polyline, unsigned char r, unsigned char g, unsigned char b);

	double avg(Point<Space::World> a, Point<Space::World> b);

	void save(const std::string &fname);

	// Drawing visitors.
	// visit(x,y,val) gets called for every pixel with
	//   x, y the pixel coordinates in global space
	//   val the original value at that pixel (double)
	struct Draw {
		Draw(double val) : myValue(val) {}
		double myValue;
		void visit(int, int, double &value);
	};
	struct DrawRGBA {
		DrawRGBA(std::vector<unsigned char> &rgba, int W, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
			: rgba(rgba), W(W), r(r), g(g), b(b), a(a) {}
		std::vector<unsigned char> &rgba;
		int W;
		unsigned char r, g, b, a;
		void visit(int x, int y, double&);

	};
	struct SegmentStat {
		int count = 0;
		double sum = 0;
		SegmentStat() = default;
		void visit(int, int, double &value);
	};

	// Bresenham implementation with visitor template
	template< class Visitor >
	void bresenham(Visitor &v, Point<Space::World> a, Point<Space::World> b) {
		if (abs(b.y - a.y) < abs(b.x - a.x)) {
			if (a.x > b.x) {
				return bresenhamLow(v, b, a);
			}
			else {
				return bresenhamLow(v, a, b);
			}
		}
		else {
			if (a.y > b.y) {
				return bresenhamHigh(v, b, a);
			}
			else {
				return bresenhamHigh(v, a, b);
			}
		}
	}
	template< class Visitor >
	void bresenhamLow(Visitor &v, Point<Space::World> a, Point<Space::World> b) {
		int dx = b.x - a.x;
		int dy = b.y - a.y;
		int yi = 1;
		if (dy < 0) {
			yi = -1;
			dy = -dy;
		}
		int D = 2 * dy - dx;
		int y = a.y;
		for (int x = a.x; x <= b.x; ++x) {
			v.visit(x, y, get(x, y));
			if (D > 0) {
				y = y + yi;
				D = D - 2 * dx;
			}
			D = D + 2 * dy;
		}
	}
	template< class Visitor >
	void bresenhamHigh(Visitor &v, Point<Space::World> a, Point<Space::World> b) {
		int dx = b.x - a.x;
		int dy = b.y - a.y;
		int xi = 1;
		if (dx < 0) {
			xi = -1;
			dx = -dx;
		}
		int D = 2 * dx - dy;
		int x = a.x;
		for (int y = a.y; y <= b.y; ++y) {
			v.visit(x, y, get(x, y));
			if (D > 0) {
				x = x + xi;
				D = D - 2 * dy;
			}
			D = D + 2 * dx;
		}
	}


};

#endif //ndef INCLUDED_IMAGE
