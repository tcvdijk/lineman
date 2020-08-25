#ifndef INCLUDED_LINESTRING
#define INCLUDED_LINESTRING

#include <vector>
#include <memory>
#include <limits>

#include "Point.h"
class Site;
class Image;

class LineString {
public:
	// Input polyline
	std::vector<Point<Space::World>> points;

	// Viterbi tables, if available. Computed by ::align.
	std::vector<std::unique_ptr<Site>> viterbi;
	// Viterbi optimal solution, if available. Computed by ::align
	std::vector<Point<Space::World>> opt;
	// Log of Viterbi optimal score. Computed by ::align.
	double log_score = std::numeric_limits<double>::lowest();

	// Calculate Viterbi values and optimal aligned sequence.
	void align_viterbi( Image *img, int window_size, double sigma );

};

#endif //ndef INCLUDED_LINESTRING
