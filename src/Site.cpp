#include <limits>
using std::numeric_limits;

#include "Site.h"

Site::Site(Image *img, Point<Space::World> p, int size, double sigma ) : img(img), p(p), size(size), sigma(sigma) {
	int dim = 2 * size + 1;
	state.resize(dim*dim);
	if (p.x - size < 0) xmin = p.x;
	else xmin = -size;
	if (p.x + size >= img->W) xmax = img->W - 1 - p.x;
	else xmax = size;
	if (p.y - size < 0) ymin = p.y;
	else ymin = -size;
	if (p.y + size >= img->H) ymax = img->H - 1 - p.y;
	else ymax = size;
}

Site::State &Site::get(Point<Space::Local> lp) {
	lp.x += size; lp.y += size;
	return state[lp.y*(2 * size + 1) + lp.x];
}

double Site::scoreTransition(Point<Space::World> a, Point<Space::World> b) {
	return log(1.0 - img->avg(a, b));
}

double Site::scorePosition(Point<Space::Local> lp) {
	double d = sqrt(lp.x*lp.x + lp.y*lp.y);
	double s = sigma;
	double gauss = exp(-(d*d) / (s*s));
	return log(gauss);
}

void Site::calculateAsFirst() {
	// don't move the first vertex:
	// 1. set all scores to -infty, except:
	// 2. set local (0,0) to score 0
	for (int y = ymin; y <= ymax; ++y) {
		for (int x = xmin; x <= xmax; ++x) {
			get(Point<Space::Local>{ x, y }) = { numeric_limits<double>::lowest(), {-1, -1} };
		}
	}
	get(Point<Space::Local>{0, 0}) = { 0, {-1,-1} };
}

void Site::calculateFromPrevious(Site *prevSite) {
	for (int y = ymin; y <= ymax; ++y) {
		for (int x = xmin; x <= xmax; ++x) {
			Point<Space::Local> lp{ x, y };
			get(lp).value = numeric_limits<double>::lowest();
			double posScoreHere = scorePosition(lp);
			for (int prevx = prevSite->xmin; prevx <= prevSite->xmax; ++prevx) {
				for (int prevy = prevSite->ymin; prevy <= prevSite->ymax; ++prevy) {
					Point<Space::Local> prevP{ prevx,prevy };
					double score = posScoreHere;
					score += prevSite->get(prevP).value;
					score += scoreTransition(translateToWorld(lp), prevSite->translateToWorld(prevP));
					if (score > get(lp).value) {
						get(lp) = { score, prevP };
					}
				}
			}
		}
	}
}

Point<Space::Local> Site::findBest() {
	double bestValue = numeric_limits<double>::lowest();
	Point<Space::Local> best;
	for (int y = ymin; y <= ymax; ++y) {
		for (int x = xmin; x <= xmax; ++x) {
			Point<Space::Local> lp{ x, y };
			if (get(lp).value > bestValue) {
				bestValue = get(lp).value;
				best = lp;
			}
		}
	}
	return best;
}

Point<Space::World> Site::translateToWorld(Point<Space::Local> local) {
	Point<Space::World> result = { local.x, local.y };
	result.x += p.x;
	result.y += p.y;
	return result;
}
