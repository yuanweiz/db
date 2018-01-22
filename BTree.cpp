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

//template <class View>
//iterator BTree::insertAndSplitPage(View& view, StringView newTuple){
iterator BTree::insertAndSplitPage(DataPageView& view, StringView newTuple){
    auto parent = view.parent();
    auto parentPage=pageAllocator_->getPage(PageNo_t(parent));
}

iterator BTree::insertAndSplitPage(RootPageView& view, StringView strView){
    split(view);
    return this->insert(strView);
}
template <class View> iterator BTree::insertIntoPage(PagePtr&pagePtr, StringView strView){
        auto view = getView<View>(pagePtr);
        assert(view.type() == View::Type);
        auto nCells = view.numOfCells();
        decltype(nCells)i=0;
        for (;i<nCells;++i){
            auto tuple = view.getCell(i);
            if (tupleCmp_(tuple,strView))
                continue;
            else break;
        }
        void* ptr=view.allocCellAt(i,strView.size());
        if (!ptr){
            return insertAndSplitPage(view,strView);
        }
        ::memcpy(ptr,strView.data(),strView.size());
        return iterator{i,pageAllocator_,root_};
}

int getPageType(const PagePtr & ptr){
    auto * ptype = reinterpret_cast<const uint8_t*>(ptr->getConst());
    return *ptype;
}

void BTree::split(RootPageView& rootView){
    size_t totalSz = 0;
    auto nCells = rootView.numOfCells();
    decltype(nCells)i=0, j=0;
    for (i=0;i<nCells;++i){
        totalSz += rootView.getCell(i).size();
    }
    size_t target = totalSz/2; //"evenly" distributed
    for (i=0;i<nCells;++i){
        auto cellSz = rootView.getCell(i).size();
        if (target <= cellSz)break;
        else target-=cellSz;
    }
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
iterator BTree::insert( RootPageView& rootView,StringView strView){
    assert(rootView.type() == RootPageView::TYPE);
    if (rootView.hasChildren()){
        assert(rootView.numOfCells()!=0);
        auto newKey = retrieveKey_(strView);
        auto nCells = rootView.numOfCells();
        decltype(nCells) i ;
        for ( i=0;i+1<nCells;++i){
            auto cell = rootView.getCell(i); 
            StringView key{cell.data()+4,cell.size()-4};
            if (keyCmp_ (newKey,key))
                break;
        }
        assert(i<nCells);
        auto cell = rootView.getCell(i);
        assert(cell.size()>=4);
        PageNo_t child;
        ::memcpy(&child,cell.data(),4);
        assert(child!=0);
        auto childPage = pageAllocator_->getPage(child);
        auto type = getPageType(childPage);
        switch (type){
            case RootPageView::TYPE:
                assert(false);
                return end();
            case DataPageView::TYPE:
            {
                auto dataPageView = getView<DataPageView>(childPage);
                return insert(dataPageView,strView);
            }
            case InternalPageView::TYPE:
            default:
            {
                throw Exception("not implemented: insert into internal page");
            }
        }
    }
    else {
        auto nCells = rootView.numOfCells();
        decltype(nCells)i=0;
        for (;i<nCells;++i){
            auto view = rootView.getCell(i);
            if (tupleCmp_(view,strView))
                continue;
            else break;
        }
        void* ptr=rootView.allocCellAt(i,strView.size());
        if (!ptr){
            split(rootView);
            return this->insert(strView);
        }
        ::memcpy(ptr,strView.data(),strView.size());
        return iterator{i,pageAllocator_,root_};
    }
}

//iterator insert( InternalPageView&,StringView){
//}
//iterator insert( InternalPageView&,StringView){
//}



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

iterator BTree::insert(StringView strView){
    auto rootView = getView<RootPageView>(root_);
    return insert(rootView,strView);
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

iterator& iterator::operator++(){
    auto type = getPageType(tie_);
    switch (type){
        case DataPageView::TYPE:
            {
                DataPageView view(tie_->getNonConst(), tie_->pageSize());
                if (++cellIdx_ == view.numOfCells()){
                    PageNo_t nextPage (view.next());
                    tie_ = pageAllocator_->getPage(nextPage);
                    cellIdx_=0;
                }
            }
            return *this;
        case RootPageView::TYPE:
            {
                RootPageView view(tie_->getNonConst(), tie_->pageSize());
                if (++cellIdx_ == view.numOfCells()){
                    tie_ = nullptr;
                    cellIdx_=0;
                }
            }
            return *this;
        default:
            throw Exception("unhandled case");
    }
}


iterator::iterator(uint16_t cellIdx, PageAllocatorBase* pageAllocator,const PagePtr& tie)
    :cellIdx_(cellIdx),pageAllocator_(pageAllocator), tie_(tie)
{
    if (!tie_)
        return;
    auto type = getPageType(tie_);
    switch (type){
        case RootPageView::TYPE:
            {
                RootPageView view(tie_->getNonConst(),tie_->pageSize());
                if (cellIdx_ == view.numOfCells()){
                    //no next page
                    tie_=nullptr;
                    cellIdx_ = 0;
                }
                else if (cellIdx_>view.numOfCells()){
                    throw Exception("");
                }
            }
            return;
        default:
            throw Exception("unhandled case");
    }
}


