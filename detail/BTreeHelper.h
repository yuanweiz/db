#ifndef __BTREE_HELPER_H
#define __BTREE_HELPER_H
#include <StringView.h>
#include <PagePtr.h>
#include <BTree.h>
#include <detail/DataPage.h>
#include <memory>
using TupleComparator = BTree::TupleComparator;
using KeyComparator = BTree::KeyComparator;
using RetrieveKeyFunc = BTree::RetrieveKeyFunc;
struct PageProxy{
    explicit PageProxy(PagePtr&);
    ~PageProxy(){}

    static std::unique_ptr<PageProxy> fromPagePtr(PagePtr&);
    virtual void insert(StringView, const TupleComparator&,
            const KeyComparator&,const RetrieveKeyFunc&);
    virtual PagePtr findPageByTuple(StringView, const TupleComparator&,const KeyComparator&,const RetrieveKeyFunc&);
    PagePtr pagePtr_;
};


#endif //__BTREE_HELPER_H
