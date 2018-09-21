#pragma once

#include <vector>
#include <map>
#include <list>
#include <string>
#include <fstream>

class BufferManager {

public:

	typedef void(*WRITETODISKCALLBACKFUNC)(std::string fileName, int block, void* buf);

private:

	struct Chunk {
		std::string fileName;
		int block;
		std::string mapKeyName;
		bool isDirty;
		void* buf;
		WRITETODISKCALLBACKFUNC writeToDiskCallBack;
	};

public:

	static BufferManager& GetInstance() {
		static BufferManager bufferManager;
		return bufferManager;
	}

	void* Create(std::string fileName, int block, WRITETODISKCALLBACKFUNC writeToDiskCallBack) {
		Chunk* chunk = AllocateChunk(fileName, block, writeToDiskCallBack);
		memset(chunk->buf, 0, CHUNKSIZE);
		FILE *fp = fopen(fileName.c_str(), "rb+");
		if (!fp) fp = fopen(fileName.c_str(), "wb+");
		fseek(fp, CHUNKSIZE * block, SEEK_SET);
		fwrite(chunk->buf, 1, CHUNKSIZE, fp);
		fclose(fp);
		return chunk->buf;
	}

	std::pair<bool, void*> Read(std::string fileName, int block, WRITETODISKCALLBACKFUNC writeToDiskCallBack) {
		std::string mapKeyName = GetMapKeyName(fileName, block);
		if (bufferMap.count(mapKeyName)) {
			Chunk* chunk = *bufferMap[mapKeyName];
			MoveChunkToBack(mapKeyName);
			return std::make_pair(true, chunk->buf);
		} else {
			Chunk* chunk = AllocateChunk(fileName, block, writeToDiskCallBack);
			FILE *fp = fopen(fileName.c_str(), "rb");
			fseek(fp, CHUNKSIZE * block, SEEK_SET);
			fread(chunk->buf, 1, CHUNKSIZE, fp);
			fclose(fp);
			return std::make_pair(false, chunk->buf);
		}
	}

	void Write(std::string fileName, int block) {
		std::string mapKeyName = GetMapKeyName(fileName, block);
		if (!bufferMap.count(mapKeyName)) {
			return;
		}
		Chunk* chunk = *bufferMap[mapKeyName];
		Write(chunk);
	}

	void SetDirty(std::string fileName, int block) {
		std::string mapKeyName = GetMapKeyName(fileName, block);
		if (!bufferMap.count(mapKeyName)) {
			return;
		}
		Chunk* chunk = *bufferMap[mapKeyName];
		chunk->isDirty = true;
	}

	void FreeAll() {
		while (!usedChunksList.empty()) {
			Chunk* chunk = usedChunksList.front();
			Free(chunk);
		}
	}

private:

	BufferManager() {
		for (int i = 0; i < NUMOFCHUNKS; i++) {
			Chunk* chunk = new Chunk;
			chunk->buf = operator new(CHUNKSIZE);
			availableChunksList.push_back(chunk);
		}
	}

	void Write(Chunk* chunk) {
		if (!chunk->isDirty) {
			return;
		}
		chunk->writeToDiskCallBack(chunk->fileName, chunk->block, chunk->buf);
		FILE *fp = fopen(chunk->fileName.c_str(), "rb+");
		fseek(fp, CHUNKSIZE * chunk->block, SEEK_SET);
		fwrite(chunk->buf, 1, CHUNKSIZE, fp);
		fclose(fp);
		chunk->isDirty = false;
	}

	void Free(Chunk* chunk) {
		if (chunk->isDirty) {
			Write(chunk);
		}
		std::list<Chunk*>::iterator it = bufferMap[chunk->mapKeyName];
		bufferMap.erase(chunk->mapKeyName);
		usedChunksList.erase(it);
		availableChunksList.push_back(chunk);
	}

