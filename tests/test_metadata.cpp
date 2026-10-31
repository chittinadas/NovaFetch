#include <gtest/gtest.h>
#include "../src/storage/metadata_store.h"
#include <filesystem>
TEST(Metadata, Save){auto p=(std::filesystem::temp_directory_path()/"nf-meta.json").string();EXPECT_TRUE(novafetch::saveMetadata(p,"{}"));std::filesystem::remove(p);}
