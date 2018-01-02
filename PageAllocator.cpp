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
        const auto ptr = superBlockPtr_->getConst();
        if (0==memcmp(ptr,kMagic,sizeof(kMagic))){
            state_ = State::Formatted;
        }
    }
    PagePtr allocPage(){
        if (state_==State::None){
            throw Exception("Hasn't yet been formatted");
        }
        uint32_t suggestedGroup=lastAlloc_ / kPagesPerGroup;
        uint32_t suggestedPage=lastAlloc_ % kPagesPerGroup;
        auto nGroups = maxGroup();
        for (uint32_t i=0;i<nGroups;++i){
            auto ii=(i+suggestedGroup)%nGroups;
            auto bitmapPage = getBitmap(ii,false);
            BitVector bitmap(bitmapPage.get());
            for (uint32_t j=0;j<kPagesPerGroup;++j){
                uint32_t jj = (j+ suggestedPage)%kPagesPerGroup;
                if (!bitmap.get(jj)){
                    bitmap.set(jj,true);
                    lastAlloc_ = ii*kPagesPerGroup+jj;
                    return pager_.getPage(PageNo_t(lastAlloc_));
                }
            }
        }
        //allocate a new Group
        auto & bitmapPage = getBitmap (nGroups,true);
        assert(nGroups + 1== maxGroup()); //expanded
        BitVector bitmap(bitmapPage.get());
        bitmap.set(2,true);
        lastAlloc_ =nGroups*kPagesPerGroup+2;
        return pager_.getPage(PageNo_t(lastAlloc_));
    }
    void freePage(const PagePtr& pagePtr){
        if (state_==State::None){
            throw Exception("Hasn't yet been formatted");
        }
        uint32_t page = pagePtr->pageNo();
        auto & ptr = getBitmap(page/kPagesPerGroup,true);
        BitVector bitmap ( ptr.get());
        auto off = page % kPagesPerGroup;
        if (!bitmap.get(off)){
            throw Exception("trying to free a page that hasn't been allocated");
        }
        bitmap.set(off,false);
    }
    bool formatted()const{
        return state_ == State::Formatted;
    }
    void format(){
        state_ = State::Formatted;
        bitmapPages_.clear();
        auto ptr = superBlockPtr_->getNonConst();
        memcpy(ptr,kMagic,sizeof(kMagic));
        auto nGroups = maxGroup();
        for (uint32_t i=0;i<nGroups;++i){
            rewriteBitmap(i);
        }
    }
    uint32_t maxGroup()const{
        if (state_!=State::Formatted){
            throw Exception("not formatted, maxGroup() unavailable");
        }
        auto nPages= pager_.pageCount() ;
        uint32_t base = nPages/ kPagesPerGroup;
        uint32_t roundUp = nPages%kPagesPerGroup? 1:0;
        return base + roundUp;
    }
private:
    void rewriteBitmap(uint32_t groupNo){
        getBitmap(groupNo,true);
    }
    PagePtr& getBitmap(uint32_t groupNo, bool rewrite){
        auto it = bitmapPages_.find(groupNo);
        if (it != bitmapPages_.end()){
            return it->second;
        }
        PageNo_t page(groupNo* kPagesPerGroup+1);
        auto ret = bitmapPages_.insert({groupNo,pager_.getPage(page)});
        assert(ret.second);
        auto & bitmapPage = ret.first->second;
        if (rewrite){
            auto ptr = bitmapPage.get();
            ::bzero(ptr,bitmapPage->pageSize());
            BitVector bitmap(ptr); 
            bitmap.set(0,true);
            bitmap.set(1,true);
        }
        return bitmapPage;
    }
    enum class State {
        None, Formatted
    }state_;
    uint32_t lastAlloc_ = 0;
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

PagePtr PageAllocator::allocate(){
    return pImpl_->allocPage();
}

void PageAllocator::deallocate(PagePtr& ptr){
    return pImpl_->freePage(ptr);
}

uint32_t PageAllocator::maxGroup()const{
    return pImpl_->maxGroup();
}

PageAllocator::~PageAllocator(){
}
