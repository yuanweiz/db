#ifndef __STRING_VIEW_H
#define __STRING_VIEW_H
#include <sys/types.h>
class StringView{
public:
    StringView(const char*,size_t);
    const char* data()const{return data_;}
    size_t size()const {return sz_;}
    bool operator==(const StringView&rhs);
private:
    const char* data_;
    size_t sz_;
};
#endif// __STRING_VIEW_H
