#include <gtest/gtest.h>
#include "../src/storage/file_writer.h"
TEST(FileWriter, Parent){EXPECT_TRUE(novafetch::ensureParent("testfile.bin"));}
