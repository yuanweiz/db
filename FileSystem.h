#ifndef __FILESYSTEM_H
#define __FILESYSTEM_H
#include <string>
#include <functional>
#include <memory>
#include <StringView.h>
#include <PagePtr.h>

class Cursor{
    public:
        Cursor(Cursor&&)=default;
        StringView operator*() const;
        Cursor& operator++();
        bool operator!=(const Cursor&);
        ~Cursor();
    protected:
        class Impl;
        static Cursor fromPimpl(Impl*);
    private:
        std::unique_ptr<Impl> pImpl_;
};
class File{
    public:
        File(File&&)=default;
        using KeyComparator = std::function<bool(StringView,StringView)>;
        using RetrieveKeyFunc = std::function<StringView(StringView)>;
        Cursor insert(StringView,const KeyComparator&); 
        Cursor find(StringView, const KeyComparator&);
        Cursor erase(StringView, const KeyComparator&);
        Cursor begin();
        Cursor end();
        explicit File(const PagePtr&);
        ~File();
    protected:
        class Impl;
        static File fromPimpl(Impl*);
    private:
        std::unique_ptr<Impl> pImpl_;
};

class FileSystem{
    public:
        FileSystem(FileSystem&&)=default;
        File openFile(const std::string &name);
        ~FileSystem();
    protected:
        class Impl;
        static  FileSystem fromPimpl(Impl*);
    private:
        std::unique_ptr<Impl> pImpl_;
};

#endif// __FILESYSTEM_H

