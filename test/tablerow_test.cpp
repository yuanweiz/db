#include <vector>
#include <stdlib.h>
#include <gtest/gtest.h>
#include <iostream>
#include "TableRow.h"
#include "Value.h"
using namespace std;
class TableRowTest: public ::testing::Test{
};

TEST(TableRowTest, Construct){
    float f = 0.0;
    int i = 42;
    TableRow row;
    row << f << i;
    for (auto & value: row){
        delete value.clone();
    }
}

size_t const_traverse(const TableRow & row){
    size_t i=0;
    for (auto & v: row){
        std::cout<< &v;
        ++i;
    }
    return i;
}
TEST(TableRowTest, Iterator){
    TableRow row;
    row << 42 << "text";
    ASSERT_EQ(row.size(), 2);
    size_t i=0;
    for (auto & v: row){
        std::cout<< &v;
        ++i;
    }
    ASSERT_EQ(const_traverse(row),2);
    ASSERT_EQ(i,2);
}
int main (int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
