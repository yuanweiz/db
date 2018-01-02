#include <vector>
#include <stdlib.h>
#include <gtest/gtest.h>
#include <iostream>
#include "Value.h"
using namespace std;
class ValueTest: public ::testing::Test{
};

TEST(ValueTest, Assign){
    std::string str="str";
    TextValue value = str;
    RealValue r=1.0;
    IntegerValue i=10;
}
TEST(ValueTest, Cast){
    Value * i1 = new IntegerValue(53);
    Value * i2 = i1->clone();
    IntegerValue * c1 =  value_cast<IntegerValue*>(i1);
    ASSERT_NE( nullptr ,c1);
    IntegerValue * c2 =  value_cast<IntegerValue*>(i2);
    ASSERT_NE( nullptr ,c2);
}
TEST(ValueTest, ConstCast){
    const Value * i1 = new IntegerValue(53);
    const Value * i2 = i1->clone();
    const IntegerValue * c1 =  value_cast<const IntegerValue*>(i1);
    ASSERT_NE( nullptr ,c1);
    const IntegerValue * c2 =  value_cast<const IntegerValue*>(i2);
    ASSERT_NE( nullptr ,c2);
}
int main (int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
