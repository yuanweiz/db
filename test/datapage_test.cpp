#include <detail/DataPage.h>
#include <vector>
#include <stdlib.h>
#include <gtest/gtest.h>
#include <Logging.h>
#include <memory>
#include "helper.h"
using detail::DataPageView;

struct DataPageViewTest : ::testing::Test{
    void SetUp() override{
        srand(0);
        buffer_.reset(new char[4096]);
        view_.reset( new DataPageView(buffer_.get(), 4096));
        view_->format();
    }
    void TearDown()override{
        view_->sanityCheck();
    }
    std::unique_ptr<char[]> buffer_;
    std::unique_ptr<DataPageView> view_;
};

TEST_F ( DataPageViewTest, Init){
    auto & view = *view_;
    ASSERT_EQ(view.numOfCells(), 0);
}
TEST_F ( DataPageViewTest, Insert){
    auto & view = *view_;
    printf("before alloc\n");
    //view.dump();
    auto ret = view.allocCell(10);
    printf("after alloc\n");
    view.sanityCheck();
    view.dump();
    Helper::fillRandomData(ret,10);
    ASSERT_EQ(view.numOfCells(), 1);
}

TEST_F ( DataPageViewTest, AllocateMultipleCells){
    auto & view = *view_;
    printf("before alloc\n");
    view.dump();
    for (int i=0;i<5;++i){
        auto ret = view.allocCell(10);
        printf("after alloc i=%i\n",i);
        view.dump();
        Helper::fillRandomData(ret,10);
        ASSERT_EQ(view.numOfCells(), i+1);
    }
}
TEST_F ( DataPageViewTest, AllocateAndDrop){
    auto & view = *view_;
    printf("before alloc\n");
    view.dump();
    for (int i=0;i<5;++i){
        auto ret = view.allocCell(10);
        printf("after alloc i=%i\n",i);
        //view.dump();
        Helper::fillRandomData(ret,10);
        ASSERT_EQ(view.numOfCells(), i+1);
    }
    while(true){
        auto nCells = view.numOfCells();
        if(nCells==0)break;
        int idx = rand()% nCells;
        LOG_DEBUG << "drop cell " << idx;
        view.dropCell(idx);
        view.dump();
        view.sanityCheck();
        //view.dump();
    }
}

TEST_F(DataPageViewTest, RandomSizeAllocationAndDrop){
    const int MAX_SIZE=50;
    auto & view = *view_;
    const int MaxCellCount = 100;
    Logger::setLevel(Logger::LogFatal);
    for (int i=0;i<MaxCellCount;++i){
        view.allocCell(rand()%MAX_SIZE);
    }
    for (int i=0;i<MaxCellCount/2;++i){
        auto nCells = view.numOfCells();
        view.dropCell(rand()%nCells);
        view.sanityCheck();
    }
    Logger::setLevel(Logger::LogDebug);
    for (int i=0;i<MaxCellCount/2;++i){
        auto size=rand()%MAX_SIZE;
        //LOG_DEBUG << "allocating size=" <<size;
        view.allocCell(size);
        view.sanityCheck();
    }
    int nCells;
    while((nCells=view.numOfCells())!=0){
        view.dropCell(rand()%nCells);
        view.sanityCheck();
    }
}

TEST_F(DataPageViewTest, InsertAt){
    auto & view = * view_;
    void * p = view.allocCellAt(0,100);
    auto strView = view.getCell(0);
    ASSERT_EQ(p,strView.data());
}

TEST_F(DataPageViewTest, InsertAt2){
    auto & view = * view_;
    void *p1=view.allocCellAt(0,101);
    void *p2=view.allocCellAt(0,102);
    void *p3=view.allocCellAt(1,103);
    {
        auto strView = view.getCell(0);
        ASSERT_EQ(p2,strView.data());
    }
    {
        auto strView = view.getCell(1);
        ASSERT_EQ(p3,strView.data());
    }
    {
        auto strView = view.getCell(2);
        ASSERT_EQ(p1,strView.data());
    }
}

int main (int argc, char** argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
