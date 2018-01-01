#ifndef __PAGE_PTR_H
#define __PAGE_PTR_H
#include <memory>
#include <functional>
#include <stdio.h>
#include "Types.h"
class PagePtr{
public: 
    //legacy code with design flaws
    class PageData{
        public:
            PageSz_t pageSize;
            PageNo_t pageNo;
            bool dirty;
            char buf[1];
    };
    PagePtr()=default;
    PagePtr(const std::shared_ptr<PageData>&ptr):ptr_(ptr){}
    using EvictionCallback= 
        std::function<void(void*,PageSz_t,PageNo_t)>;
    PagePtr(const PagePtr&)=default;
    static PagePtr fromFile(FILE*, PageSz_t,PageNo_t);
    static PagePtr fromFile(FILE*, PageSz_t,PageNo_t,const EvictionCallback&);
    char* get();
    const char* getConst()const;
    bool operator==(const PagePtr&rhs){return ptr_==rhs.ptr_;}
    bool operator!=(const PagePtr&rhs){return ptr_!=rhs.ptr_;}
    std::weak_ptr<PageData> getWeakPtr();
    ~PagePtr();
    operator bool() const noexcept {return ptr_.operator bool();}
    PageNo_t pageNo() const noexcept{return ptr_->pageNo;}
private:
    PagePtr(PageNo_t page,PageSz_t sz);
    PagePtr(PageNo_t page,PageSz_t sz, const EvictionCallback&);
    std::shared_ptr<PageData> ptr_;
};
#endif // __PAGE_PTR_H

