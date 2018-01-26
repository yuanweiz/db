#include <FileSystem.h>
#include <detail/FileSystemImpl.h>
#include <detail/FileImpl.h>
#include <detail/CursorImpl.h>

// Cursor 
StringView Cursor::operator*() const{
    return pImpl_->get();
}

Cursor& Cursor::operator++(){
    pImpl_->advance();
    return *this;
}

bool Cursor::operator!=(const Cursor& rhs){
    return !(pImpl_->equals(*rhs.pImpl_));
}

Cursor::~Cursor(){
}

Cursor::Cursor(Cursor::Impl*pImpl)
    :pImpl_(pImpl)
{
}

// FileSystem 
FileSystem::~FileSystem(){
}
File FileSystem::openFile(const std::string &name){
    return pImpl_->openFile(name);
}
FileSystem::FileSystem(FileSystem::Impl* pimpl)
    :pImpl_(pimpl)
{
}

//File
//Cursor File::insert(StringView,const KeyComparator&);

//Cursor File::find(StringView strView, const File::KeyComparator&cmp){
//    return pImpl_->find(strView,cmp);
//}
//Cursor erase(StringView, const KeyComparator&);
Cursor File::begin(){
    return pImpl_->begin();
}

Cursor File::end(){
    return pImpl_->end();
}

File::~File(){
}

File::File(Impl* pImpl)
    :pImpl_(pImpl)
{
}
