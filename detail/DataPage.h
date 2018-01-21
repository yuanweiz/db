#ifndef __DATAPAGE_H
#define __DATAPAGE_H
#include <inttypes.h>
#include <assert.h>
#include <PagePtr.h>
#include <StringView.h>
namespace detail{
struct DataPageHeader;
struct RootPageHeader;
struct InternalPageHeader;

template <typename Header,typename Parent>
class PageView{
public:
    explicit PageView(  void* data,size_t pageSz)
        :data_(static_cast<char*>(data)),
        pageSz_(pageSz)
    {
    }
    void format();

    void* allocCell(size_t ); //for unsorted file maybe?
    void* allocCellAt(size_t idx, size_t sz);
    void dropCell(size_t idx);
    uint16_t numOfCells()const;
    void sanityCheck();
    void dump();
    StringView getCell(size_t)const;
    uint32_t next();
protected:
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
class RootPageView: 
    public PageView<RootPageHeader,void>
{
};
class InternalPageView: 
    public PageView<InternalPageView,RootPageView>
{
};
class DataPageView : 
    public PageView<DataPageHeader,InternalPageView>
{
    using Base =PageView<DataPageHeader,InternalPageView>;
    using Base::Base;
};

}
#endif// __DATAPAGE_H
