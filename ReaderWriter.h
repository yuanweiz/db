#ifndef __READER_WRITER_H
#define __READER_WRITER_H
#include <sys/types.h>
class Writer{
public:
    ~Writer(){}
    virtual void write(const void*,size_t)=0;
};

class Reader{
public:
    ~Reader(){}
    virtual void read(void*,size_t)=0;
};

#endif// __READER_WRITER_H

