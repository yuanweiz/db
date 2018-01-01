#include <unordered_map>
#include <assert.h>
#include "Pager.h"
#include "PagePtr.h"
#include "Exception.h"
using namespace std;
class Pager::Impl{
public:
    Impl(FILE* fp, PageSz_t sz)
        :pageSz_(sz),fp_(fp,&fclose)
    {
    }
    PagePtr getPage(PageNo_t _page){
        uint32_t page = _page;
        auto mapIter = map_.find(page);
        if (mapIter!=map_.end()){
            return mapIter->second;
        }
        auto ptr= PagePtr::fromFile(fp_.get(), pageSz_,_page);
        auto ret = map_.insert({page,ptr});
        assert(ret.second);(void)ret;
        return ptr;
    }
private:
    //dumb implementation, never evicts
    PageSz_t pageSz_;
    std::unique_ptr<FILE,decltype(&fclose)> fp_;
    std::unordered_map<uint32_t,PagePtr> map_;
};

Pager::Pager(const string& fname, PageSz_t sz){
    FILE* fp = fopen(fname.c_str(), "r+");
    if (!fp){
        throw Exception("open file failed");
    }
    pImpl_.reset(new Impl(fp,sz));
}

PagePtr Pager::getPage(PageNo_t _page){
    return pImpl_->getPage(_page);
}
Pager::~Pager(){
}
