#ifndef __BIT_VECTOR_H
#define __BIT_VECTOR_H
#include <inttypes.h>
#include <sys/types.h>
class BitVector{
public:
    explicit BitVector(void* ptr):
        ptr_(static_cast<uint8_t*>(ptr)){}
    bool get(size_t idx)const{
        auto byte = idx/8;
        auto offset = idx%8;
        return (ptr_[byte] & (1<<offset));
    }
    void set(size_t idx,bool value){
        auto byte = idx/8;
        auto offset = idx%8;
        if (value){
            uint32_t u32 = (1<<offset);
            ptr_[byte] |= static_cast<uint8_t>(u32);
        }
        else {
            uint32_t u32 = (255& (~(1<<offset)));
            ptr_[byte] &= static_cast<uint8_t>(u32);
        }
    }
private:
    uint8_t * ptr_;
};
#endif// __BIT_VECTOR_H
