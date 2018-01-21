#ifndef __HELPER_H
#define __HELPER_H
#include <string>
#include <stdlib.h>
#include <unordered_map>
#include <PageAllocator.h>
#include <SignalSlot.h>
#include <Exception.h>
namespace Helper{
inline std::string randomData(size_t sz){
    std::string readableCharacters=
        "qwertyuiopasdfghjklzxcvbnm"
        "QWERTYUIOPASDFGHJKLZXCVBNM"
        "0123456789"
        ;
    std::string s;
    const auto size = static_cast<int>(readableCharacters.size());
    for (size_t i=0;i<sz;++i){
        int index = rand()%size;
        s.push_back(readableCharacters[index]);
    }
    return s;
}
inline void fillRandomData(void* dst_,size_t size){
    char * dst = static_cast<char*>(dst_);
    for (size_t i=0;i<size;++i){
        char c = static_cast<char>(rand());
        dst[i]=c;
    }
}
class MockPageAllocator: public PageAllocatorBase{
public:
    PagePtr forceAllocate( PageNo_t page)override{
        auto & pagePtr = pages_[page];
        if (pagePtr){
            throw Exception("occupied");
        }
        pagePtr.reset( PageData::allocMemory(page,PageSz_t(4096)));
        pageAlloced.emit(page);
        return pagePtr;
    }
    PagePtr getPageImpl(PageNo_t t)override{
        auto &ptr= pages_[t];
        if (!ptr){
            ptr.reset(PageData::allocMemory(t,PageSz_t(4096)));
        }
        return ptr;
    }
    PagePtr allocate()override{
        //naive algorithm
        for (uint32_t i=0; ; ++i){
            PageNo_t pageNo(i);
            auto & pagePtr = pages_[i];
            if (!pagePtr){
                pagePtr.reset(PageData::allocMemory(pageNo, PageSz_t(4096)));
                return pagePtr;
            }
            else continue;
        }
    }
    void deallocate(PagePtr & ptr)override{
        auto it = pages_.find(ptr->pageNo());
        if (it ==pages_.end()){
            throw Exception("double free");
        }
        pages_.erase(it);
        ptr = nullptr; //clear it
        pageDealloced.emit(ptr->pageNo());
    }
    Signal<PageNo_t> pageAlloced;
    Signal<PageNo_t> pageDealloced;
private:
    std::unordered_map<size_t, PagePtr> pages_;
};
}
#endif// __HELPER_H
