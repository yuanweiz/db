#include <vector>
#include <unordered_map>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "PageAllocator.h"
#include "Exception.h"
#include "BitVector.h"
#include "Pager.h"
using namespace std;
const uint32_t kPageSize = 4096;
const uint32_t kSuperBlockNo = 0;
const uint32_t kPagesPerGroup = kPageSize*8;
const char kMagic[] = "WZDB";
class PageAllocator::Impl{
public:
    explicit Impl(const string& fname)
        :state_(State::None),
        pager_(fname,PageSz_t(kPageSize))
    {
        superBlockPtr_ = pager_.getPage(PageNo_t(0));
        const auto ptr = superBlockPtr_.getConst();
        if (0==memcmp(ptr,kMagic,sizeof(kMagic))){
            state_ = State::Formatted;
        }
    }
    PagePtr allocPage(){
        if (state_==State::None){
            throw Exception("Hasn't yet been formatted");
        }
        return PagePtr();
    }
    void freePage(const PagePtr&){
        if (state_==State::None){
            throw Exception("Hasn't yet been formatted");
        }
    }
    bool formatted()const{
        return state_ == State::Formatted;
    }
    void format(){
        bitmapPages_.clear();
        state_ = State::Formatted;
        auto ptr = superBlockPtr_.get();
        memcpy(ptr,kMagic,sizeof(kMagic));
        auto nPages = pager_.pageCount();
        auto nGroups = nPages / kPagesPerGroup;
        auto rest = nPages % kPagesPerGroup;
        for (uint32_t i=0;i<nGroups;++i){
            rewriteBitmap(i);
        }
        if (rest){
            rewriteBitmap(nGroups);
        }
    }
private:
    void rewriteBitmap(uint32_t groupNo){
        auto & bitmapPage = getBitmap(groupNo);
        auto ptr = bitmapPage.get();
        BitVector bitmap(ptr);
        bitmap.set(0,true);
        bitmap.set(1,true);
    }
    PagePtr& getBitmap(uint32_t groupNo){
        auto it = bitmapPages_.find(groupNo);
        if (it != bitmapPages_.end()){
            return it->second;
        }
        PageNo_t page(groupNo* kPagesPerGroup+1);
        auto ret = bitmapPages_.insert({groupNo,pager_.getPage(page)});
        assert(ret.second);
        return ret.first->second;
    }
    enum class State {
        None, Formatted
    }state_;
    Pager pager_;
    PagePtr superBlockPtr_;
    std::unordered_map<size_t, PagePtr> bitmapPages_;
};

PageAllocator::PageAllocator(const string& fname)
    :pImpl_(new Impl(fname))
{
}
void PageAllocator::format(){
    pImpl_->format();
}
bool PageAllocator::formatted()const{
    return pImpl_->formatted();
}

PageAllocator::~PageAllocator(){
}
