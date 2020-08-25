#ifndef INCLUDED_POINT
#define INCLUDED_POINT

#include "rapidjson/document.h"

// Space is a tag type for Point just to keep track of whether
// the point is in "World"space (pixel coordinates in the full image)
// or local coordinates where (0,0) is the original current vertex.
// This does not do any conversion between coordinate systems: it is just
// a marker to keep track of what coordinate system a variable is supposed
// to be in: we get a compile error if we assign from one system to another.
//
// Usage example:
// Site::translateToWorld converts from Point<Local> to Point<World>.
enum class Space {
	World,
	Local
};

template<Space>
struct Point {
	Point() = default;
	Point(int x, int y) : x(x), y(y) {}
	int x, y;
};

struct Segment {
	Segment() = default;
	Segment(const Segment &seg) = default;
	Segment(Point<Space::World> a, Point<Space::World> b) : a(a), b(b) {}
	Point<Space::World> a, b;
};

#endif //ndef INCLUDED_POINT
