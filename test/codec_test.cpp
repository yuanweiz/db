#include <vector>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <gtest/gtest.h>
#include "Codec.h"
#include "ReaderWriter.h"
#include "TableDesc.h"
#include "Exception.h"
#include "TableRow.h"
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
        if (str_.size() < pos_+sz){
            throw Exception("No enough bytes to read");
        }
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

TEST(CodecTest, INT32 ){
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
TEST(CodecTest, INT64 ){
    auto buf = Helper::randomData(1024);
    MockReader reader(buf);
    MockWriter writer;
    for (size_t i=0;i<buf.size()/sizeof(int64_t);++i){
        auto ptr = ::deserialize(DataType::INTEGER,StorageClass::INT64,reader);
        ::serialize(*ptr,StorageClass::INT64,writer);
        delete ptr;
    }
    auto str =writer.str();
    ASSERT_EQ(str,buf);
}

TEST(CodecTest, TEXT ){
    vector<string> strs;
    MockWriter writer;
    for (int i=0;i<10;++i){
        strs.push_back(Helper::randomData(32));
    }
    for (auto & str: strs){
        TextValue tv(str);
        ::serialize(tv,StorageClass::TEXT, writer);
    }
    auto buf = writer.str();
    MockReader reader(buf);
    for (int i=0;i<10;++i){
        auto pv = ::deserialize(DataType::TEXT,StorageClass::TEXT, reader);
        TextValue * tv = value_cast<TextValue*>(pv);
        ASSERT_TRUE( tv!=nullptr);
        ASSERT_EQ(tv->value() ,strs[i]);
        delete pv;
    }
    auto str =writer.str();
    //ASSERT_EQ(str,buf);
}

TEST(CodecTest, SerializeColumn){
    TableDesc desc("new table");
    desc << textColumn("text");
    TableRow row;
    row << "String field" ;
    MockWriter writer;
    ::serialize(row,desc,writer);
    auto str = writer.str();
    MockReader reader(str);
    TableRow newRow;
    deserialize(desc,&row,reader);
}
int main (int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
