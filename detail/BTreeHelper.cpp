#include <PageAllocator.h>
#include <detail/BTreeHelper.h>
#include <detail/DataPage.h>
#include <assert.h>
#include <string.h>

namespace detail{

    using KeyComparator = PageProxy::KeyComparator;
    using TupleComparator = PageProxy::TupleComparator;
    using RetrieveKeyFunc = PageProxy::RetrieveKeyFunc;
    using Result = std::pair<uint16_t,PagePtr>;

template <class T>
T getView(PagePtr& ptr){
    return T(ptr->getNonConst(),ptr->pageSize());
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

template <typename Proxy,bool = Proxy::HasParent> class Splitter;

template <typename Proxy> class Splitter<Proxy,false>{
    public:
    Result split(Proxy& rootView, StringView strView,
            const TupleComparator& tupleCmp,const KeyComparator&keyCmp, 
            const RetrieveKeyFunc& retrieveKey,PageAllocatorBase* allocator,bool insertTuple);
    Result insertTupleAndSplit(Proxy& rootView, StringView strView,
            const TupleComparator& tupleCmp, PageAllocatorBase* allocator);
    Result insertIndexAndSplit(Proxy& rootView, StringView strView,
            const KeyComparator& keyCmp, PageAllocatorBase* allocator);
    template <class Child> void setUpChildren(Proxy& root,Child& leftChild, Child& rightChild);
};
template <typename Proxy> class Splitter<Proxy,true>{
    public:
    Result split(Proxy& pageView, StringView strView,
            const TupleComparator& tupleCmp,const KeyComparator&keyCmp, 
            const RetrieveKeyFunc& retrieveKey,PageAllocatorBase* allocator,bool insertTuple);
};

template <class View>
uint16_t insert_point(View& pageView, StringView strView, const TupleComparator & cmp){
    auto nCells = pageView.numOfCells();
    decltype(nCells) i=0;
    for (;i<nCells;++i){
        auto view = pageView.getCell(i);
        bool result = cmp(view,strView);
        if (result) 
            continue;
        else break;
    }
    return i;
}

template <typename Proxy,bool = Proxy::ContainsRecord> struct Inserter;

template <typename Proxy> struct Inserter<Proxy,true>{
    Result insert(Proxy& proxy,StringView strView, const TupleComparator& tupleCmp,const KeyComparator& keyCmp, const RetrieveKeyFunc& retrieveKey,PageAllocatorBase*allocator) {
        StringView toBeInserted(nullptr,0);
        toBeInserted = strView;
        uint16_t i;
        i=insert_point(proxy,strView,tupleCmp);
        void * dst=proxy.allocCellAt(i,strView.size());
        if (!dst){
            return Splitter<Proxy>{}.split(proxy,strView,tupleCmp,keyCmp,retrieveKey,allocator,true);
        }
        else {
            ::memcpy(dst,toBeInserted.data(),toBeInserted.size());
            return {i,proxy.pagePtr_};
        }
    }
};
template <typename Proxy> struct Inserter<Proxy,false>{
    Result insert(Proxy& proxy,StringView strView, const TupleComparator& tupleCmp,const KeyComparator& keyCmp, const RetrieveKeyFunc& retrieveKey,PageAllocatorBase*allocator) {
        auto key = retrieveKey(strView);
        auto cmp = [&](StringView a, StringView b){
            assert(b.size()>=4);
            StringView key2(b.data()+4,b.size()-4);
            return keyCmp(a,key2);
        };
        auto nCells = proxy.numOfCells();
        auto i=nCells;
        for (i=0;i+1<nCells;++i){
            if (cmp(key,proxy.getCell(i)))
                break;
        }
        auto ptrAndKey = proxy.getCell(i);
        uint32_t pChild;
        ::memcpy(&pChild,ptrAndKey.data(), 4);
        auto pagePtr = allocator->getPage(PageNo_t(pChild));
        auto pProxy = PageProxy::fromPagePtr(pagePtr);
        return pProxy->insert(strView,tupleCmp,keyCmp,retrieveKey,allocator);
    }
};


template <typename T, bool _ContainsRecord,bool _HasParent >
struct PageProxyImpl: public PageProxy,public T{
    PageProxyImpl(PagePtr&pagePtr)
        :PageProxy(pagePtr),
        T(getView<T>(pagePtr))
    {
    }
    static const bool ContainsRecord = _ContainsRecord;
    static const bool HasParent = _HasParent;
    virtual Result insert(StringView strView, const TupleComparator& tupleCmp,const KeyComparator& keyCmp, const RetrieveKeyFunc& retrieveKey,PageAllocatorBase*allocator) override{
        return Inserter<PageProxyImpl>{}.insert(*this,strView,tupleCmp,keyCmp,retrieveKey,allocator);
    }
    virtual Result split(StringView strView, const TupleComparator& tupleCmp,const KeyComparator& keyCmp, const RetrieveKeyFunc& retrieveKey,PageAllocatorBase*allocator,bool insertTuple) override{
        return Splitter<PageProxyImpl>{}.split(*this,strView,tupleCmp,keyCmp,retrieveKey,allocator,insertTuple);
    }
};


//
//template <typename Proxy>
//     void insert(Proxy& p,StringView, const TupleComparator&,const KeyComparator&, const RetrieveKeyFunc&){
//         if (Proxy::ContainsRecord){
//             //... insert_point<Proxy>(...)
//         }
//         void * ptr=nullptr; // insertion here
//         if (ptr){ // if allocation failed
//             //split and insert into parent
//             // auto middle = split_point(...);
//             //Splitter<Proxy>().split(p, middle, ...);
//         }
//     }
//
//    static const bool ContainsRecord=false;
//    static const bool HasParent=false;
//    using Base = PageProxyImpl<RootPageView>;
//    using Base::Base;
//};

using RootPageWithRecord =  PageProxyImpl<RootPageView,true,false>;
using RootPageWithIndex =  PageProxyImpl<RootPageView,false,false>;
using InternalPage =  PageProxyImpl<InternalPageView,false,true>;
using DataPage = PageProxyImpl<DataPageView,true,true>;


PageProxy::PageProxy(PagePtr& pagePtr)
    :pagePtr_(pagePtr)
{
}

std::unique_ptr<PageProxy> PageProxy::fromPagePtr(PagePtr& pagePtr){
    auto * ptype = reinterpret_cast<const uint8_t*>(pagePtr->getConst());
    auto type = *ptype;
    std::unique_ptr<PageProxy> ret;
    switch(type){
        case RootPageView::TYPE:
        {
            auto view =getView<RootPageView>(pagePtr);
            if (view.hasChildren()){
                return std::unique_ptr<PageProxy>
                    (new RootPageWithIndex(pagePtr));
            }
            else {
                return std::unique_ptr<PageProxy>
                    (new RootPageWithRecord(pagePtr));
            }
        }
        case InternalPageView::TYPE:
        {
            return std::unique_ptr<PageProxy>
                (new InternalPage(pagePtr));
        }
        case DataPageView::TYPE:
        {
            return std::unique_ptr<PageProxy>
                (new DataPage(pagePtr));
        }
        default:return nullptr;
    }
}
template <class Proxy>
template <class Child> void Splitter<Proxy,false>::
setUpChildren(Proxy& root,Child& leftChild, Child& rightChild){
    auto i = divide_point(root);
    leftChild.format();
    rightChild.format();
    auto j=i;
    auto nCells = root.numOfCells();
    for (j=0;j<i;++j){
        auto oldCell=root.getCell(j);
        void* dst=leftChild.allocCellAt(j,oldCell.size());
        assert(dst);
        ::memcpy(dst, oldCell.data(),oldCell.size());
    }
    for (j=i;j<nCells;++j){
        auto oldCell= root.getCell(j);
        void*dst= rightChild.allocCellAt(j-i,oldCell.size());
        assert(dst);
        ::memcpy(dst, oldCell.data(),oldCell.size());
    }
    leftChild.setNext(rightChild.pageNo());
    leftChild.setParent(root.pageNo());
    rightChild.setPrev(leftChild.pageNo());
    rightChild.setParent(root.pageNo());
}
template <class Proxy>
Result Splitter<Proxy,false>::split(Proxy& root, StringView strView,
        const TupleComparator& tupleCmp,const KeyComparator&keyCmp, 
        const RetrieveKeyFunc& retrieveKey,PageAllocatorBase* allocator, bool insertTuple){
    auto leftChildPage = allocator->allocate();
    auto rightChildPage = allocator->allocate();
    std::string payload;
    //assert(insertTuple == Proxy::ContainsRecord);
    (void)insertTuple;
    if (Proxy::ContainsRecord){
        DataPage leftChild(leftChildPage);
        DataPage rightChild(rightChildPage);
        setUpChildren(root,leftChild,rightChild);
        payload = retrieveKey(rightChild.getCell(0));
    }
    else {
        InternalPage leftChild(leftChildPage);
        InternalPage rightChild(rightChildPage);
        setUpChildren(root,leftChild,rightChild);
        auto cell = rightChild.getCell(0);
        payload = std::string(cell.data()+4,cell.size()-4);
    }
    root.format();
    root.setHasChildren(true);
    {
        char * dst = static_cast<char*>(root.allocCellAt(0,payload.size()+4));
        uint32_t pLeft = leftChildPage->pageNo();
        ::memcpy(dst,&pLeft,4);
        ::memcpy(dst+4,payload.data(),payload.size());
    }
    {
        char * dst = static_cast<char*>(root.allocCellAt(1,4));
        uint32_t pRight = rightChildPage->pageNo();
        ::memcpy(dst,&pRight,4);
    }
    auto newRoot = PageProxy::fromPagePtr(root.pagePtr_);
    return newRoot->insert(strView,tupleCmp,keyCmp,retrieveKey,allocator);
}
template <typename Proxy>
Result Splitter<Proxy,true>::split(Proxy& pageView, StringView strView,
        const TupleComparator& tupleCmp,const KeyComparator&keyCmp, 
        const RetrieveKeyFunc& retrieveKey,PageAllocatorBase* allocator,bool ){
    auto i=divide_point(pageView);
    auto newPage = allocator->allocate();
    Proxy sibling(newPage);
    sibling.format();
    //transfer cells to sibling
    decltype(i) j =0;
    while(pageView.numOfCells()!=i){
        auto cell = pageView.getCell(i);
        void * dst = sibling.allocCellAt(j++,cell.size());
        ::memcpy(dst,cell.data(),cell.size());
        pageView.dropCell(i);
    }
    sibling.setNext(pageView.next());
    sibling.setPrev(pageView.pageNo());
    sibling.setParent(pageView.parent());
    pageView.setNext(sibling.pageNo());
    PageNo_t parent (pageView.parent());
    auto parentPage = allocator->getPage(parent);
    auto parentProxy = PageProxy::fromPagePtr(parentPage);
    parentProxy->split(strView,tupleCmp,keyCmp,retrieveKey,allocator,Proxy::ContainsRecord);
    return {0,0};
}
template <typename Proxy>
Result Splitter<Proxy,false>::insertIndexAndSplit(Proxy& rootView, StringView strView,
        const KeyComparator& keyCmp, PageAllocatorBase* allocator){
}
template <typename Proxy>
Result Splitter<Proxy,false>::insertTupleAndSplit(Proxy& rootView, StringView strView,
        const TupleComparator& keyCmp, PageAllocatorBase* allocator){
}

//template <class Proxy>
//Result Splitter<Proxy,true>::split(Proxy& root, StringView strView,
//        const TupleComparator& tupleCmp,const KeyComparator&keyCmp, 
//        const RetrieveKeyFunc& retrieveKey,PageAllocatorBase* allocator, int16_t){
//    return {0,0};
//}

PageNo_t PageProxy::pageNo(){
    return pagePtr_->pageNo();
}

}
