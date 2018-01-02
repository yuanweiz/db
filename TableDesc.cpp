#include <unordered_map>
#include "TableDesc.h"

//enum class DataType : int{
//    INTEGER, REAL, TEXT,
//};
//enum class StorageClass :int{
//    VARINT,INT64,INT32,INT16, DOUBLE,FLOAT, TEXT,
//} ; 
std::unordered_map<StorageClass,DataType> storageClassToDataTypeMap = {
    {StorageClass::TEXT, DataType::TEXT},
    {StorageClass::VARINT, DataType::INTEGER},
    {StorageClass::INT16, DataType::INTEGER},
    {StorageClass::INT32, DataType::INTEGER},
    {StorageClass::INT64, DataType::INTEGER},
    {StorageClass::DOUBLE, DataType::REAL},
    {StorageClass::FLOAT, DataType::REAL},
};

std::unique_ptr<TableColumn> textColumn(const std::string& name){
    return std::unique_ptr<TableColumn>(new TableColumn(StorageClass::TEXT,name));
}

std::unique_ptr<TableColumn> integerColumn(const std::string& name){
    return std::unique_ptr<TableColumn>(new TableColumn(StorageClass::INT64,name));
}

std::unique_ptr<TableColumn> doubleColumn(const std::string& name){
    return std::unique_ptr<TableColumn>(new TableColumn(StorageClass::DOUBLE,name));
}
DataType storageClassToDataType(StorageClass storage){
    return storageClassToDataTypeMap[storage];
}


