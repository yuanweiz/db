#include <unordered_map>
#include <gtest/gtest.h>
#include <FileSystem.h>
#include <PageAllocator.h>
#include <Exception.h>
#include <SignalSlot.h>
#include "helper.h"

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
    PagePtr getPage(PageNo_t t)override{
        return pages_[t];
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

bool stringViewLess(StringView a,StringView b){
    return std::string(a) < std::string(b); //slow
}
void test(){
    {
        MockPageAllocator allocator;
        FileSystem fs(&allocator);
        auto file = fs.openFile("master");
        char buf[1024];
        std::vector<std::string> sortedStrings;
        for (int i=0;i<20;++i){
            size_t size = rand()%100;
            Helper::fillRandomData(buf,size);
            StringView strView{buf,size};
            sortedStrings.push_back(strView);
            file->insert(strView, stringViewLess );
        }
        std::sort(sortedStrings.begin(),sortedStrings.end());
        size_t i=0;
        for (auto && strView: *file){
            std::string str = strView;
            ASSERT_TRUE( i < sortedStrings.size());
            ASSERT_EQ( str, sortedStrings[i++]);
        }
    }
}

int main (int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