	Chunk* AllocateChunk(std::string fileName, int block, WRITETODISKCALLBACKFUNC writeToDiskCallBack) {
		if (availableChunksList.size() == 0) {
			Chunk* leastRecentlyUsedChunk = usedChunksList.front();
			Free(leastRecentlyUsedChunk);
		}
		Chunk* chunk = availableChunksList.back();
		chunk->fileName = fileName;
		chunk->block = block;
		chunk->mapKeyName = GetMapKeyName(fileName, block);
		chunk->isDirty = false;
		chunk->writeToDiskCallBack = writeToDiskCallBack;
		availableChunksList.pop_back();
		usedChunksList.push_back(chunk);
		bufferMap[chunk->mapKeyName] = (--usedChunksList.end());
		return chunk;
	}

	void MoveChunkToBack(std::string mapKeyName) {
		if (!bufferMap.count(mapKeyName)) {
			return;
		}
		std::list<Chunk*>::iterator it = bufferMap[mapKeyName];
		Chunk* chunk = *it;
		usedChunksList.erase(it);
		usedChunksList.push_back(chunk);
		bufferMap[mapKeyName] = (--usedChunksList.end());
	}

	std::string GetMapKeyName(std::string fileName, int block) {
		std::string mapKeyName = fileName + ":" + std::to_string(block);
		return mapKeyName;
	}

	std::list<Chunk*> availableChunksList;
	std::list<Chunk*> usedChunksList;
	std::map<std::string, std::list<Chunk*>::iterator> bufferMap;

public:

	static const int CHUNKSIZE = 4096;
	static const int NUMOFCHUNKS = 512;

};

struct FixedLengthChar {
	char str[256];
	FixedLengthChar() {
		memset(this->str, 0, sizeof(this->str));
	}
	FixedLengthChar(const char* str) {
		memset(this->str, 0, sizeof(this->str));
		strcpy(this->str, str);
	}
	bool operator<(const FixedLengthChar& rhs) const {
		if (strcmp(str, rhs.str) < 0) {
			return true;
		} else {
			return false;
		}
	}
	bool operator>(const FixedLengthChar& rhs) const {
		return (rhs < (*this));
	}
};

std::ostream& operator << (std::ostream& stream, const FixedLengthChar& fixedLengthChar)
{
	stream << fixedLengthChar.str;
	return stream;
}

template <typename T>
class BPlusTree {

private:

	struct Node {
		std::vector<int> pointers;
		std::vector<T> keys;
		std::vector<bool> validFlags;
		T minKeyInLeaves;
		int index;
		int parentIndex;
		int nextIndex;
		bool isLeaf;
		Node(int index, int parentIndex, int nextIndex, int isLeaf)
			: index(index), parentIndex(parentIndex),
			nextIndex(nextIndex), isLeaf(isLeaf), minKeyInLeaves(T()) {
		}
	};

	struct FindResult {
		bool found;
		bool valid;
		Node* node;
		int posInNode;
	};

public:

	// 新建空树
	BPlusTree(std::string dataFileName, int keySize) : dataFileName(dataFileName), numOfNodes(0), keySize(keySize) {
		degree = GetDegree(keySize);
		;		rootIndex = CreateNode(NULLPTR, NULLPTR, true)->index;
	}

