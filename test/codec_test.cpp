#include <vector>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <gtest/gtest.h>
#include "Codec.h"
#include "ReaderWriter.h"
#include "Value.h"
#include "helper.h"
using namespace std;

class MockReader : public Reader{
public:
    MockReader (const string& str)
        :str_(str),pos_(0)
    {
    }
    void read (void*dst, size_t sz)override{
        auto src = str_.c_str()+pos_;
        ::memcpy(dst,src,sz);
        pos_+=sz;
    }
private:
    string str_;
    size_t pos_;
};

class MockWriter : public Writer{
public:
    MockWriter ()
    {
    }
    void write (const void*_src, size_t sz)override{
        const char * src = static_cast<const char*> (_src);
        str_.append(src,sz);
    }
    const string& str(){return str_;}
    string str_;
};

class CodecTest: public ::testing::Test{
public:
    
};

TEST(CodecTest, SimpleIO ){
    auto buf = Helper::randomData(1024);
    MockReader reader(buf);
    MockWriter writer;
    for (size_t i=0;i<buf.size()/sizeof(int);++i){
        auto ptr = ::deserialize(DataType::INTEGER,StorageClass::INT32,reader);
        ::serialize(*ptr,StorageClass::INT32,writer);
        delete ptr;
    }
    auto str =writer.str();
    ASSERT_EQ(str,buf);
}
int main (int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
