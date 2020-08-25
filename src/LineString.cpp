#include "LineString.h"
#include "Image.h"
#include "Site.h"
#include "LinearProgress.h"

#include "Logging.h"

using std::vector;
using std::unique_ptr;
using std::make_unique;
using std::numeric_limits;

void LineString::align_viterbi(Image *img, int window_size, double sigma) {

	LinearProgress progress("Viterbi...", "points", points.size() - 1);
	log_score = numeric_limits<double>::lowest();
	viterbi.clear();
	viterbi.reserve(points.size());
	viterbi.emplace_back(make_unique<Site>(img, points[0], window_size, sigma));
	viterbi[0]->calculateAsFirst();
	progress.start();
	for (int i = 1; i < points.size(); ++i) {
		progress.tick();
		viterbi.emplace_back(make_unique<Site>(img, points[i], window_size, sigma));
		viterbi[i]->calculateFromPrevious(viterbi[i - 1].get());
	}
	progress.done();

	// Trace solution backwards
	console->info("Tracing solution backwards ...");
	opt.clear();
	opt.reserve(points.size());
	Point<Space::Local> best{ 0, 0 }; // in local space: don't move last vertex
	log_score = viterbi.back()->get(best).value;
	opt.push_back(viterbi[points.size() - 1]->translateToWorld(best));
	for (int i = points.size() - 2; i >= 0; --i) {
		auto state = viterbi[i + 1]->get(best);
		best = state.from;
		opt.push_back(viterbi[i]->translateToWorld(best));
	}
	reverse(opt.begin(), opt.end());

}