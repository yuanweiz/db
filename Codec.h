#ifndef __CODEC_H
#define __CODEC_H
#include "Types.h"
#include "ReaderWriter.h"
class TableRow;
class TableDesc;
class Value;
void serialize(const Value&, StorageClass, Writer&); 
size_t serializedSize(const Value&, StorageClass);
Value* deserialize(DataType, StorageClass, Reader&); 
void serialize(const TableRow&, const TableDesc&, Writer&);
void serializedSize(const TableRow&, const TableDesc&);
void deserialize(const TableDesc&,TableRow*, Reader&); 

#endif// __CODEC_H
