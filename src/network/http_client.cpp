#include "http_client.h"
#include <curl/curl.h>
namespace novafetch {bool canReach(const std::string&u,std::string&e){CURL*c=curl_easy_init();if(!c){e="curl init failed";return false;}curl_easy_setopt(c,CURLOPT_URL,u.c_str());curl_easy_setopt(c,CURLOPT_NOBODY,1L);curl_easy_setopt(c,CURLOPT_FOLLOWLOCATION,1L);auto r=curl_easy_perform(c);long s=0;curl_easy_getinfo(c,CURLINFO_RESPONSE_CODE,&s);curl_easy_cleanup(c);if(r!=CURLE_OK)e=curl_easy_strerror(r);return r==CURLE_OK&&s<400;}}
