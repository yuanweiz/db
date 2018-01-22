#ifndef __BTREE_H
#define __BTREE_H
#include <functional>
#include <Types.h>
#include <StringView.h>
#include <PagePtr.h>
class PageAllocatorBase;
namespace detail{
    class RootPageView;
    class InternalPageView;
    class DataPageView;
}
class BTree{
public:
    using TupleComparator = std::function<bool(StringView,StringView)>;
    using KeyComparator = std::function<bool(StringView,StringView)>;
    using RetrieveKeyFunc = std::function<StringView(StringView)>;
    BTree(PageNo_t root, PageAllocatorBase* ,const TupleComparator&, const KeyComparator&, const RetrieveKeyFunc& );
    class iterator{
        friend class BTree;
        using value_type=StringView;
        public:
        value_type operator*() const;
        iterator& operator++();
        bool operator!=(const iterator&);
        private:
        iterator(uint16_t, PageAllocatorBase*,const PagePtr&);
        uint16_t cellIdx_;
        PageAllocatorBase* pageAllocator_;
        PagePtr tie_;
    };
    iterator insert(StringView);
    iterator erase(iterator);
    iterator begin();
    iterator end();
private:
    iterator insert( detail::RootPageView& ,StringView );
    iterator insert( detail::DataPageView& ,StringView );
    template <class T> iterator insertIntoPage(PagePtr&,StringView);
    template <class View>
        iterator insertAndSplitPage(View& view, StringView newTuple);
    //special overload for root page
    iterator insertAndSplitPage(detail::RootPageView& view, StringView newTuple);
    iterator insertAndSplitPage(detail::DataPageView& view, StringView newTuple);
    void split( detail::RootPageView& );
    PageAllocatorBase* pageAllocator_;
    PagePtr root_;
    TupleComparator tupleCmp_;
    KeyComparator keyCmp_;
    RetrieveKeyFunc retrieveKey_;
};
#endif// __BTREE_H
