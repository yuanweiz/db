#include <FileSystem.h>
#include <PageAllocator.h>
#include <detail/DataPage.h>
using iterator = File::iterator;
bool File::iterator::operator!=(const iterator& rhs){
    return cellIdx_ == rhs.cellIdx_ && tie_ == rhs.tie_;
};

StringView iterator::operator*()const {
    detail::DataPageView view(tie_->getNonConst(), tie_->pageSize());
    return view.getCell(cellIdx_);
}

iterator& iterator::operator++(){
    detail::DataPageView view(tie_->getNonConst(), tie_->pageSize());
    if (++cellIdx_ == view.numOfCells()){
        tie_ = pageAllocator_->getPage( PageNo_t(view.next()));
        cellIdx_ = 0;
    }
    return *this;
}

iterator::iterator(uint16_t cellIdx, PageAllocatorBase* pageAllocator,const PagePtr& tie)
    :cellIdx_(cellIdx),pageAllocator_(pageAllocator), tie_(tie)
{
}

iterator File::begin(){
    return iterator(0,pageAllocator_, 
            pageAllocator_->getPage(PageNo_t(0)));
}

iterator File::end(){
    return iterator(0,pageAllocator_, 
            pageAllocator_->getPage(PageNo_t(0)));
}

File::File(const PagePtr& rootPage)
    :rootPage_(rootPage)
{
}


iterator File::insert(StringView , const File::KeyComparator&){
    return begin();
}
void FileSystem::mountOrBuild(){
}
FileSystem::FileSystem(PageAllocatorBase* allocator)
    :pageAllocator_(allocator){
    }

std::shared_ptr<File> FileSystem::openFile(const std::string& name){
    if (name == "master"){
        mountOrBuild();
        auto rootPage = pageAllocator_->getPage(PageNo_t(2));
        return std::make_shared<File>(std::move(rootPage));
    }
    else {
        return nullptr;//TODO
    }
}
