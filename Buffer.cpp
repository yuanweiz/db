#include <string.h>
#include <assert.h>
#include "Buffer.h"
OnStackBuffer::OnStackBuffer() :start_(0), end_(0) {
    //bzero(data_,sizeof(data_));
}
char * OnStackBuffer::data() {
	return data_;
}

char * OnStackBuffer::peek() {
	return data_ + start_;
}
void OnStackBuffer::retrieve(size_t sz) {
	start_ += static_cast<int>(sz);
}
size_t OnStackBuffer::readable() {
	assert(end_ >= start_);
	return end_ - start_;
}

void OnStackBuffer::retrieveString(void *dst, size_t sz) {
	::memcpy(dst, peek(), sz);
	retrieve(sz);
}

void OnStackBuffer::append(const char*src, size_t sz) {
	::memcpy(data_+end_, src, sz);
	end_ += static_cast<int>(sz);
}
void OnStackBuffer::append(char c){
    data_[end_++] = c;
}
size_t OnStackBuffer::readFile(FILE* fp){
    auto avail = readable();
    ::memmove(data_, peek(), avail);
    start_ = 0;
    end_ = avail;
    auto sz= ::fread(data_ + end_, 1,sizeof(data_) - end_,fp);
	end_ += static_cast<int>(sz);
    return sz;
}
