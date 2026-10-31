#include "file_writer.h"
#include <filesystem>
namespace novafetch {bool ensureParent(const std::string&p){std::filesystem::path x(p);if(x.has_parent_path())std::filesystem::create_directories(x.parent_path());return true;}}
