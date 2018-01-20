#ifndef __HELPER_H
#define __HELPER_H
#include <string>
#include <stdlib.h>
namespace Helper{
inline std::string randomData(size_t sz){
    srand(0);
    std::string s;
    for (size_t i=0;i<sz;++i){
        char c = static_cast<char>(rand());
        s.push_back(c);
    }
    return s;
}
inline void fillRandomData(void* dst_,size_t size){
    char * dst = static_cast<char*>(dst_);
    for (size_t i=0;i<size;++i){
        char c = static_cast<char>(rand());
        dst[i]=c;
    }
}
}
#endif// __HELPER_H
