#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <gtest/gtest.h>
#include "PagePtr.h"

class PagePtrTest: public ::testing::Test{
public:
    FILE* fp;
    PagePtrTest():sz(4096){
    }
    void SetUp() override{
        fp = fopen("/tmp/empty","w+"); //truncated to 0, allow read/write
        //rw means "don't truncate, allow read/write"
    }
    void TearDown() override{
        fclose(fp);
    }
    PageSz_t sz;
};

TEST_F(PagePtrTest, StdioBehavior){
    char buf_[100]={0};
    auto ret = fwrite(buf_,1,100, fp);
    ASSERT_EQ(ret,100);
    fseek(fp,0,SEEK_SET);
    ASSERT_EQ(fread(buf_,1,100,fp),100);
}

TEST_F(PagePtrTest, EmptyFile){
    std::vector<char>  emptyTrunk(4096);
    auto ptr = PageData::fromFile(fp,sz,PageNo_t(0));
    const auto content = ptr->getConst();
    auto ret = memcmp(content, emptyTrunk.data(),emptyTrunk.size());
    ASSERT_EQ(ret,0);
}

TEST_F(PagePtrTest, CorrectPosition){
    auto pos = ftell(fp);
    ASSERT_EQ(pos,0);
    fseek(fp,100,SEEK_SET); //out of bound
    {
        auto ptr = PageData::fromFile(fp,sz,PageNo_t(0));
    }
    ASSERT_EQ( 100, ftell(fp));
}

TEST_F(PagePtrTest, SmallFile){
    std::vector<char> smallFile(100);
    srand(0);
    for (auto & c: smallFile){
        c=static_cast<char>(rand());
    }

    PageNo_t page(0);
    auto ret = fwrite(smallFile.data(),1,100, fp);
    ASSERT_EQ(ret,100);
    smallFile.resize(4096); //rest of the part should be zero
    auto ptr = PageData::fromFile(fp,sz,page);
    auto buf = ptr->getConst();
    ASSERT_EQ(0,::memcmp(buf,smallFile.data(), smallFile.size()));
}

TEST_F(PagePtrTest, WriteBack){
    std::vector<char> smallTrunk(100);
    srand(0);
    for (auto & c: smallTrunk){
        c=static_cast<char>(rand());
    }
    auto deleter = [this](void*ptr, PageSz_t  ,PageNo_t page){
        long offset = page*sz;
        ::fseek(fp,offset ,SEEK_SET);
        int size_ = sz;
        ASSERT_EQ(size_,fwrite(ptr, 1,size_,fp));
    };
    {
        auto ptr1 = PageData::fromFile(fp,sz,PageNo_t(0),deleter);
        auto buf = ptr1->getNonConst(); //now it's dirty
        memcpy(buf,smallTrunk.data(),smallTrunk.size());
    }
    auto ptr2 = PageData::fromFile(fp,sz,PageNo_t(0));
    auto constBuf = ptr2->getConst();
    ASSERT_EQ(0,::memcmp(constBuf,smallTrunk.data(),smallTrunk.size()));
}
int main (int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
