#ifndef __PAGE_CACHE_H
#define __PAGE_CACHE_H
#include <memory>
#include <string>
#include <stdio.h>
#include "Types.h"
#include "PagePtr.h"
class Pager{
public:
    Pager(const std::string &fname, PageSz_t ,size_t lruThreshold=0);
    PagePtr getPage(PageNo_t);
    ~Pager();
private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

#endif// __PAGE_CACHE_H
