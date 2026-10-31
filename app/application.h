#pragma once
#include <windows.h>
class NovaFetchApplication { HINSTANCE instance_; public: explicit NovaFetchApplication(HINSTANCE h):instance_(h){} int run(int show); };
