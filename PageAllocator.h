#ifndef __PAGE_ALLOCATOR_H
#define __PAGE_ALLOCATOR_H
#include <string>
#include <memory>
#include "PagePtr.h"
class PageAllocator{
public:
    explicit PageAllocator(const std::string& fname);
    ~PageAllocator();
    PagePtr allocate();
    void deallocate( PagePtr &);
    void format();
    uint32_t maxGroup()const;
    bool formatted()const;
private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};
#endif// __PAGE_ALLOCATOR_H
