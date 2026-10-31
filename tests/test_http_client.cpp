#include <gtest/gtest.h>
#include "../src/network/http_client.h"
TEST(Http, InvalidPort){std::string e;EXPECT_FALSE(novafetch::canReach("http://127.0.0.1:1/",e));}
