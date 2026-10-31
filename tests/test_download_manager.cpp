#include <gtest/gtest.h>
#include "../src/core/download_manager.h"
TEST(Download, RejectsFTP){std::string e;EXPECT_FALSE(novafetch::DownloadManager{}.download("ftp://x","x",{},e));}
