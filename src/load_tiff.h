#ifndef INCLUDED_LOAD_TIFF
#define INCLUDED_LOAD_TIFF

#include <memory>
#include <filesystem>

class Image;

std::unique_ptr<Image> load_tiff(const std::filesystem::path &path);

#endif //ndef INCLUDED_LOAD_TIFF
