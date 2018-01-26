#ifndef __FILESYSTEM_H
#define __FILESYSTEM_H
#include <string>
#include <functional>
#include <memory>
#include <StringView.h>
#include <PagePtr.h>
class PageAllocatorBase;
class File{
    public:
        friend class FileSystem;
        class iterator{
            friend class File;
            public:
            StringView operator*() const;
            iterator& operator++();
            bool operator!=(const iterator&);
            private:
            iterator(uint16_t, PageAllocatorBase*,const PagePtr&);
            uint16_t cellIdx_;
            PageAllocatorBase* pageAllocator_;
            PagePtr tie_;
        };
        using KeyComparator = std::function<bool(StringView,StringView)>;
        using RetrieveKeyFunc = std::function<StringView(StringView)>;
        iterator insert(StringView,const KeyComparator&); 
        iterator find(StringView, const KeyComparator&);
        iterator erase(StringView, const KeyComparator&);
        iterator begin();
        iterator end();
        explicit File(const PagePtr&);
    private:
        PageAllocatorBase* pageAllocator_;
        PagePtr rootPage_;
};
class FileSystem{
    public:
        explicit FileSystem(PageAllocatorBase*);
        std::shared_ptr<File> openFile(const std::string &name);
    private:
        void mountOrBuild();
        PageAllocatorBase* pageAllocator_;
};

#endif// __FILESYSTEM_H