	// bulk load新建树
	// keys需要是排好序的
	BPlusTree(std::string dataFileName, int keySize, std::vector<T> keys, std::vector<int> offsets) : dataFileName(dataFileName), numOfNodes(0), keySize(keySize) {
		degree = GetDegree(keySize);
		if (keys.size() == 0) {
			rootIndex = CreateNode(NULLPTR, NULLPTR, true)->index;
		} else {
			std::vector<Node*> nodes;
			// 叶子
			int numOfKeysAndOffsets = (int)keys.size();
			int maxNumOfKeysAndOffsetsInEachLeaf = degree - 1;
			int numOfLeaves = CeilDiv(numOfKeysAndOffsets, maxNumOfKeysAndOffsetsInEachLeaf);
			for (int i = 0; i < numOfLeaves; i++) {
				Node* node = CreateNode(NULLPTR, NULLPTR, true);
				if (i != 0) {
					nodes[i - 1]->nextIndex = node->index;
				}
				nodes.push_back(node);
			}
			Node* currentLeaf = nodes[0];
			for (int i = 0; i < numOfKeysAndOffsets; i++) {
				if (currentLeaf->keys.size() == maxNumOfKeysAndOffsetsInEachLeaf) {
					currentLeaf = GetNodeViaIndex(currentLeaf->nextIndex);
				}
				currentLeaf->pointers.push_back(offsets[i]);
				currentLeaf->keys.push_back(keys[i]);
				currentLeaf->validFlags.push_back(true);
			}
			for (int i = 0; i < numOfLeaves; i++) {
				Node* node = nodes[i];
				node->minKeyInLeaves = node->keys[0];
			}
			//非叶子节点
			while (nodes.size() != 1) {
				std::vector<Node*> parentNodes;
				int numOfParentNodes = CeilDiv((int)nodes.size(), degree);
				for (int i = 0; i < numOfParentNodes; i++) {
					Node* node = CreateNode(NULLPTR, NULLPTR, false);
					if (i != 0) {
						parentNodes[i - 1]->nextIndex = node->index;
					}
					parentNodes.push_back(node);
				}
				Node* currentParentNode = parentNodes[0];
				for (Node* node : nodes) {
					if (currentParentNode->pointers.size() == degree) {
						currentParentNode = GetNodeViaIndex(currentParentNode->nextIndex);
					}
					node->parentIndex = currentParentNode->index;
					if (currentParentNode->pointers.size() != 0) {
						currentParentNode->keys.push_back(node->minKeyInLeaves);
					} else {
						currentParentNode->minKeyInLeaves = node->minKeyInLeaves;
					}
					currentParentNode->pointers.push_back(node->index);
				}
				nodes = parentNodes;
			}
			rootIndex = nodes[0]->index;
		}
	}

	// 从文件中获取树
	BPlusTree(std::string dataFileName) : dataFileName(dataFileName) {
		std::ifstream file(dataFileName + ".schema");
		file >> rootIndex;
		file >> degree;
		file >> numOfNodes;
		file >> keySize;
		file.close();
	}

	bool Insert(const T& key, int offset) {
		FindResult result = Find(key);
		if (result.found && result.valid) {
			return false;
		} else if (result.found && !result.valid) {
			Node* node = result.node;
			node->pointers[result.posInNode] = offset;
			node->validFlags[result.posInNode] = true;
			return true;
		} else {
			Node* node = result.node;
			InsertInVec(node->keys, key, result.posInNode);
			InsertInVec(node->pointers, offset, result.posInNode);
			InsertInVec(node->validFlags, true, result.posInNode);
			if (node->keys.size() == degree) {
				Spilt(node);
			}
			return true;
		}
	}

	bool Delete(const T& key) {
		FindResult result = Find(key);
		if (!result.found) {
			return false;
		} else if (result.found && !result.valid) {
			return false;
		} else {
			Node* node = result.node;
			node->validFlags[result.posInNode] = false;
			return true;
		}
	}

	int FindOffset(const T& key) {
		FindResult result = Find(key);
		if (result.found && result.valid) {
			return result.node->pointers[result.posInNode];
		} else {
			return NULLPTR;
		}
	}

	void Print() {
		Node* currentNode1 = GetNodeViaIndex(rootIndex);
		while (true) {
			Node* currentNode2 = currentNode1;
			while (true) {
				int numOfPointers = (int)currentNode2->pointers.size();
				for (int i = 0; i < numOfPointers; i++) {
					//std::cout << "(" << currentNode2->pointers[i] << ")";
					if (i != numOfPointers - 1 || currentNode2->isLeaf) {
						std::cout << currentNode2->keys[i];
						if (currentNode2->isLeaf) {
							if (currentNode2->validFlags[i]) {
								std::cout << "[T]";
							} else {
								std::cout << "[F]";
							}
						}
						std::cout << ",";
					}
				}
				int nextIndex = currentNode2->nextIndex;
				if (nextIndex != NULLPTR) {
					std::cout << " | ";
					currentNode2 = GetNodeViaIndex(nextIndex);
				} else {
					std::cout << std::endl;
					break;
				}
			}
			if (!currentNode1->isLeaf) {
				int nextIndex = currentNode1->pointers[0];
				currentNode1 = GetNodeViaIndex(nextIndex);
			} else {
				break;
			}
		}
		std::cout << std::endl;
	}

