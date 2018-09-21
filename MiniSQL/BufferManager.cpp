#define _CRT_SECURE_NO_WARNINGS

#include "BufferManager.h"
#include <fstream>
#include <cassert>


BufferPool::BufferPool()
{
	try
	{
		blockPool = new Block[513];
	}catch(const bad_alloc&)
	{
		cerr << "Error: can not create the BlockPool" << endl;
	}
	for (int i = 0; i < 513; i++)
	{
		//initilalize all the block available to use
		table.push_back(make_pair(i, true));
	}
}

Block* BufferPool::mem_malloc(const string& fileName, unsigned int offset)
{
	auto& item = table.front();

	if(!item.second)//false has been free
	{
		cerr << "this unit has been malloced" << endl;
		return nullptr;
	}

	item.second = false;

	Block* blockPtr = &blockPool[item.first];

	blockPtr->file_name = fileName;
	blockPtr->offset = offset;
	blockPtr->pin = 0;
	blockPtr->dirty = false;
	blockPtr->size = 0;
	memset(blockPtr->buf, 0, BLOCKSIZE);

	table.push_back(item);
	table.pop_front();

	return blockPtr;
}

void BufferPool::mem_free(Block* blockPtr)
{
	int offset = (int)(blockPtr - blockPool);

	table.remove(make_pair(offset, false));
	table.push_front(make_pair(offset, true));
}

BufferManager::BufferManager()
{
}

BufferManager::~BufferManager()
{
}

bool BufferManager::isDirty(const Block* blockPtr)
{
	return blockPtr->dirty;
}

bool BufferManager::isPin(const Block* blockPtr)
{
	return blockPtr->pin != 0;
}

void BufferManager::increPin(Block* blockPtr)
{
	blockPtr->pin++;
}

void BufferManager::decrePin(Block* blockPtr)
{
	blockPtr->pin--;
	if (blockPtr->pin < 0)
		exit(0);
}

void BufferManager::setDirty(Block* blockPtr)
{
	blockPtr->dirty = true;
}

Block* BufferManager::readBlock(const string& blockName, unsigned int offset)
{
	auto blockTag = make_pair(blockName, offset);
	auto tIter = table.find(blockTag);

	if(tIter != table.end())//already load to the block
	{
		buffer.push_back(*(tIter->second));
		buffer.erase(tIter->second);

		auto bIter = buffer.end();
		--bIter;

		table.find(blockTag)->second = bIter;

		return *bIter;
	}
	else//load from disk to the buffer
	{
		Block *blockPtr = readFromDisk(blockName, offset);

		buffer.push_back(blockPtr);
		auto bIter = buffer.end();
		--bIter;

		table.insert(make_pair(blockTag, bIter));

		if (buffer.size() > 512)
			//it has been large than the whole buffer
		{
			auto vic = buffer.begin();//vic = list<Block*>::iterator
			while ((*vic)->pin)//not no possible
				vic++;
			writeBlock(*vic);
		}

		return *bIter;
	}
}

void BufferManager::writeBlock(Block* blockPtr)
{
	tag blockTag = make_pair(blockPtr->file_name, blockPtr->offset);

	writeFile(blockPtr);

	auto tIter = table.find(blockTag);
	if (tIter != table.end())//find in the table
	{
		buffer.erase(tIter->second);
		table.erase(tIter);
	}
}

void BufferManager::flushBlock()
{
	for (auto& blockPtr : buffer)
		writeFile(blockPtr);

	buffer.clear();
	table.clear();
}

void BufferManager::deleteFile(const string& fileName)
{
	auto tIter = table.lower_bound(make_pair(fileName, 0));
	while(tIter != table.end()&&tIter->first.first == fileName)
	{
		auto tmp = tIter;
		writeBlock(*(tmp->second));
		++tIter;
	}

	remove(fileName.c_str());
}

Block* BufferManager::readFromDisk(const string& fileName, unsigned int offset)
{
	FILE *fp = fopen(fileName.c_str(), "rb");
	if (fp == nullptr)
		cerr << "Error: open file " << fileName << " fail when reading" << endl;

	fseek(fp, offset, SEEK_SET);//fseek: offset, start

	//malloc unit in the buffer
	Block* blockPtr = BufferPool::instance().mem_malloc(fileName, offset);

	fread(blockPtr->buf, 1, BLOCKSIZE, fp);//write back: each size number_of_unit stream
	fclose(fp);

	return blockPtr;
}

void BufferManager::writeFile(Block* filePtr)
{
	if(isDirty(filePtr))
	{
		FILE *fp = fopen(filePtr->file_name.c_str(),"rb+");

		if(!fp)
		{
			cerr << "can not open the file";
			cerr << filePtr->file_name << "when writing" << endl;
		}

		fseek(fp, filePtr->offset, SEEK_SET);
		fwrite(filePtr->buf, 1, BLOCKSIZE, fp);
		fclose(fp);
	}

	//not necessary to write back to the disk if the buffer is not dirty
	BufferPool::instance().mem_free(filePtr);
}