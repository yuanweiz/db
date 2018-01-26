#ifndef __PAGE_ALLOCATOR_H
#define __PAGE_ALLOCATOR_H
#include <string>
#include <memory>
#include "PagePtr.h"
class PageAllocatorBase{
public:
    virtual ~PageAllocatorBase(){}
    virtual PagePtr allocate()=0;
    virtual PagePtr forceAllocate(PageNo_t)=0;
    virtual void deallocate( PagePtr &)=0;
    PagePtr getPage(PageNo_t); //guard, for PageNo_t=0 should return nullptr
protected:
    virtual PagePtr getPageImpl(PageNo_t)=0;
};
class PageAllocator :PageAllocatorBase{
public:
    explicit PageAllocator(const std::string& fname);
    ~PageAllocator() override;
    PagePtr allocate()override;
    void deallocate( PagePtr &)override;
    PagePtr forceAllocate(PageNo_t) override;
    void format();
    uint32_t maxGroup()const;
    bool formatted()const;
    PagePtr getPageImpl(PageNo_t)override;
private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};
#endif// __PAGE_ALLOCATOR_H
