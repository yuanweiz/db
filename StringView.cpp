#include <string.h>
#include <StringView.h>
bool StringView::operator==(const StringView&rhs)
{
    return rhs.sz_ == sz_ && ::strncmp(rhs.data_,data_,sz_)==0;
}
StringView::StringView(const char* data, size_t sz)
    :data_(data),sz_(sz)
{
}

