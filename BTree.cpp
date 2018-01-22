#include <BTree.h>
#include <PageAllocator.h>
#include <Exception.h>
#include <detail/DataPage.h>
#include <algorithm>
#include <string.h>
using iterator = BTree::iterator;
using detail::RootPageView;
using detail::DataPageView;
using detail::InternalPageView;

template <class T>
T getView(PagePtr& ptr){
    return T(ptr->getNonConst(),ptr->pageSize());
}

template <class T>
struct has_parent{
    const static bool value=false;
};
template <>
struct has_parent<DataPageView>{
    const static bool value=true;
};
template <>
struct has_parent<InternalPageView>{
    const static bool value=true;
};

bool useKeyComparator(RootPageView& view){
    return view.hasChildren();
}

bool useKeyComparator(InternalPageView&){
    return false;
}

bool useKeyComparator(DataPageView&){
    return true;
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
uint16_t BTree::insert_point(View& pageView, StringView strView){
    auto nCells = pageView.numOfCells();
    decltype(nCells) i=0;
    for (;i<nCells;++i){
        auto view = pageView.getCell(i);
        bool result;
        if (useKeyComparator(pageView))
            result = keyCmp_(view,strView);
        else 
            result = tupleCmp_(view,strView);
        if (result) 
            continue;
        else break;
    }
    return i;
}
int getPageType(const PagePtr & ptr){
    auto * ptype = reinterpret_cast<const uint8_t*>(ptr->getConst());
    return *ptype;
}

void BTree::split(RootPageView& rootView){
    auto nCells = rootView.numOfCells();
    decltype(nCells)i=0, j=0;
    i=divide_point(rootView);
    assert(i<nCells);
    auto leftChild = pageAllocator_ ->allocate();
    auto leftDataPage = getView <DataPageView>(leftChild);
    auto rightChild = pageAllocator_ ->allocate();
    auto rightDataPage = getView <DataPageView>(rightChild);
    leftDataPage.format();
    rightDataPage.format();
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
    //sanity check
    assert(rightDataPage.numOfCells()+leftDataPage.numOfCells()
            == nCells);
    {
        auto tuple = rootView.getCell(i);
        auto key = retrieveKey_(tuple);
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
    }
}


iterator BTree::insertAndSplitPage(RootPageView& view, PageNo_t,StringView newTuple, uint16_t){
    this->split(view);
    return this->insert(newTuple);
}

template <class View>
iterator BTree::insertAndSplitPage(View& pageView, 
        PageNo_t pageNo,StringView newEntry, uint16_t insertPoint){
    auto cmp= useKeyComparator(pageView)? keyCmp_: tupleCmp_;
    auto nCells = pageView.numOfCells();
    auto rightChildPage = pageAllocator_->allocate();
    auto rightChildPageView = getView<View>(rightChildPage);
    uint16_t i=divide_point(pageView);
    auto j=i;
    for (; j<nCells;++j){
        auto tuple = pageView.getCell(j);
        void * dst= rightChildPageView.allocCellAt( j-i, tuple.size());
        ::memcpy(dst,tuple.data(),tuple.size());
    }
    for (j=0;j<nCells;++j){
        pageView.dropCell(i);
    }
    assert(pageView.numOfCells() + rightChildPageView.numOfCells()== nCells);
    //insert to left or right
    
    PagePtr retPtr;
    uint16_t retIdx;
    {
        void*dst;
        if (insertPoint>=i){
            retIdx = static_cast<uint16_t>(
                    static_cast<int>(insertPoint)-i);
            static_assert(sizeof(retIdx)==sizeof(insertPoint),"");
            static_assert(sizeof(retIdx)==sizeof(i),"");
            retPtr = rightChildPage;
            dst = rightChildPageView.allocCellAt(insertPoint-i,newEntry.size());
        }
        else {
            retIdx= insertPoint;
            retPtr = pageAllocator_->getPage(pageNo);
            dst = pageView.allocCellAt(insertPoint,newEntry.size());
        }
        ::memcpy(dst,newEntry.data(),newEntry.size());
    }
    auto key = retrieveKey_(rightChildPageView.getCell(0));
    auto newPageNo = rightChildPage->pageNo();
    std::string buf(4+key.size(),'\0');
    if(!useKeyComparator(pageView)){
        //parent must use key comparator
        ::memcpy(&*buf.begin(), &newPageNo,4);
        ::memcpy(&*buf.begin()+4, key.data(),key.size());
    }
    else {
        buf = newEntry;
    }

    PageNo_t parent (pageView.parent());
    rightChildPageView.setNext( pageView.next());
    rightChildPageView.setPrev( pageNo);
    pageView.setNext( rightChildPage->pageNo());
    rightChildPageView.setParent(parent);
    auto parentPage = pageAllocator_->getPage(parent);
    auto type = getPageType(parentPage);
    switch (type){
        case RootPageView::TYPE:
        {
            auto view = getView<RootPageView>(parentPage);
            insert(view,buf,pageNo);
            break;
        }
        case InternalPageView::TYPE:
        {
            auto view = getView<InternalPageView>(parentPage);
            insert(view,buf,pageNo);
            break;
        }
        default:
        throw Exception("");
    }
    return iterator{retIdx,nullptr,retPtr};
}
template <class View>
iterator BTree::insert( View& pageView,StringView strView,PageNo_t pageNo){
    auto i = insert_point(pageView,strView);
    void* ptr=pageView.allocCellAt(i,strView.size());
    if (!ptr){
        return insertAndSplitPage(pageView,pageNo, strView,i);
    }
    ::memcpy(ptr,strView.data(),strView.size());
    return iterator{i,pageAllocator_,
        pageAllocator_->getPage(pageNo)};
}

BTree::BTree(PageNo_t root, 
        PageAllocatorBase* allocator,
        const TupleComparator& tupleCmp,
        const KeyComparator& keyCmp,
        const RetrieveKeyFunc& retrieveKey)
    :pageAllocator_(allocator),
    root_(allocator->getPage(root)),
    tupleCmp_(tupleCmp),
    keyCmp_(keyCmp),
    retrieveKey_(retrieveKey)
{
    //force construct
    detail::RootPageView rootView(root_->getNonConst(), root_->pageSize());
    rootView.format();
}

iterator BTree::begin(){
    auto rootView = getView<RootPageView>(root_);
    if (rootView.hasChildren()){
        assert(rootView.numOfCells()!=0);
        throw Exception("not implemented branch");
    }
    else {
        return iterator{0,pageAllocator_,root_};
    }
}
iterator BTree::end(){
    return iterator{0,nullptr,nullptr};
}
template <class T>
PagePtr BTree::findPageByKey(StringView strView, T& pageView){
    auto newKey = retrieveKey_(strView);
    auto nCells = pageView.numOfCells();
    decltype (nCells) i;
    for ( i=0;i+1<nCells;++i){
        auto cell = pageView.getCell(i); 
        StringView key{cell.data()+4,cell.size()-4};
        if (keyCmp_ (newKey,key))
            break;
    }
    assert(i<nCells);
    auto cell = pageView.getCell(i);
    assert(cell.size()>=4);
    PageNo_t child;
    ::memcpy(&child,cell.data(),4);
    assert(child!=0);
    auto childPage = pageAllocator_->getPage(child);
    return findPageByTuple(strView,childPage);
}
PagePtr BTree::findPageByTuple(StringView strView,  PagePtr& start){
    auto type = getPageType(start);
    switch (type){
        case RootPageView::TYPE:
        {
            auto rootView = getView<RootPageView>(start);
            if (!rootView.hasChildren())
                return start;
            return findPageByKey(strView,rootView);
        }
        case DataPageView::TYPE:
        {
            return start;
        }
        case InternalPageView::TYPE:
        {
            auto internalPageView = getView<InternalPageView>
                (start);
            return findPageByKey(strView,internalPageView);
        }
        default:
        {
            throw Exception("not implemented");
        }
    }
}

iterator BTree::insert(StringView strView){
    auto pagePtr = findPageByTuple(strView,root_);
    auto type=getPageType(pagePtr);
    switch (type){
        case RootPageView::TYPE:
        {
            auto view = getView<RootPageView>(pagePtr);
            assert(!view.hasChildren());
            return insert(view,strView,pagePtr->pageNo());
        }
        break;
        case DataPageView::TYPE:
        {
            auto view = getView<DataPageView>(pagePtr);
            return insert(view,strView,pagePtr->pageNo());
        }
        break;
        default:
            throw Exception("Page Corruption");
    }
}

bool iterator::operator!=(const iterator& rhs){
    return cellIdx_ != rhs.cellIdx_ || tie_ != rhs.tie_;
};

StringView iterator::operator*()const {
    auto type = getPageType(tie_);
    switch (type){
        case DataPageView::TYPE:
            {
            DataPageView view(tie_->getNonConst(), tie_->pageSize());
            return view.getCell(cellIdx_);
            }
        case RootPageView::TYPE:
            {
            RootPageView view(tie_->getNonConst(), tie_->pageSize());
            return view.getCell(cellIdx_);
            }
        default:
            throw Exception("unhandled case");
    }
}

void iterator::rewindIfOverflow(){
    auto type = getPageType(tie_);
    switch (type){
        case DataPageView::TYPE:
            {
                DataPageView view(tie_->getNonConst(), tie_->pageSize());
                if (cellIdx_ == view.numOfCells()){
                    PageNo_t nextPage (view.next());
                    tie_ = pageAllocator_->getPage(nextPage);
                    cellIdx_=0;
                }
            }
            return ;
        case RootPageView::TYPE:
            {
                RootPageView view(tie_->getNonConst(), tie_->pageSize());
                if (cellIdx_ == view.numOfCells()){
                    tie_ = nullptr;
                    cellIdx_=0;
                }
            }
            return ;
        default:
            throw Exception("unhandled case");
    }
}
iterator& iterator::operator++(){
    ++cellIdx_;
    rewindIfOverflow();
    return *this;
}


iterator::iterator(uint16_t cellIdx, PageAllocatorBase* pageAllocator,const PagePtr& tie)
    :cellIdx_(cellIdx),pageAllocator_(pageAllocator), tie_(tie)
{
    if (!tie_){
        cellIdx_=0;
        return;
    }
    rewindIfOverflow();
}
