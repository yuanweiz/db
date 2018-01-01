#include <vector>
#include <stdlib.h>
#include <gtest/gtest.h>
#include <iostream>
#include "TableDesc.h"
using namespace std;
class TableDescTest: public ::testing::Test{
};

TEST(TableDescTest, Operator){
        TableDesc mytable("table");
        mytable << textColumn("some_text_column")
        << integerColumn("int_column")
        << doubleColumn("double_column");
    for (auto& column: mytable){
        auto type =  static_cast<int>(column.type());
        auto storage = static_cast<int>(column.storageClass());
        std::cout<< column.name() << type<< storage<<std::endl;
    }
}
int main (int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
