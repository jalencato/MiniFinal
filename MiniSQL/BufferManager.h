#pragma once
#define BLOCKSIZE 4096
#include<iostream>
#include <map>
#include <list>
#include<string>

using namespace std;

class Block
{
public:
	string file_name;
	unsigned int offset;
	bool dirty;
	int pin;

	int size;

	unsigned char buf[BLOCKSIZE];

	Block operator=(const Block&);
};

class BufferPool
{
public:
	static BufferPool& instance()
	{
		static BufferPool instance;
		return instance;
	}

	BufferPool();

	Block* mem_malloc(const string& fileName, unsigned int offset);
	void mem_free(Block* blockPtr);
private:
	Block* blockPool;
	//true for available
	list<pair<int, bool>> table;
};

typedef pair<string, unsigned int> tag;
typedef list<Block*>::iterator bufiter;

class BufferManager
{
public:
	BufferManager();
	~BufferManager();

	static BufferManager& instance()
	{
		static BufferManager instance;
		return instance;
	}

	bool isDirty(const Block* blockPtr);
	bool isPin(const Block* blockPtr);
	void increPin(Block* blockPtr);
	void decrePin(Block* blockPtr);
	void setDirty(Block* blockPtr);

	Block* readBlock(const string& blockName, unsigned int offset);

	void writeBlock(Block* blockPtr);
	void flushBlock();
	void deleteFile(const string & fileName);
private:
	map<tag, bufiter> table;// cache name store
	list<Block*> buffer;// cache buffer

	Block* readFromDisk(const string& fileName, unsigned int offset);

	void writeFile(Block* filePtr);
};

class BufferStream//only char and Block*
{
private:
	unsigned char* buf_;
	size_t size_;
	size_t iter_;

public:
	BufferStream(unsigned char* buf_, size_t size)//stream char
		:buf_(buf_), size_(size), iter_(0) {};
	BufferStream(Block* block)//stream pass block
		:buf_(block->buf), size_(4096), iter_(0) {};

	void setIter(unsigned int offset)
	{
		iter_ = offset;
	}

	size_t getIter()
	{
		return iter_;
	}

	void addIter(int offset)
	{
		iter_ += offset;
	}

	size_t available()
	{
		return size_ - iter_;
	}

	template<class T>
	void front(T& strval)
	{
		//using char to not recode without any changes
		memcpy(reinterpret_cast<char*>(&strval), buf_ + iter_ , sizeof(T));
	}

	template<class T>
	BufferStream& operator <<(T& strval)
	{
		memcpy(buf_ + iter_, reinterpret_cast<char*>(&strval), sizeof(T));
		iter_ += sizeof(T);

		return *this;
	}

	BufferStream& operator <<(string& strval)
	{
		memcpy(buf_ + iter_, strval.c_str(), strval.size() + 1);
		iter_ += strval.size() + 1;

		return *this;
	}

	template<class T>
	BufferStream& operator >>(T& strval)
	{
		memcpy(reinterpret_cast<char*>(&strval), buf_ + iter_, sizeof(T));
		iter_ += sizeof(T);

		return *this;
	}

	BufferStream& operator >>(string& strval)
	{
		strval.clear();
		while(buf_[iter_] != '\0')
		{
			strval.push_back(buf_[iter_++]);
		}
		iter_++;

		return *this;
	}
};