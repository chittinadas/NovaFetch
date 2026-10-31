#include "portable_paths.h"
#include <filesystem>
namespace novafetch {std::string dataDirectory(){return (std::filesystem::current_path()/"NovaFetchData").string();}}
