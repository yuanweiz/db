#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <detail/DataPage.h>
#include <Logging.h>
#include <Exception.h>
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
namespace detail{
    struct Header{
        uint16_t type;
        uint16_t freeList;
        uint32_t next;
        uint16_t nFragment; // number of free bytes
        uint16_t nCells;
        uint16_t cells[0];
    private:
        Header(){} //non constructable
    };
    struct FreeBlock{
        uint16_t next; 
        uint16_t size; // non-inclusive
        char data[0];
    private:
        FreeBlock(){}
    };
    struct Cell{
        uint16_t capacity;
        uint16_t size;
        char data[0];
    private:
        Cell(){}
    };
    static_assert( sizeof (Header) == 12, "wrong memory layout");
    static_assert( sizeof (FreeBlock) == 4, "wrong memory layout");
    static_assert( sizeof (Cell) == 4, "wrong memory layout");

    uint16_t DataPageView::numOfCells()const{
        return header().nCells;
    }
    struct DataPageView::FreeBlockIterator{
        using Self = FreeBlockIterator;
        explicit FreeBlockIterator(DataPageView * view)
            :view_(*view)
        {
            ptr_ = view_.header().freeList;
        }
        FreeBlockIterator(const Self&rhs)=default;
        FreeBlockIterator(DataPageView * view, int)
            :view_(*view)
        {
            ptr_ = 0;
        }
        FreeBlock& operator*()const {
            return *view_.view_cast<FreeBlock*>
                (view_.data_+ptr_);
        }
        Self & operator++(){
            auto * data = view_.data_;
            auto * pFreeBlock = view_.view_cast
                <FreeBlock*>(data+ptr_);
            ptr_ = pFreeBlock->next;
            return *this;
        }
        bool operator==(const Self& rhs){
            return &view_ == &rhs.view_ && ptr_ == rhs.ptr_;
        }
        bool operator!=(const Self&rhs){
            return !(*this == rhs);
        }
        DataPageView& view_;
        uint16_t ptr_;
    };
    void DataPageView::dump(){
        uint16_t* ptr = (uint16_t*)(data_+ header().freeList);
        uint16_t bound = 0;
        while(ptr!=(uint16_t*)data_){
            if ((char*)ptr > data_+pageSz_){
                printf("error: offset %lu is out of range\n", (char*)ptr-data_);
                assert(false);
                break;
            }
            uint16_t offset = offset_of(ptr);
            auto * freeBlock = (FreeBlock*)ptr;
            assert(offset > bound);
            printf("freeblock = {header:[%d,%lu),data:[%d,%d)}\n", 
                    offset, offset+sizeof(FreeBlock),
                    offset_of(freeBlock->data), offset_of(freeBlock->data)+(freeBlock->size)
                    );
            ptr = (uint16_t*)(data_+*ptr);
            bound = offset + freeBlock->size;
        }
        size_t nCells = header().nCells;
        uint16_t * cells =  header().cells;
        for (size_t i=0;i<nCells;++i){
            uint16_t offset = cells[i];
            if (offset > pageSz_){
                printf("error: cells[%lu]=%d is out of range\n",i,offset);
                break;
            }
            auto* pCell = (Cell*) (data_+offset);
            printf("cells[%lu]={size=%d,header=%d,data=(%d,%d)}\n"
                    ,i,pCell->size,offset_of(pCell),offset_of(pCell->data),
                    offset_of(pCell->data)+pCell->size);
            //auto * pCell = view_cast<Cell*>(data_+offset);
        }
    }
    void* DataPageView::allocCell(size_t sz){
        auto & h = header();
        bool isTop = false;
        uint16_t * curr = (uint16_t*)(data_+h.freeList);
        uint16_t* prev = & h.freeList;
        uint16_t* top = h.cells + h.nCells;
        auto advance = [this](uint16_t *&ptr){
            ptr = (uint16_t*)(data_ + *ptr);
        };
        for (; curr!= (void*)data_; prev= curr, advance(curr)){
            auto * pFreeBlock = view_cast<FreeBlock*>(curr);
            if (pFreeBlock->size <= sz)continue;
            if (curr == top){
                if ( pFreeBlock-> size <= sz + 2){
                    continue;
                }
                else {
                    isTop = true;
                }
            }
            break;
        }
        if (curr==(uint16_t*)data_ )return nullptr;
        if (isTop){
            ::memmove(curr+1,curr,4); 
            curr++;
            *prev +=2;
            auto *pFreeBlock = (FreeBlock*)curr;
            pFreeBlock->size-=2;
            //fall through
        }
        auto *pFreeBlock = (FreeBlock*)curr;
        auto cellSz = pFreeBlock->size;
        int fragment=0;
        if (cellSz < sz+4){
            //assign it
            fragment = sz+4-cellSz;
            *prev = *curr;
            *top = (char*)curr - data_;
            h.nCells++;
        }
        else {
            //split this chunk into two parts, no need to modify prev pointer
            // first part is not moved ,just return the second part
            uint16_t newSize =  pFreeBlock->size - sz - 4;
            pFreeBlock->size = newSize;
            *top = pFreeBlock->data + newSize - data_;
            h.nCells++;
        }
        auto *pCell = (Cell*)(data_+*top);
        pCell->size = sz;
        pCell->capacity = sz+fragment;
        return pCell->data;
    }
    void DataPageView::dropCell(size_t idx){
        auto & h = header();
        if (idx>=h.nCells){
            throw Exception("out of range");
        }
        auto * top = data_ + h.freeList;
        assert( top==(char*)( h.cells+h.nCells));(void)top;
        //auto* pFirstBlock = (FreeBlock*)( data_+h.freeList);
        h.nCells--;
        h.freeList-=2;
        uint16_t cellPtr = h.cells[idx];
        //move cells and first freeblock backward
        ::memmove(h.cells+idx, h.cells+idx+1,sizeof(uint16_t)*(h.nCells-idx+2));

        {
            uint16_t* curr = (uint16_t*)(data_+h.freeList);
            auto * firstFreeBlock = (FreeBlock*)curr;
            firstFreeBlock->size +=2;
        }
        {
        auto * pCell = view_cast<Cell*>(data_+cellPtr);
        auto * pFreeBlock = view_cast<FreeBlock*>(data_+cellPtr);
        pFreeBlock->size = pCell->size; //can be removed?
        }
        auto * newFreeBlock = (FreeBlock*) (data_+cellPtr);
        uint16_t* prev = &h.freeList;
        uint16_t* curr = (uint16_t*)(data_+h.freeList);
        for (; (void*)curr != data_; prev= curr, curr= (uint16_t*)(data_+*curr)){
            LOG_DEBUG << "offset_of curr:" << offset_of(curr);
            if ((void*)curr < newFreeBlock)continue;
            else break;
        }
        auto * prevFreeBlock = (FreeBlock*)prev;
        auto * currFreeBlock = (FreeBlock*)curr;
        prevFreeBlock->next = (char*)newFreeBlock - data_;
        newFreeBlock->next = (char*)currFreeBlock - data_;
        LOG_DEBUG << "prevFreeBlock = " << offset_of(prevFreeBlock);
        LOG_DEBUG << "newFreeBlock = " << offset_of(newFreeBlock);
        LOG_DEBUG << "currFreeBlock = " << offset_of(currFreeBlock);
        if ((char*)curr!=data_ &&
                newFreeBlock->data + newFreeBlock->size == (void*)curr){
            newFreeBlock->next = currFreeBlock->next;
            newFreeBlock->size += (4+currFreeBlock->size);
        }
        if (prev!=&h.freeList && 
                prevFreeBlock->data+ prevFreeBlock->size == (void*) newFreeBlock){
            prevFreeBlock->next = newFreeBlock->next;
            prevFreeBlock->size += (4+newFreeBlock->size);
        }
    }
    StringView DataPageView::getCell(size_t sz)const {
        auto & h = header();
        if (sz >= h.nCells){
            throw Exception("index out of range");
        }
        auto* pCell = (Cell*)data_ + h.cells[sz];
        return StringView{pCell->data,pCell->size};
    }
    void DataPageView::sanityCheck()
    {
        enum ErrorType{Overlap,Missing,OutOfRange};
        struct Error{
            ErrorType type;
            int start;
            int end;
            Error(){}
            Error(ErrorType t,int s,int e)
                :type(t),start(s),end(e)
            {
            }
        };
        std::vector<std::pair<int,int>> ranges;
        std::vector<Error> errors;
        auto & h = header();
        int headerSize = offset_of(h.cells) + sizeof(uint16_t)*h.nCells;
        ranges.push_back({0,headerSize});
        uint16_t* ptr = (uint16_t*)(data_+h.freeList);
        while((void*)ptr!=data_){
            auto * pFreeBlock = (FreeBlock*) ptr;
            //TODO: violation error checks
            auto start = offset_of(pFreeBlock);
            auto end = start+ pFreeBlock->size + sizeof(FreeBlock);
            ranges.push_back({start,end});
            ptr = (uint16_t*)(data_ + *ptr);
        }
        auto nCells = h.nCells;
        for (size_t i=0;i<nCells;++i){
            auto * pCell = (Cell*)(data_ + h.cells[i]);
            auto start = offset_of(pCell);
            auto end = start + pCell->capacity + sizeof(Cell);
            ranges.push_back({start,end});
        }
        int max_ = 0;
        std::sort(ranges.begin(),ranges.end());
        for (auto range : ranges){
            LOG_DEBUG << "range: ("<<range.first <<","<< range.second<<")";
        }
        for (auto pr: ranges){
            if (pr.first > max_){
                //a "hole" in the address space
                errors.push_back({Missing,max_, pr.first});
            }
            else if (pr.first < max_ ){
                errors.push_back({Overlap, pr.first,max_});
            }
            max_ = std::max(max_,pr.second);
        }
        if (errors.empty())
            return;
        LOG_DEBUG << errors.size() << " errors found";
        for (auto & error:errors){
            switch (error.type){
                case ErrorType::Missing:
                    {
                        LOG_DEBUG << "Can't track block [" << error.start 
                            << "," << error.end << ")";
                        break;
                    }
                case ErrorType::Overlap:
                    {
                        LOG_DEBUG << "Overlap [" << error.start 
                            << "," << error.end << ")";
                        break;
                    }
                default:;
            }
        }
        throw Exception("sanityCheck failed");
    }
    void DataPageView::format()
    {
        //::bzero(&header(), sizeof(Header));
        ::bzero(data_,pageSz_);
        header().type = 1;
        header().freeList = sizeof(Header);
        auto * pFreeBlock = view_cast<FreeBlock*>(data_+header().freeList);
        pFreeBlock->next = 0;
        pFreeBlock->size = pageSz_ - sizeof(Header)- sizeof(FreeBlock);
    }
}
