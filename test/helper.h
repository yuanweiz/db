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
}
#endif// __HELPER_H
