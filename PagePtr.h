#ifndef __PAGE_PTR_H
#define __PAGE_PTR_H
#include <memory>
#include <functional>
#include <stdio.h>
#include "Types.h"

using EvictionCallback= 
std::function<void(void*,PageSz_t,PageNo_t)>;
class PageData;
using PagePtr=std::shared_ptr<PageData>;
class PageData{
public:
    static PagePtr fromFile(FILE* ,PageSz_t,PageNo_t);
    static PagePtr fromFile(FILE*, PageSz_t,PageNo_t,const EvictionCallback&);
    char *getNonConst();
    const char* getConst()const;
    void setDirty(bool b) {
        dirty = b;
    }
    PageNo_t pageNo() const noexcept{return pageNo_;}
    PageSz_t pageSize() const noexcept{return pageSize_;}
    static PageData* allocMemory(PageNo_t,PageSz_t); //for mock object
private:
    static PageData* allocMemory(FILE*,PageNo_t,PageSz_t);
    PageSz_t pageSize_;
    PageNo_t pageNo_;
    bool dirty;
    char buf[1];
};
#endif // __PAGE_PTR_H

