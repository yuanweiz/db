#ifndef __BTREE_HELPER_H
#define __BTREE_HELPER_H
#include <StringView.h>
#include <PagePtr.h>
#include <BTree.h>
//#include <detail/DataPage.h>
#include <utility>
#include <Types.h>
#include <memory>
class PageAllocatorBase;
namespace detail{
struct PageProxy{
    using KeyComparator = BTree::KeyComparator;
    using TupleComparator = BTree::TupleComparator;
    using RetrieveKeyFunc = BTree::RetrieveKeyFunc;
    explicit PageProxy(PagePtr&);
    ~PageProxy(){}

    static std::unique_ptr<PageProxy> fromPagePtr(PagePtr&);
    virtual std::pair<uint16_t,PagePtr> insert(StringView, const TupleComparator&,
            const KeyComparator&,const RetrieveKeyFunc&, PageAllocatorBase*)=0;
    PagePtr pagePtr_;
    PageNo_t pageNo();
};
}


#endif //__BTREE_HELPER_H
