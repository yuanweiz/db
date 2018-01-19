#ifndef __STRING_VIEW_H
#define __STRING_VIEW_H
#include <string>
#include <sys/types.h>
class StringView{
public:
    StringView(const char*,size_t);
    StringView(const std::string&);
    const char* data()const{return data_;}
    size_t size()const {return sz_;}
    operator std::string ();
    bool operator==(const StringView&rhs);
private:
    const char* data_;
    size_t sz_;
};
#endif// __STRING_VIEW_H
