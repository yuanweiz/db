#ifndef __DATAPAGE_H
#define __DATAPAGE_H
#include <inttypes.h>
#include <assert.h>
#include <PagePtr.h>
#include <StringView.h>
namespace detail{
struct Header;
class DataPageView{
public:
    //explicit DataPageView( PagePtr& pagePtr)
    //    :data_(pagePtr->getNonConst()),
    //    pageSz_(pagePtr->pageSize())
    explicit DataPageView(  void* data,size_t pageSz)
        :data_(static_cast<char*>(data)),
        pageSz_(pageSz)
    {
    }
    void format();

    void* allocCell(size_t );
    void dropCell(size_t idx);
    uint16_t numOfCells()const;
    void sanityCheck();
    void dump();
private:
    void checkAccessViolation(uint16_t ptr){
        assert(ptr < pageSz_);(void)ptr;
    }
    void checkAccessViolation(void* ptr){
        assert(ptr < data_ + pageSz_);(void)ptr;
    }
    void checkAccessViolation(uint16_t ptr,uint16_t sz){
        assert(ptr < pageSz_);(void)ptr;
        assert(ptr + sz <= pageSz_); (void)sz;
    }
    StringView getCell(size_t)const;
    struct FreeBlockIterator;
    template <class T>
        T view_cast(void * ptr){
            return reinterpret_cast<T>(ptr);
        }
    
    Header& header(){
        return *reinterpret_cast<Header*>(data_);
    }
    void* pointerAt(size_t sz){
        return data_+sz;
    }
    uint16_t offset_of(const void* ptr)const{
        return static_cast<uint16_t>(static_cast<const char*>(ptr) - data_);
    }
    const Header& header()const{
        return *reinterpret_cast<const Header*>(data_);
    }
    char * data_;
    const size_t pageSz_;
};
}
#endif// __DATAPAGE_H
