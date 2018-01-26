#ifndef __CURSOR_IMPL_H
#define __CURSOR_IMPL_H
#include <FileSystem.h>
class Cursor::Impl{
    public:
        virtual StringView get() const=0;
        virtual Impl& advance()=0;
        virtual bool equals(const Impl&)=0;
};

#endif
