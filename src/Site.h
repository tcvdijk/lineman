#ifndef INCLUDED_SITE
#define INCLUDED_SITE

#include <vector>

#include "Image.h"
#include "Point.h"

#include "Logging.h"

class Site {
public:
	Image *img;
	Point<Space::World> p;
	int xmin, xmax;
	int ymin, ymax;
	int size;
	double sigma;
	struct State {
		double value;
		Point<Space::Local> from;
	};
	std::vector<State> state;

	Site(Image *img, Point<Space::World> p, int size, double sigma);
	Site(const Site &) = delete;

	State &get(Point<Space::Local> lp);
	double scorePosition(Point<Space::Local> lp);
	double scoreTransition(Point<Space::World> a, Point<Space::World> b);
	void calculateAsFirst();
	void calculateFromPrevious(Site *prevSite);
	Point<Space::Local> findBest();
	Point<Space::World> translateToWorld(Point<Space::Local> local);
};

#endif //ndef INCLUDED_SITE
