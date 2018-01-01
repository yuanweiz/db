#ifndef __PAGE_PTR_H
#define __PAGE_PTR_H
#include <memory>
#include <functional>
#include <stdio.h>
#include "Types.h"
class PagePtr{
public: 
    using EvictionCallback= 
        std::function<void(void*,PageSz_t,PageNo_t)>;
    PagePtr(const PagePtr&)=default;
    static PagePtr fromFile(FILE*, PageSz_t,PageNo_t);
    static PagePtr fromFile(FILE*, PageSz_t,PageNo_t,const EvictionCallback&);
    char* get();
    const char* getConst()const;
    ~PagePtr();
    operator bool() const noexcept {return ptr_.operator bool();}
private:
    class PageData;
    PagePtr(const std::shared_ptr<PageData>&);
    PagePtr(PageNo_t page,PageSz_t sz);
    PagePtr(PageNo_t page,PageSz_t sz, const EvictionCallback&);
    std::shared_ptr<PageData> ptr_;
};
#endif // __PAGE_PTR_H

