#ifndef __CODEC_H
#define __CODEC_H
#include "Types.h"
#include "ReaderWriter.h"
class Value;
void serialize(const Value&, StorageClass, Writer&); 
Value* deserialize(DataType, StorageClass, Reader&); 

#endif// __CODEC_H
