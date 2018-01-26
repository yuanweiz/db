#ifndef __SIMPLE_FILESYSTEM_H
#define __SIMPLE_FILESYSTEM_H
#include <FileSystem.h>
#include <detail/FileSystemImpl.h>
#include <detail/FileImpl.h>
#include <detail/CursorImpl.h>

class SimpleFileSystem: public FileSystem{
public:
    SimpleFileSystem();
    class Impl;
};

#endif// __SIMPLE_FILESYSTEM_H
