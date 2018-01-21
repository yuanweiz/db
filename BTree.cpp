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

int getPageType(const PagePtr & ptr){
    auto * ptype = reinterpret_cast<const uint16_t*>(ptr->getConst());
    return *ptype;
}

template <class T>
T getView(PagePtr& ptr){
    return T(ptr->getNonConst(),ptr->pageSize());
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

iterator BTree::insert(StringView strView){
    auto rootView = getView<RootPageView>(root_);
    if (rootView.hasChildren()){
        throw Exception("not implemented");
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
            throw Exception("not implemented");
        }
        ::memcpy(ptr,strView.data(),strView.size());
        return iterator{i,pageAllocator_,root_};
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


