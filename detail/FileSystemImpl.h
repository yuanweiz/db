#ifndef __FILESYSTEM_IMPL_H
#define __FILESYSTEM_IMPL_H
#include <FileSystem.h>
class FileSystem::Impl{
public:
    virtual File openFile(const std::string &name)=0;
    virtual ~Impl(){}
};
#endif// __FILESYSTEM_IMPL_H

