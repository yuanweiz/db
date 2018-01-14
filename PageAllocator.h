#ifndef __PAGE_ALLOCATOR_H
#define __PAGE_ALLOCATOR_H
#include <string>
#include <memory>
#include "PagePtr.h"
class PageAllocatorBase{
public:
    virtual ~PageAllocatorBase(){}
    virtual PagePtr allocate()=0;
    virtual PagePtr forceAllocate(size_t)=0;
    virtual void deallocate( PagePtr &)=0;
};
class PageAllocator :PageAllocatorBase{
public:
    explicit PageAllocator(const std::string& fname);
    ~PageAllocator() override;
    PagePtr allocate()override;
    void deallocate( PagePtr &)override;
    PagePtr forceAllocate(size_t) override;
    void format();
    uint32_t maxGroup()const;
    bool formatted()const;
private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};
#endif// __PAGE_ALLOCATOR_H
