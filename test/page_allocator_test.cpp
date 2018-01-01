#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <gtest/gtest.h>
#include "PageAllocator.h"
using namespace std;
class PageAllocatorTest: public ::testing::Test{
public:
    void SetUp()override{
        auto fp=fopen("/tmp/empty","w+");
        ASSERT_TRUE(fp!=nullptr);
        ::fclose(fp); //now it's empty
    }
    void fixedSizedFile(size_t size){
        auto fp=fopen("/tmp/empty","w+");
        ASSERT_TRUE(fp!=nullptr);
        vector<char> zeros(size);
        ASSERT_EQ(size, fwrite(zeros.data(),1,size,fp));
        ::fclose(fp); //now it's empty
    }
};

TEST_F(PageAllocatorTest, Formatted){
    {
        PageAllocator allocator("/tmp/empty");
        ASSERT_FALSE(allocator.formatted());
        allocator.format();
        ASSERT_TRUE(allocator.formatted());
    }
    //out of scope
    PageAllocator allocator("/tmp/empty");
    ASSERT_TRUE(allocator.formatted());
}

TEST_F(PageAllocatorTest, GroupCount){
    uint32_t  groupSz= 8*4096*4096;
    {
        fixedSizedFile(0);
        PageAllocator allocator("/tmp/empty");
        allocator.format();
        ASSERT_EQ(allocator.maxGroup(),1);
    }
    {
        fixedSizedFile(groupSz);
        PageAllocator allocator("/tmp/empty");
        allocator.format();
        ASSERT_EQ(allocator.maxGroup(),1);
    }
    {
        fixedSizedFile(groupSz+1);
        PageAllocator allocator("/tmp/empty");
        allocator.format();
        ASSERT_EQ(allocator.maxGroup(),2);
    }
}

TEST_F(PageAllocatorTest, AllocFree){
    std::vector<PagePtr> pages;
    PageAllocator allocator("/tmp/empty");
    allocator.format();
    for (int i=0;i<50000;++i){
        pages.push_back(allocator.allocate());
    }
    for (auto & ptr:pages){
        allocator.deallocate(ptr);
    }
}

int main(int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}

