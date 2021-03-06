#include <list>
#include <unordered_map>
#include <assert.h>
#include <limits.h>
#include "Pager.h"
#include "PagePtr.h"
#include "Exception.h"
#include "ScopeExit.h"
using namespace std;
class Pager::Impl{
    using LRUList=std::list<PagePtr>;
    struct MapEntry{
        MapEntry(PagePtr& ptr, LRUList::iterator it)
            :wkPtr(ptr), listIter(it)
        {
        }
        std::weak_ptr<PageData> wkPtr;
        LRUList::iterator listIter;
    };
    using HashMap=std::unordered_map<uint32_t,MapEntry>;
public:
    Impl(FILE* fp, PageSz_t sz, size_t lruThreshold)
        :pageSz_(sz),largestGet_(0),lruThreshold_(lruThreshold),fp_(fp,&fclose)
    {
        ::fseek(fp,0,SEEK_END);
        auto off_int64 = ftell(fp);
        if (off_int64 <0){
            throw Exception("IO Error");
        }
        if (off_int64 > UINT_MAX){
            throw Exception("File too large");
        }
        auto off = static_cast<uint32_t>(off_int64);
        uint32_t pageSz = pageSz_;
        uint32_t roundUp = (off % pageSz) ? 1: 0;
        uint32_t initVal = off /pageSz + roundUp;
        if (initVal>0)--initVal;
        largestGet_= PageNo_t(initVal );
    }
    PageNo_t pageCount()const{ //automatically round up
        uint32_t  ret = largestGet_;
        return PageNo_t(ret+1);
    }
    PagePtr getPage(PageNo_t _page){
        auto onExit = [this](){sanityCheck();};
        ScopeExit<decltype(onExit)> exit(onExit);
        uint32_t page = _page;
        if (page > largestGet_){
            largestGet_ = _page;
        }
        auto mapIter = map_.find(page);
        if (mapIter!=map_.end()){
            auto& entry= mapIter->second;
            auto sharedPtr = entry.wkPtr.lock();
            assert(sharedPtr);
            tryTouch(entry);
            return PagePtr(sharedPtr);
        }
        auto ptr= PageData::fromFile(fp_.get(), pageSz_,_page,
                [this](void*buf, PageSz_t sz,PageNo_t pageNo){
                writeBack(buf,sz,pageNo);
                });
        auto listIter=list_.insert(list_.begin(), ptr);
        auto ret = map_.insert({page,{ptr,listIter}});
        assert(ret.second);(void)ret;
        if (lruThreshold_ >0 && list_.size() ){
            uint32_t key = list_.back()->pageNo();
            //invalidate it
            auto it = map_.find(key);
            assert(it!=map_.end());
            it->second.listIter = list_.end();
            list_.pop_back();
        }
        return ptr;
    }
    //PagePtr getPage(PageNo_t page, const PagePtr::EvictionCallback&cb){
    //}
    void tryTouch(MapEntry& entry){
        auto iter= entry.listIter;
        if (iter!=list_.end())
            list_.splice(list_.begin(),list_,iter);
    }
    void writeBack(void* buf,PageSz_t sz, PageNo_t page){
        //__asm__("int3");
        auto mapIter = map_.find(page);
        assert(mapIter!=map_.end());
        auto &wkPtr = mapIter->second.wkPtr;
        assert(wkPtr.expired());(void)wkPtr;
        map_.erase(mapIter);
        sanityCheck();

        long offset = sz;
        offset*= page;
        fseek(fp_.get(),offset, SEEK_SET);
        auto ret=fwrite(buf,1,sz,fp_.get());
        if (ret!=sz){
            char msg[128]={0};
            uint32_t u32 = sz;
            snprintf(msg,sizeof(msg),"expect %u but got %ld",u32,ret);
            throw Exception(msg);
        }
    }
    void sanityCheck()const{
        //assert(map_.size()==list_.size());
    }
private:
    //dumb implementation, never evicts
    PageSz_t pageSz_;
    PageNo_t largestGet_;
    size_t lruThreshold_;
    std::unique_ptr<FILE,decltype(&fclose)> fp_;
    //std::unordered_map<uint32_t,PagePtr> map_;
    HashMap map_;
    LRUList list_;
};

Pager::Pager(const string& fname, PageSz_t sz, size_t lruThreshold){
    FILE* fp = fopen(fname.c_str(), "r+");
    if (!fp){
        fp = fopen(fname.c_str(),"w+");
    }
    if (!fp){
        throw Exception("open file failed");
    }
    pImpl_.reset(new Impl(fp,sz,lruThreshold));
}

PagePtr Pager::getPage(PageNo_t _page){
    return pImpl_->getPage(_page);
}
PageNo_t Pager::pageCount()const{
    return pImpl_->pageCount();
}
Pager::~Pager(){
}
