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
    bool hasChildren()const; // for root page it can be true/false, otherwise false
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
    public:
    static constexpr uint8_t TYPE=1;
    void setHasChildren(bool);
    using Base = PageView<RootPageHeader,void>;
    using Base::Base;
};
class InternalPageView: 
    public PageView<InternalPageView,RootPageView>
{
    public:
    static constexpr uint8_t TYPE=2;
};
class DataPageView : 
    public PageView<DataPageHeader,InternalPageView>
{
public:
    static constexpr uint8_t TYPE=3;
    using Base =PageView<DataPageHeader,InternalPageView>;
    using Base::Base;
    uint32_t prev()const;
    uint32_t next()const;
};

}
#endif// __DATAPAGE_H
