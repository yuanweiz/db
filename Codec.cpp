#include <functional>
#include <map>
#include <unordered_map>
#include <assert.h>
#include "Exception.h"
#include "ReaderWriter.h"
#include "TableRow.h"
#include "TableDesc.h"
#include "Codec.h"
#include "Value.h"

using Key = std::pair<DataType,StorageClass>;
using ReadFunc =std::function<Value*(Reader&)>;
using WriteFunc = std::function<void(const Value*,Writer&)>;
using WriteFunctionMap = std::map<Key,WriteFunc>;
using ReadFunctionMap = std::map<Key,ReadFunc>;
Value* readInt32(Reader& reader){
    int32_t i;
    reader.read(&i,4);
    return new IntegerValue(i);
}

Value* readInt64(Reader&reader){
    int64_t i;
    reader.read(&i,sizeof(i));
    return new IntegerValue(i);
}

Value* readDouble(Reader& reader){
    double d;
    reader.read(&d,sizeof(d));
    return new RealValue(d);
}

Value* readFloat(Reader& reader){
    float f;
    reader.read(&f,sizeof(f));
    return new RealValue(f);
}

Value* readText(Reader& reader){
    uint32_t sz;
    reader.read(&sz,sizeof(sz));
    std::string str(sz,'\0');
    reader.read(&*str.begin(), sz);
    return new TextValue(str);
}

size_t serializedSize(const Value& v, StorageClass sc){
    if (v.dataType() != storageClassToDataType(sc))
        throw Exception("type mismatch");
    switch (sc){
        case StorageClass::INT64:
        case StorageClass::DOUBLE:
            return 8;
        case StorageClass::FLOAT:
        case StorageClass::INT32:
            return 4;
        case StorageClass::INT16:
            return 2;
        case StorageClass::TEXT:
            {
            const auto pt = value_cast<const TextValue*>(&v);
            return pt->value().size()+4;
            }
        default:;
    }
    throw Exception("not implemented");
}

//writer functions
void writeDouble(const RealValue* real,Writer& writer){
    double d=real->value();
    writer.write(&d,sizeof(d));
}

void writeFloat(const RealValue* real,Writer& writer){
    float f=static_cast<float>(real->value());
    writer.write(&f,sizeof(f));
}

void writeInt32(const IntegerValue* iv, Writer& writer){
    int i = static_cast<int>(iv->value());
    writer.write(&i,sizeof(i));
}

void writeInt64(const IntegerValue* iv, Writer& writer){
    int64_t i = iv->value();
    writer.write(&i,sizeof(i));
}

void writeText(const TextValue* tv, Writer& writer){
    auto sz_u64 = tv->value().size();
    auto ptr = tv->value().c_str();
    uint32_t sz_u32 = static_cast<uint32_t>(sz_u64);
    writer.write(&sz_u32,4);
    writer.write(ptr,sz_u32);
}

class WriteFunctionTable{
public:
    static WriteFunctionTable& instance(){
        static WriteFunctionTable table;
        return table;
    }
    WriteFunctionMap& getMap(){
        return map_;
    }
private:
//    template <class T,class Func>
//        void addEntry(StorageClass storageClass, const Func& func){
    template <class T>
        void addEntry(StorageClass storageClass, const std::function<void(const T*,Writer&)>& func){
            DataType dataType = T::classDataType();
            auto typeErasedFunc = [=](const Value* v, Writer& writer){
                const T* tv = value_cast<const T*>(v);
                if (!tv){
                    throw Exception("Bad type cast");
                }
                func(tv,writer);
            };
            Key key={dataType,storageClass};
            auto ret = map_.insert({key,typeErasedFunc});
            assert(ret.second); (void)ret;
        }
    WriteFunctionTable(){
        addEntry<RealValue>(StorageClass::DOUBLE, writeDouble);
        addEntry<RealValue>(StorageClass::FLOAT, writeFloat);
        addEntry<IntegerValue>(StorageClass::INT32, writeInt32);
        addEntry<IntegerValue>(StorageClass::INT64, writeInt64);
        addEntry<TextValue>(StorageClass::TEXT, writeText);
    }
    WriteFunctionMap map_;
};

class ReadFunctionTable{
public:
    static ReadFunctionTable& instance(){
        static ReadFunctionTable table;
        return table;
    }
    ReadFunctionMap& getMap(){
        return map_;
    }
private:
    ReadFunctionTable(){
        addEntry(DataType::INTEGER, StorageClass::INT64, readInt64);
        addEntry(DataType::INTEGER, StorageClass::INT32, readInt32);
        addEntry(DataType::INTEGER, StorageClass::FLOAT, readFloat);
        addEntry(DataType::INTEGER, StorageClass::DOUBLE, readDouble);
        addEntry(DataType::TEXT, StorageClass::TEXT, readText);
    }
    void addEntry(DataType dataType,StorageClass storageClass, 
            const ReadFunc& func){
        Key key = {dataType,storageClass};
        auto ret= map_.insert({key,func});
        assert(ret.second);(void)ret;
    }
    ReadFunctionMap map_;
};

class Value;

void serialize(const Value&v, StorageClass storageType, Writer& writer){
    auto dataType=v.dataType();
    auto & functionTable = WriteFunctionTable::instance().getMap();
    Key key = {dataType, storageType};
    auto it = functionTable.find(key);
    if (it==functionTable.end()){
        throw Exception("unsupported cast");
    }
    it->second(&v,writer);
}

Value* deserialize(DataType dataType, StorageClass storageType, Reader& reader){
    auto & functionTable = ReadFunctionTable::instance().getMap();
    Key key = {dataType, storageType};
    auto it = functionTable.find(key);
    if (it==functionTable.end()){
        throw Exception("unsupported cast");
    }
    return it->second(reader);
}

void serialize(const TableRow& row, const TableDesc& desc, Writer& writer){
    //TODO: check mismatch
    if (row.size() != desc.size()){
        throw Exception("the row does not match table descriptor");
    }
    auto rowIter = row.begin();
    auto descIter = desc.begin();
    for (; rowIter!=row.end(); ++rowIter, ++descIter){
        auto & value = *rowIter;
        serialize (value, descIter->storageClass(),writer);
    }
}

void deserialize(const TableDesc&desc,TableRow*pRow, Reader& reader){
    auto &row =*pRow;
    for (auto & column: desc){
        auto sc = column.storageClass();
        auto dataType = storageClassToDataType(sc);
        auto * value = deserialize(dataType,sc,reader);
        row << value;
    }
}
