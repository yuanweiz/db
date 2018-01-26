#ifndef __FILE_IMPL_H
#define __FILE_IMPL_H
#include <FileSystem.h>

class File::Impl{
    public:
        using KeyComparator = std::function<bool(StringView,StringView)>;
        using RetrieveKeyFunc = std::function<StringView(StringView)>;
        //virtual Cursor insert(StringView,const KeyComparator&)=0; 
        //virtual Cursor find(StringView, const KeyComparator&)=0;
        //virtual Cursor erase(StringView, const KeyComparator&)=0;
        virtual Cursor begin()=0;
        virtual Cursor end()=0;
};


#endif
