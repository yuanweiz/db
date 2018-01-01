#include <vector>
#include <gtest/gtest.h>
#include "Pager.h"

class PagerTest: public ::testing::Test{
    public:
    PagerTest(){
    }
    void SetUp()override{
        auto fp=fopen("/tmp/empty","w");
        fclose(fp); //now it is empty
    }
    void fillRandomData(size_t sz){
        randomData.resize(sz); // 4k trunk
        srand(0);
        for (auto & i : randomData){
            i=rand();
        }
    }
    void checkFileContent(uint32_t pageSz,size_t nPages){
        //now the pager is destroyed
        FILE * fp = fopen("/tmp/empty","r");
        std::vector<int> buf(randomData.size(),0);
        auto ret = fread(buf.data(), pageSz,nPages,fp);
        ASSERT_EQ(ret,nPages);
        fclose(fp);
        ASSERT_EQ( buf,randomData);
    }
    std::vector<int> randomData;
};


TEST_F(PagerTest, WriteWholeFile){
    uint32_t pageSz = 4096;
    fillRandomData(1024*1024);
    auto nPages = randomData.size()*4/pageSz;
    {
        Pager pager("/tmp/empty", PageSz_t(pageSz));
        for (uint32_t i=0;i<nPages;++i){
            auto ptr = pager.getPage( PageNo_t(i));
            auto dst = ptr.get();
            auto src = randomData.data()+ (i*pageSz / 4);
            ::memcpy(dst,src,pageSz);
        }
    }
    checkFileContent(pageSz,nPages);
}

TEST_F(PagerTest, WriteTwice){
    uint32_t pageSz = 4096;
    fillRandomData(1024*1024);
    auto nPages = randomData.size()*4/pageSz;
    {
        Pager pager("/tmp/empty", PageSz_t(pageSz));
        for (int j=0;j<2;++j){
            for (uint32_t i=0;i<nPages;++i){
                auto ptr = pager.getPage( PageNo_t(i));
                auto dst = ptr.get();
                auto src = randomData.data()+ (i*pageSz / 4);
                ::memcpy(dst,src,pageSz);
            }
        }
    }
    checkFileContent(pageSz,nPages);
}
TEST_F (PagerTest, WithEviction){
    uint32_t pageSz = 4096;
    fillRandomData(1024*10);
    auto nPages = randomData.size()*4/pageSz;
    {
        Pager pager("/tmp/empty", PageSz_t(pageSz), 1);
        for (int j=0;j<2;++j){
            for (uint32_t i=0;i<nPages;++i){
                auto ptr = pager.getPage( PageNo_t(i));
                auto dst = ptr.get();
                auto src = randomData.data()+ (i*pageSz / 4);
                ::memcpy(dst,src,pageSz);
            }
        }
    }
    checkFileContent(pageSz,nPages);
}

TEST_F (PagerTest, ShouldExpire){
    uint32_t pageSz = 4096;
    {
        Pager pager("/tmp/empty", PageSz_t(pageSz),2);
        std::weak_ptr<PagePtr::PageData> wkPtr;
        {
            auto ptr1 = pager.getPage(PageNo_t(1));
            wkPtr = ptr1.getWeakPtr();
        }
        auto ptr2 = pager.getPage(PageNo_t(2));
        auto ptr3 = pager.getPage(PageNo_t(3));
        ASSERT_TRUE(wkPtr.expired());
    }
}

TEST_F (PagerTest, ShouldNotCreateTwoBuffers){
    uint32_t pageSz = 4096;
    {
        Pager pager("/tmp/empty", PageSz_t(pageSz),2);
        auto ptr1 = pager.getPage(PageNo_t(1));
        auto ptr2 = pager.getPage(PageNo_t(2));
        auto ptr3 = pager.getPage(PageNo_t(3));
        //now ptr1 is evicted from LRU list but still alive
        auto ptr4 = pager.getPage(PageNo_t(1));
        ASSERT_EQ(ptr1,ptr4);
    }
}

int main (int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
