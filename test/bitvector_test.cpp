#include <vector>
#include <stdlib.h>
#include <gtest/gtest.h>
#include "BitVector.h"
using namespace std;
class BitVectorTest: public ::testing::Test{
};

TEST(BitVectorTest, RandomVector){
    std::vector<bool> randomBits(10000);
    char buf[2000];
    BitVector bitmap(buf);
    srand(0);
    for (size_t i=0;i<randomBits.size();++i){
        bool odd = (rand()%2==0);
        randomBits[i]=odd;
        bitmap.set(i,odd);
    }
    for (size_t i=0;i<randomBits.size();++i){
        ASSERT_EQ(bitmap.get(i), randomBits[i]);
    }
}
int main (int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
