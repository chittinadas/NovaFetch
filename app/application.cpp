#include "application.h"
#include "../src/ui/main_window.h"
int NovaFetchApplication::run(int show){NovaFetchMainWindow w(instance_);if(!w.create())return 1;ShowWindow(w.handle(),show);MSG m{};while(GetMessageW(&m,nullptr,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}return (int)m.wParam;}
