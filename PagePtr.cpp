#include <functional>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "PagePtr.h"
#include "Types.h"
using namespace std;
using Callback=PagePtr::EvictionCallback;


PagePtr::~PagePtr(){
}

PagePtr::PagePtr(PageNo_t page,PageSz_t sz){
    size_t trunkSize = sizeof(PageData)+sz;
    auto rawPtr = static_cast<PageData*>(malloc(trunkSize));
    ::bzero(rawPtr, trunkSize);
    rawPtr->dirty = false;
    rawPtr->pageNo = page;
    rawPtr->pageSize = sz;
    ptr_.reset(rawPtr);
}

weak_ptr<PagePtr::PageData> PagePtr::getWeakPtr(){
    return ptr_;
}

PagePtr::PagePtr(PageNo_t page,PageSz_t sz,const EvictionCallback& cb){
    size_t trunkSize = sizeof(PageData)+sz;
    auto rawPtr = static_cast<PageData*>(malloc(trunkSize));
    ::bzero(rawPtr, trunkSize);
    rawPtr->dirty = false;
    rawPtr->pageNo = page;
    rawPtr->pageSize = sz;
    auto deleter = [cb] (PageData* ptr){
        if (ptr->dirty){
            cb(ptr->buf,ptr->pageSize,ptr->pageNo);
        }
        delete ptr;
    };
    ptr_.reset(rawPtr,deleter);
}

PagePtr PagePtr::fromFile(FILE* fp, PageSz_t sz,PageNo_t page){
    auto oldPos = ::ftell(fp);
    auto offset = sz*page;
    ::fseek(fp,offset,SEEK_SET);
    PagePtr ptr(page,sz);
    const char * cp = ptr.getConst();
    char * p = const_cast<char*>(cp);
    //auto ret = fread(p,1,sz,fp);
    fread(p,1,sz,fp); //discard return value
    //printf("fread return %lu, (%u~%lu)\n",ret,offset,offset+ret);
    //no need to append zeros
    ::fseek(fp,oldPos,SEEK_SET);
    return ptr; //exception safety
}

PagePtr PagePtr::fromFile(FILE* fp, PageSz_t sz,PageNo_t page, const EvictionCallback&cb){
    auto oldPos = ::ftell(fp);
    auto offset = sz*page;
    ::fseek(fp,offset,SEEK_SET);
    PagePtr ptr(page,sz,cb);
    const char * cp = ptr.getConst();
    char * p = const_cast<char*>(cp);
    fread(p,1,sz,fp);
    //no need to append zeros
    ::fseek(fp,oldPos,SEEK_SET);
    return ptr; //exception safety
}

const char* PagePtr::getConst()const{
    return ptr_->buf;
}

char * PagePtr::get(){
    ptr_->dirty = true;
    return ptr_->buf;
}
