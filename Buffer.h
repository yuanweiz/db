#ifndef __BUFFER_H
#define __BUFFER_H
#include <sys/types.h>
#include <vector>
#include <stdio.h>
#include <memory.h>

class OnStackBuffer {
public:
	OnStackBuffer(); 
	void append(const char *,size_t);
	void append(char);
	void retrieve(size_t);
	void retrieveString(void *, size_t);
	char* data();
	void add(size_t);
	char* peek();
	size_t readable();
    size_t readFile(FILE*);
private:
	size_t start_, end_;
	char data_[1024];
};


#endif// __BUFFER_H
