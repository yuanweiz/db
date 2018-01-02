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
TEST(TableRowTest, Cast){
}
int main (int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
