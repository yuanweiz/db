#include <detail/BTreeHelper.h>
#include <detail/DataPage.h>

namespace BTreeHelper{

template <class T>
T getView(PagePtr& ptr){
    return T(ptr->getNonConst(),ptr->pageSize());
}
template <class View>
    void distributeCells(View& left,View&right,StringView newTuple)
{
}
int getPageType(const PagePtr & ptr){
    auto * ptype = reinterpret_cast<const uint8_t*>(ptr->getConst());
    return *ptype;
}
template <class View>
uint16_t divide_point(View& pageView){
    size_t totalSz=0;
    auto nCells = pageView.numOfCells();
    decltype(nCells)i;
    for (i=0;i<nCells;++i){
        totalSz += pageView.getCell(i).size();
    }
    size_t target = totalSz/2; //"evenly" distributed
    for (i=0;i<nCells;++i){
        auto cellSz = pageView.getCell(i).size();
        if (target <= cellSz)break;
        else target-=cellSz;
    }
    return i;
}
template <class View>
uint16_t insert_point(View& pageView, StringView strView, const TupleComparator & cmp){
    auto nCells = pageView.numOfCells();
    decltype(nCells) i=0;
    for (;i<nCells;++i){
        auto view = pageView.getCell(i);
        bool result = cmp(view,strView);
        //if (useKeyComparator(pageView))
        //    result = keyCmp_(view,strView);
        //else 
        //    result = tupleCmp_(view,strView);
        if (result) 
            continue;
        else break;
    }
    return i;
}
template <typename Proxy>
    PagePtr findPageByTuple(Proxy&,StringView, const TupleComparator&,const KeyComparator&, const RetrieveKeyFunc&){
        // do some search here
        // using Proxy::ContainsRecord
    }
template <typename Proxy,bool = Proxy::HasParent> class Splitter;

template <typename Proxy> class Splitter<Proxy,true>{
    void split(Proxy& , StringView ,const TupleComparator&,const KeyComparator&, const RetrieveKeyFunc&){
        //allocate one page
        //copy half the data to new page
        //insert new record
        //reassign pointers
        //insert first key of right page to parent
    }
};

template <typename Proxy> class Splitter<Proxy,false>{
    template <class Child> void distributeCells(){
        auto leftPageNo = leftChild->pageNo();
        auto rightPageNo = rightChild->pageNo();
        for (j=0;j<i;++j){
            auto oldCell=rootView.getCell(j);
            void* dst=leftDataPage.allocCellAt(j,oldCell.size());
            assert(dst);
            ::memcpy(dst, oldCell.data(),oldCell.size());
        }
        for (j=i;j<nCells;++j){
            auto oldCell= rootView.getCell(j);
            void*dst= rightDataPage.allocCellAt(j-i,oldCell.size());
            assert(dst);
            ::memcpy(dst, oldCell.data(),oldCell.size());
        }
        leftDataPage.setNext(rightPageNo);
        leftDataPage.setParent(root_->pageNo());
        rightDataPage.setPrev(leftPageNo);
        rightDataPage.setParent(root_->pageNo());
    }
    void split(Proxy& rootView, StringView ,const TupleComparator&,const KeyComparator&, const RetrieveKeyFunc&){
    auto nCells = rootView.numOfCells();
    decltype(nCells)i=0, j=0;
    i=divide_point(rootView);
    assert(i<nCells);
    auto leftChildPage = pageAllocator_ ->allocate();
    auto rightChildPage = pageAllocator_ ->allocate();
    auto leftChild = PageProxy::fromPagePtr(leftChildPage);
    auto rightChild = PageProxy::fromPagePtr(rightChildPage);
    //TODO: maybe trait
    if (!Proxy::ContainsRecord){
        auto &left = *static_cast<InternalPage*>(leftChild);
        auto &right = *static_cast<InternalPage*>(rightChild);
        left.format();
        right.format();
        distributeCells(rootView,left,right);
    }
    else {
        auto &left = *static_cast<LeafPage*>(leftChild);
        auto &right = *static_cast<LeafPage*>(rightChild);
        left.format();
        right.format();
        distributeCells(rootView,left,right);
    }

    //auto leftDataPage = getView <DataPageView>(leftChild);
    //auto rightDataPage = getView <DataPageView>(rightChild);
    //leftDataPage.format();
    //rightDataPage.format(); //let ctor handle this?
    //sanity check
    assert(rightDataPage.numOfCells()+leftDataPage.numOfCells()
            == nCells);
    {
        auto tuple = rootView.getCell(i);
        auto key = Proxy::ContainsRecord? retrieveKey_(tuple):tuple;
        //FIXME: what if memory allocation is needed in retrieveKey?
        std::string buf=key;//make a copy and write back
        //in-place reset
        rootView.format();
        rootView.setHasChildren(true);
        char *leftPtr = static_cast<char*>(rootView.allocCellAt(0,4+buf.size()));
        ::memcpy(leftPtr,&leftPageNo,4);
        ::memcpy(leftPtr+4,buf.data(),buf.size());
        void* rightPtr = rootView.allocCellAt(1,4);
        ::memcpy(rightPtr, &rightPageNo, 4);
        LOG_DEBUG << "leftPageNo="<<leftPageNo << 
            "rightPageNo="<<rightPageNo;
    }
    }
};

template <typename Proxy>
     void insert(Proxy& p,StringView, const TupleComparator&,const KeyComparator&, const RetrieveKeyFunc&){
         if (Proxy::ContainsRecord){
             //... insert_point<Proxy>(...)
         }
         void * ptr=nullptr; // insertion here
         if (ptr){ // if allocation failed
             //split and insert into parent
             // auto middle = split_point(...);
             //Splitter<Proxy>().split(p, middle, ...);
         }
     }

struct RootPageWithRecord:public PageProxy,public detail::RootPageView{
    static const bool ContainsRecord=false;
    static const bool HasParent=false;

    virtual void insert(StringView, const TupleComparator&,const KeyComparator&, const RetrieveKeyFunc&) override{
    }
    virtual PagePtr findPageByTuple(StringView, const TupleComparator&,const KeyComparator&,const RetrieveKeyFunc&)override{
        //return findPageByTuple(*this,...);
    }
    
    
};

struct RootPageWithIndex: public PageProxy{
    static const bool ContainsRecord=true;
    static const bool HasParent=false;
    virtual void insert(StringView, const TupleComparator&,const KeyComparator&, const RetrieveKeyFunc&) override{
    }
    virtual PagePtr findPageByTuple(StringView, const TupleComparator&,const KeyComparator&,const RetrieveKeyFunc&)override{
        //return findPageByTuple(*this,...);
    }
};


struct InternalPage: public PageProxy{
    static const bool ContainsRecord=false;
    static const bool HasParent=true;
    virtual void insert(StringView, const TupleComparator&,const KeyComparator&, const RetrieveKeyFunc&) override{
    }
    virtual PagePtr findPageByTuple(StringView, const TupleComparator&,const KeyComparator&,const RetrieveKeyFunc&)override{
        //return findPageByTuple(*this,...);
    }
};

struct LeafPage: public PageProxy{
    static const bool ContainsRecord=false;
    static const bool HasParent=true;
    virtual void insert(StringView, const TupleComparator&,const KeyComparator&, const RetrieveKeyFunc&) override{
    }
    virtual PagePtr findPageByTuple(StringView, const TupleComparator&,const KeyComparator&,const RetrieveKeyFunc&)override{
        //return findPageByTuple(*this,...);
    }
};

}
