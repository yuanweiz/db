#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <gtest/gtest.h>
#include "PageAllocator.h"
using namespace std;
class PageAllocatorTest: public ::testing::Test{
    void SetUp()override{
        auto fp=fopen("/tmp/empty","w+");
        ASSERT_TRUE(fp!=nullptr);
        ::fclose(fp); //now it's empty
    }
};

TEST(PageAllocatorTest, Formatted){
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

int main(int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}

