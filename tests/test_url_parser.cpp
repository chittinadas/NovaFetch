#include <gtest/gtest.h>
#include "../src/network/url_parser.h"
TEST(URL, Schemes){EXPECT_TRUE(novafetch::isHttpUrl("https://example.org/a"));EXPECT_TRUE(novafetch::isHttpUrl("http://x"));EXPECT_FALSE(novafetch::isHttpUrl("ftp://x"));}
TEST(URL, Filename){EXPECT_EQ(novafetch::filenameFromUrl("https://x/a.zip?q=1"),"a.zip");}
