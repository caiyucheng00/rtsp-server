#ifndef __BUFFER__H_
#define __BUFFER__H_

#include <stdlib.h>
#include <algorithm>
#include <stdint.h>
#include <assert.h>

class Buffer
{
public:
	explicit Buffer();
	~Buffer();

	int readableBytes() const;
	int writableBytes() const;
	int prependableBytes() const;
	char* peek();
	const char* peek() const;
	const char* findCRLF() const;
	const char* findCRLF(const char* start) const;
	const char* findLastCrlf() const;
	void retrieveReadZero();
	void retrieve(int len);
	void retrieveUntil(const char* end);
	void retrieveAll();//恢复索引
	char* beginWrite();
	const char* beginWrite() const;
	void unwrite(int len);
	void ensureWritableBytes(int len);  //确保有足够的空间
	void makeSpace(int len);
	void append(const char* data, int len);
	void append(const void* data, int len);

	int read(int fd);
	int write(int fd);

	static const int initialSize;

private:
	char* begin();
	const char* begin() const;

	char* _buffer;
	int _bufferSize;
	int _readIndex;  // 当前
	int _writeIndex;  //当前从socket实际读取到的字节长度

	static const char* kCRLF;
};

#endif