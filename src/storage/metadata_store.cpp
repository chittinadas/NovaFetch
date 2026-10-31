#include "metadata_store.h"
#include <fstream>
namespace novafetch {bool saveMetadata(const std::string&p,const std::string&v){std::ofstream f(p);if(!f)return false;f<<v;return bool(f);}}
