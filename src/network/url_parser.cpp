#include "url_parser.h"
#include <algorithm>
#include <cctype>
namespace novafetch {
bool isHttpUrl(const std::string& s){auto eq=[&](const char*p){std::string x=p;return s.size()>=x.size()&&std::equal(x.begin(),x.end(),s.begin(),[](char a,char b){return std::tolower((unsigned char)a)==std::tolower((unsigned char)b);});};return eq("http://")||eq("https://");}
std::string filenameFromUrl(const std::string&s){auto e=s.find_first_of("?#");auto x=s.substr(0,e);auto p=x.find_last_of('/');auto n=p==std::string::npos?x:x.substr(p+1);return n.empty()?"download.bin":n;}
}
