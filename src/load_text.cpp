#include "load_text.h"

using std::vector;
using std::filesystem::path;

#include <string>
using std::string;
#include <sstream>
using std::istringstream;
#include <fstream>
using std::ifstream;

#include "Logging.h"

vector<Point<Space::World>> load_text(const path &path) {
	vector<Point<Space::World>> result;
	ifstream polyfile(path.string());
	if (!polyfile.good()) {
		console->error("Couldn't open {}.", path.string());
		return {};
	}
	int vertnum = 0;
	Point<Space::World> lastPoint{ -1,-1 };
	while (!polyfile.eof()) {
		string linestr;
		getline(polyfile, linestr);
		istringstream line(linestr);
		Point<Space::World> p{ -1,-1 };
		line >> p.x >> p.y;
		if (p.y != -1) {
			lastPoint = p;
			result.push_back(p);
		}
		++vertnum;
	}
	result.push_back(lastPoint);
	return result;
}