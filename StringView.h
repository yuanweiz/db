#ifndef __STRING_VIEW_H
#define __STRING_VIEW_H
#include <sys/types.h>
class StringView{
public:
    StringView(const char*,size_t);
private:
    const char* ptr_;
    const size_t sz_;
};
#endif// __STRING_VIEW_H
