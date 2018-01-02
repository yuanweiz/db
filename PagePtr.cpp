#include <functional>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "PagePtr.h"
#include "Types.h"
using namespace std;

PageData* PageData::allocMemory(FILE* fp,PageNo_t page,PageSz_t sz){
    auto oldPos = ::ftell(fp);
    auto offset = sz*page;
    ::fseek(fp,offset,SEEK_SET);
    size_t trunkSize = sizeof(PageData)+sz;
    auto rawPtr = static_cast<PageData*>(malloc(trunkSize));
    ::bzero(rawPtr, trunkSize);
    rawPtr->dirty = false;
    rawPtr->pageNo_ = page;
    rawPtr->pageSize_ = sz;
    //PagePtr ptr(page,sz,cb);
    char * p = rawPtr->buf;
    fread(p,1,sz,fp);
    //no need to append zeros
    ::fseek(fp,oldPos,SEEK_SET);
    return rawPtr;
}


PagePtr PageData::fromFile(FILE* fp, PageSz_t sz,PageNo_t page){
    auto rawPtr = allocMemory(fp,page,sz);
    PagePtr ptr(rawPtr);
    return ptr; //exception safety
}

PagePtr PageData::fromFile(FILE* fp, PageSz_t sz,PageNo_t page, const EvictionCallback&cb){
    auto rawPtr = allocMemory(fp,page,sz);
    auto deleter = [cb] (PageData* ptr){
        if (ptr->dirty){
            cb(ptr->buf,ptr->pageSize(),ptr->pageNo());
        }
        delete ptr;
    };
    PagePtr ptr(rawPtr,deleter);
    return ptr; //exception safety
}

const char* PageData::getConst()const{
    return buf;
}

char * PageData::getNonConst(){
    dirty = true;
    return buf;
}
