#include <gtest/gtest.h>
#include <BTree.h>
#include <memory>
#include "helper.h"

bool cmp(StringView a, StringView b){
    return std::string(a)<std::string(b);
}

StringView identity(StringView x){
    return x;
}

struct BTreeTest: ::testing::Test{

    void SetUp() override{
        srand(0);
        btree_ = nullptr;
        allocator_.reset(new Helper::MockPageAllocator());
        btree_.reset(new BTree(PageNo_t(2),allocator_.get()
                    ,cmp,cmp,identity));
    }
    std::vector<std::string> randomStrings(size_t maxLen, size_t count){
        std::vector<std::string>ret;
        for (size_t i=0;i<count;++i){
            int len = rand() % static_cast<int>(maxLen); //size from 0 to 100
            auto str = Helper::randomData(len);
            ret.push_back(str);
        }
        return ret;
    }
    std::unique_ptr<PageAllocatorBase> allocator_;
    std::unique_ptr<BTree> btree_;
};

TEST_F(BTreeTest, Empty){
    std::vector<std::string> shouldBeEmpty;
    for (auto && strView: *btree_){
        shouldBeEmpty.push_back(strView);
    }
    ASSERT_TRUE(shouldBeEmpty.empty());
}
TEST_F(BTreeTest, OnlyOnePage){
    std::vector<std::string> sortedStrings{randomStrings(100,20)}, actual;
    for (auto & str: sortedStrings){
        btree_->insert(str);
    }
    for (auto && strView: *btree_){
        actual.push_back(strView);
    }
    std::sort(sortedStrings.begin(),sortedStrings.end());
    ASSERT_EQ(sortedStrings,actual);
}
TEST_F(BTreeTest, ManyPages){
    std::vector<std::string> sortedStrings{randomStrings(100,200)}, actual;
    size_t i=0;
    for (auto & str: sortedStrings){
        ++i;
        printf("inserting %luth string\n",i);
        btree_->insert(str);
    }
    for (auto && strView: *btree_){
        actual.push_back(strView);
    }
    std::sort(sortedStrings.begin(),sortedStrings.end());
    ASSERT_EQ(sortedStrings,actual);
}
int main (int argc,char ** argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
