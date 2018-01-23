#include <BTree.h>
#include <PageAllocator.h>
#include <Exception.h>
#include <detail/DataPage.h>
#include <algorithm>
#include <string.h>
#include <Logging.h>
using iterator = BTree::iterator;
using detail::RootPageView;
using detail::DataPageView;
using detail::InternalPageView;


void BTree::split(RootPageView& rootView){
}


iterator BTree::insertAndSplitPage(RootPageView& view, PageNo_t,StringView newTuple, uint16_t){
    this->split(view);
    return this->insert(newTuple);
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
//template <class T>
//PagePtr BTree::findPageByKey(StringView strView, T& pageView){
//    auto newKey = retrieveKey_(strView);
//    auto nCells = pageView.numOfCells();
//    decltype (nCells) i;
//    for ( i=0;i+1<nCells;++i){
//        auto cell = pageView.getCell(i); 
//        StringView key{cell.data()+4,cell.size()-4};
//        if (keyCmp_ (newKey,key))
//            break;
//    }
//    assert(i<nCells);
//    auto cell = pageView.getCell(i);
//    assert(cell.size()>=4);
//    PageNo_t child;
//    ::memcpy(&child,cell.data(),4);
//    assert(child!=0);
//    auto childPage = pageAllocator_->getPage(child);
//    return findPageByTuple(strView,childPage);
//}
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
