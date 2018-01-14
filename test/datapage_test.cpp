#include <detail/DataPage.h>
#include <vector>
#include <stdlib.h>
#include <gtest/gtest.h>
#include <Logging.h>
#include <memory>
using detail::DataPageView;

void fillRandomData(void* dst_,size_t size){
    char * dst = static_cast<char*>(dst_);
    for (size_t i=0;i<size;++i){
        char c = static_cast<char>(rand());
        dst[i]=c;
    }
}
struct DataPageViewTest : ::testing::Test{
    void SetUp() override{
        buffer_.reset(new char[4096]);
        view_.reset( new DataPageView(buffer_.get(), 4096));
        view_->format();
    }
    std::unique_ptr<char[]> buffer_;
    std::unique_ptr<DataPageView> view_;
};

TEST_F ( DataPageViewTest, Init){
    auto & view = *view_;
    ASSERT_EQ(view.numOfCells(), 0);
    view.sanityCheck();
}
TEST_F ( DataPageViewTest, Insert){
    auto & view = *view_;
    printf("before alloc\n");
    view.dump();
    auto ret = view.allocCell(10);
    printf("after alloc\n");
    view.dump();
    fillRandomData(ret,10);
    ASSERT_EQ(view.numOfCells(), 1);
    view.sanityCheck();
}

TEST_F ( DataPageViewTest, AllocateMultipleCells){
    auto & view = *view_;
    printf("before alloc\n");
    view.dump();
    for (int i=0;i<5;++i){
        auto ret = view.allocCell(10);
        printf("after alloc i=%i\n",i);
        view.dump();
        fillRandomData(ret,10);
        ASSERT_EQ(view.numOfCells(), i+1);
    }
    view.sanityCheck();
}
TEST_F ( DataPageViewTest, AllocateAndDrop){
    auto & view = *view_;
    printf("before alloc\n");
    view.dump();
    for (int i=0;i<5;++i){
        auto ret = view.allocCell(10);
        printf("after alloc i=%i\n",i);
        view.dump();
        fillRandomData(ret,10);
        ASSERT_EQ(view.numOfCells(), i+1);
    }
    while(true){
        auto nCells = view.numOfCells();
        if(nCells==0)break;
        int idx = rand()% nCells;
        LOG_DEBUG << "drop cell " << idx;
        view.dropCell(idx);
        view.dump();
    }
    view.sanityCheck();
}

int main (int argc, char** argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