	void SaveSchemaToFile() {
		std::ofstream file(dataFileName + ".schema");
		file << rootIndex << endl;
		file << degree << endl;
		file << numOfNodes << endl;
		file << keySize << endl;
		file.close();
	}

	void Drop() {
		BufferManager::FreeAll();
		remove(dataFileName);
		remove(dataFileName + ".schema");
	}

private:

	int GetDegree(int keySize) {
		//return  4072 / (keySize + 5);
		return 3;
	}

	int CeilDiv(int dividend, int divisor) {
		return (dividend + divisor - 1) / divisor;
	}

	std::pair<bool, int> FindInVec(std::vector<T>& vec, const T& key) {
		int left = 0;
		int right = (int)vec.size() - 1;
		while (left <= right) {
			int middle = (left + right) / 2;
			if (vec[middle] < key) {
				left = middle + 1;
			} else if (vec[middle] > key) {
				right = middle - 1;
			} else {
				return std::make_pair(true, middle);
			}
		}
		return std::make_pair(false, left);
	}

	template <typename U>
	void InsertInVec(std::vector<U>& vec, const U& key, int pos) {
		typename std::vector<U>::iterator iter = vec.begin();
		advance(iter, pos);
		vec.insert(iter, key);
	}

	FindResult Find(const T& key) {
		FindResult result;
		result.found = false;
		result.valid = false;
		int currentNodeIndex = rootIndex;
		while (true) {
			Node* currentNode = GetNodeViaIndex(currentNodeIndex);
			int pointerIndex = 0;
			if (!currentNode->isLeaf) {
				std::pair<bool, int> findInVecResult = FindInVec(currentNode->keys, key);
				if (findInVecResult.first == true) {
					currentNodeIndex = currentNode->pointers[findInVecResult.second + 1];
				} else {
					currentNodeIndex = currentNode->pointers[findInVecResult.second];
				}
			} else {
				result.node = currentNode;
				std::pair<bool, int> findInVecResult = FindInVec(currentNode->keys, key);
				result.found = findInVecResult.first;
				result.posInNode = findInVecResult.second;
				if (result.found) {
					result.valid = currentNode->validFlags[result.posInNode];
				} else {
					result.valid = false;
				}
				break;
			}
		}
		return result;
	}

