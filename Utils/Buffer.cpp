#include "Buffer.h"
#include "SocketsOps.h"

const int Buffer::initialSize = 1024;
const char* Buffer::kCRLF = "\r\n";

Buffer::Buffer() :
	_bufferSize(initialSize),
	_readIndex(0),
	_writeIndex(0)
{
	_buffer = (char*)malloc(_bufferSize);
}

Buffer::~Buffer()
{
	free(_buffer);
}

int Buffer::readableBytes() const
{
	return _writeIndex - _readIndex;
}

int Buffer::writableBytes() const
{
	return _bufferSize - _writeIndex;
}

int Buffer::prependableBytes() const
{
	return _readIndex;
}

char* Buffer::peek()
{
	return begin() + _readIndex;
}

const char* Buffer::peek() const
{
	return begin() + _readIndex;
}

const char* Buffer::findCRLF() const
{
	const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
	return crlf == beginWrite() ? NULL : crlf;
}

const char* Buffer::findCRLF(const char* start) const
{
	assert(peek() <= start);
	assert(start <= beginWrite());
	const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
	return crlf == beginWrite() ? NULL : crlf;
}

const char* Buffer::findLastCrlf() const
{
	const char* crlf = std::find_end(peek(), beginWrite(), kCRLF, kCRLF + 2);
	return crlf == beginWrite() ? NULL : crlf;
}

void Buffer::retrieveReadZero()
{
	_readIndex = 0;
}

void Buffer::retrieve(int len)
{
	assert(len <= readableBytes());
	if (len < readableBytes())
	{
		_readIndex += len;
	}
	else
	{
		retrieveAll();
	}
}

void Buffer::retrieveUntil(const char* end)
{
	assert(peek() <= end);
	assert(end <= beginWrite());
	retrieve(end - peek());
}

void Buffer::retrieveAll()
{
	_readIndex = 0;
	_writeIndex = 0;
}

char* Buffer::beginWrite()
{
	return begin() + _writeIndex;
}

const char* Buffer::beginWrite() const
{
	return begin() + _writeIndex;
}

void Buffer::unwrite(int len)
{
	assert(len <= readableBytes());
	_writeIndex -= len;
}

void Buffer::ensureWritableBytes(int len)
{
	if (writableBytes() < len)
	{
		makeSpace(len);
	}
	assert(writableBytes() >= len);
}

void Buffer::makeSpace(int len)
{
	if (writableBytes() + prependableBytes() < len) //如果剩余空间不足
	{
		/* 扩大空间 */
		_bufferSize = _writeIndex + len;
		_buffer = (char*)realloc(_buffer, _bufferSize);
	}
	else //剩余空间足够
	{
		/* 移动内容 */
		int readable = readableBytes();
		std::copy(begin() + _readIndex, begin() + _writeIndex, begin());
		_readIndex = 0;
		_writeIndex = _readIndex + readable;
		assert(readable == readableBytes());
	}

}

void Buffer::append(const char* data, int len)
{
	ensureWritableBytes(len); //调整扩大的空间
	std::copy(data, data + len, beginWrite()); //拷贝数据

	assert(len <= writableBytes());
	_writeIndex += len;//重新调节写位置
}

void Buffer::append(const void* data, int len)
{
	append((const char*)(data), len);
}

int Buffer::read(int fd)
{
	char extrabuf[65536];
	const int writable = writableBytes();
	const int n = ::recv(fd, extrabuf, sizeof(extrabuf), 0);
	if (n <= 0) {
		return -1;
	}
	else if (n <= writable)
	{
		std::copy(extrabuf, extrabuf + n, beginWrite()); //拷贝数据
		_writeIndex += n;

	}
	else
	{
		std::copy(extrabuf, extrabuf + writable, beginWrite()); //拷贝数据
		_writeIndex += writable;
		append(extrabuf + writable, n - writable);
	}
	return n;
}

int Buffer::write(int fd)
{
	return sockets::write(fd, peek(), readableBytes());
}

char* Buffer::begin()
{
	return _buffer;
}

const char* Buffer::begin() const
{
	return _buffer;
}
