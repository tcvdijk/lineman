#ifndef INCLUDED_LOAD_TEXT
#define INCLUDED_LOAD_TEXT

#include <vector>
#include <filesystem>

#include "Point.h"

std::vector<Point<Space::World>> load_text( const std::filesystem::path &path );

#endif //ndef INCLUDED_LOAD_TEXT