	void UpdateParentNode(Node* childNode, const T& key) {
		int parentIndex = childNode->parentIndex;
		if (parentIndex == NULLPTR) {
			Node* newRootNode = CreateNode(NULLPTR, NULLPTR, false);
			rootIndex = newRootNode->index;
			childNode->parentIndex = newRootNode->index;
			parentIndex = newRootNode->index;
		}
		Node* parentNode = GetNodeViaIndex(parentIndex);
		std::pair<bool, int> findInVecResult = FindInVec(parentNode->keys, key);
		if (findInVecResult.second != 0 || parentNode->pointers.size() > 0) {
			InsertInVec(parentNode->keys, key, findInVecResult.second);
			InsertInVec(parentNode->pointers, childNode->index, findInVecResult.second + 1);
		} else {
			InsertInVec(parentNode->pointers, childNode->index, findInVecResult.second);
		}
		if ((int)parentNode->pointers.size() > degree) {
			Node* node = parentNode;
			int parentIndex = node->parentIndex;
			int nextIndex = node->nextIndex;

			int numOfPointersInRight = parentNode->pointers.size() / 2;
			int numOfKeysInRight = numOfPointersInRight - 1;
			int numOfPointersInLeft = parentNode->pointers.size() - numOfPointersInRight;
			int numOfKeysInLeft = numOfPointersInLeft - 1;
			T key = node->keys[numOfKeysInLeft];

			//构建右边
			Node* rightNode = CreateNode(parentIndex, nextIndex, false);

			rightNode->keys.resize(numOfKeysInRight);
			rightNode->pointers.resize(numOfPointersInRight);
			std::copy(node->keys.end() - numOfKeysInRight, node->keys.end(), rightNode->keys.begin());
			std::copy(node->pointers.end() - numOfPointersInRight, node->pointers.end(), rightNode->pointers.begin());

			//重建左边
			Node* leftNode = node;
			leftNode->nextIndex = rightNode->index;

			leftNode->keys.resize(numOfKeysInLeft);
			leftNode->pointers.resize(numOfPointersInLeft);

			//右边孩子的parent重设
			for (int childIndex : rightNode->pointers) {
				Node* childNode = GetNodeViaIndex(childIndex);
				childNode->parentIndex = rightNode->index;
			}

			//纵向的重建
			if (leftNode->parentIndex == NULLPTR) {
				UpdateParentNode(leftNode, leftNode->keys[0]);
				rightNode->parentIndex = leftNode->parentIndex;
			}
			UpdateParentNode(rightNode, key);
		}
	}

	void Spilt(Node* node) {
		int parentIndex = node->parentIndex;
		int nextIndex = node->nextIndex;
		//构建右边
		Node* rightNode = CreateNode(parentIndex, nextIndex, true);

		int numOfKeysAndPointerInRight = degree / 2;
		rightNode->keys.resize(numOfKeysAndPointerInRight);
		rightNode->pointers.resize(numOfKeysAndPointerInRight);
		rightNode->validFlags.resize(numOfKeysAndPointerInRight);
		std::copy(node->keys.end() - numOfKeysAndPointerInRight, node->keys.end(), rightNode->keys.begin());
		std::copy(node->pointers.end() - numOfKeysAndPointerInRight, node->pointers.end(), rightNode->pointers.begin());
		std::copy(node->validFlags.end() - numOfKeysAndPointerInRight, node->validFlags.end(), rightNode->validFlags.begin());

		//重建左边
		Node* leftNode = node;
		leftNode->nextIndex = rightNode->index;

		int numOfKeysAndPointerInLeft = degree - numOfKeysAndPointerInRight;
		leftNode->keys.resize(numOfKeysAndPointerInLeft);
		leftNode->pointers.resize(numOfKeysAndPointerInLeft);
		leftNode->validFlags.resize(numOfKeysAndPointerInLeft);

		//纵向的重建
		if (leftNode->parentIndex == NULLPTR) {
			UpdateParentNode(leftNode, leftNode->keys[0]);
			rightNode->parentIndex = leftNode->parentIndex;
		}
		UpdateParentNode(rightNode, rightNode->keys[0]);
	}

	Node* CreateNode(int parentIndex, int nextIndex, int isLeaf) {
		Node* node = new Node(numOfNodes, parentIndex, nextIndex, isLeaf);
		numOfNodes++;
		int** chunkBuf = (int**)BufferManager::GetInstance().Create(dataFileName, node->index, WriteToDiskCallBack);
		BufferManager::GetInstance().SetDirty(dataFileName, node->index);
		*chunkBuf = (int*)node;
		*(chunkBuf + 1) = (int*)this;
		return node;
	}

	Node* GetNodeViaIndex(int index) {
		std::pair<bool, void*> readResult = BufferManager::GetInstance().Read(dataFileName, index, WriteToDiskCallBack);
		bool alreadyProcessed = readResult.first;
		int** chunkBuf = (int**)readResult.second;
		BufferManager::GetInstance().SetDirty(dataFileName, index);
		if (alreadyProcessed) {
			Node* node = (Node*)(*chunkBuf);
			return node;
		} else {
			Node* node = UnpackNodeFromDisk(index, chunkBuf);
			*chunkBuf = (int*)node;
			*(chunkBuf + 1) = (int*)this;
			return node;
		}
	}

