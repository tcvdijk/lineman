#ifndef INCLUDED_LOAD_PNG
#define INCLUDED_LOAD_PNG

#include <memory>
#include <filesystem>

class Image;

std::unique_ptr<Image> load_png(const std::filesystem::path &path);

#endif //ndef INCLUDED_LOAD_PNG
