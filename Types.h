#ifndef __TYPES_H
#define __TYPES_H
#include <inttypes.h>
#include <sys/types.h>

class PageSz_t {
public:
    PageSz_t()=default;
    PageSz_t(const PageSz_t&)=default;
    explicit PageSz_t(uint32_t v):v_(v){}
    operator uint32_t()const{
        return v_;
    }
private:
    uint32_t v_;
};
class PageNo_t {
public:
    PageNo_t()=default;
    PageNo_t(const PageNo_t&)=default;
    explicit PageNo_t(uint32_t v):v_(v){}
    operator uint32_t()const{
        return v_;
    }
private:
    uint32_t v_;
};
//using PageNo_t = uint32_t;
//using PageSz_t = uint32_t;
#endif// __TYPES_H