	Node* UnpackNodeFromDisk(int index, void* buf) {
		Node* node = new Node(0, NULLPTR, NULLPTR, false);

		char* p = (char*)buf;

		// index
		memcpy(&node->index, p, 4);
		p += 4;
		// parentIndex
		memcpy(&node->parentIndex, p, 4);
		p += 4;
		// nextIndex
		memcpy(&node->nextIndex, p, 4);
		p += 4;
		// isLeaf
		char isLeaf;
		memcpy(&isLeaf, p, 1);
		p += 1;
		node->isLeaf = (isLeaf == 1) ? true : false;
		// numOfKeys
		int numOfKeys;
		memcpy(&numOfKeys, p, 4);
		p += 4;
		node->keys.resize(numOfKeys);
		// numOfPointers
		int numOfPointers;
		memcpy(&numOfPointers, p, 4);
		p += 4;
		node->pointers.resize(numOfPointers);
		// numOfValidFlags
		int numOfValidFlags;
		memcpy(&numOfValidFlags, p, 4);
		p += 4;
		node->validFlags.resize(numOfValidFlags);
		// minKeyInLeaves
		memcpy(&node->minKeyInLeaves, p, keySize);
		p += keySize;

		// keys
		for (int i = 0; i < numOfKeys; i++) {
			T key;
			memcpy(&key, p, keySize);
			p += keySize;
			node->keys[i] = key;
		}

		// pointers
		for (int i = 0; i < numOfPointers; i++) {
			int pointer;
			memcpy(&pointer, p, 4);
			p += 4;
			node->pointers[i] = pointer;
		}

		// validFlags
		for (int i = 0; i < numOfValidFlags; i++) {
			char chValidFlag;
			memcpy(&chValidFlag, p, 1);
			p += 1;
			node->validFlags[i] = (chValidFlag == 1) ? true : false;
		}

		return node;
	}

	void PackNodeToDisk(int index, void* buf) {
		char* p = (char*)buf;

		Node* node = GetNodeViaIndex(index);

		// index
		memcpy(p, &node->index, 4);
		p += 4;
		// parentIndex
		memcpy(p, &node->parentIndex, 4);
		p += 4;
		// nextIndex
		memcpy(p, &node->nextIndex, 4);
		p += 4;
		// isLeaf
		char isLeaf = node->isLeaf ? 1 : 0;
		memcpy(p, &isLeaf, 1);
		p += 1;
		// numOfKeys
		int numOfKeys = node->keys.size();
		memcpy(p, &numOfKeys, 4);
		p += 4;
		// numOfPointers
		int numOfPointers = node->pointers.size();
		memcpy(p, &numOfPointers, 4);
		p += 4;
		// numOfValidFlags
		int numOfValidFlags = node->validFlags.size();
		memcpy(p, &numOfValidFlags, 4);
		p += 4;
		// minKeyInLeaves
		memcpy(p, &node->minKeyInLeaves, keySize);
		p += keySize;
		// keys
		for (const T& key : node->keys) {
			memcpy(p, &key, keySize);
			p += keySize;
		}
		// pointers
		for (const int& pointer : node->pointers) {
			memcpy(p, &pointer, 4);
			p += 4;
		}
		// validFlags
		for (const bool& validFlag : node->validFlags) {
			char chValidFlag = validFlag ? 1 : 0;
			memcpy(p, &chValidFlag, 1);
			p += 1;
		}

		delete node;
	}

private:

	static void WriteToDiskCallBack(std::string fileName, int block, void* buf) {
		char* p = (char*)buf;
		BPlusTree* bpTree;
		memcpy(&bpTree, p + 4, 4);
		bpTree->PackNodeToDisk(block, buf);
	}

private:

	std::string dataFileName;

	int rootIndex;
	int degree;
	int numOfNodes;
	int keySize;

public:

	static const int NULLPTR = -1;
};

