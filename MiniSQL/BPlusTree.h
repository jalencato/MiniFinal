#pragma once
#include<vector>
#include "BufferManager.h"
using namespace std;

template<class T>
class TreeNode {
public:
	unsigned int keyValSize;
	int degree;//capacity
	TreeNode* parent;

	vector<T> keys;
	vector<TreeNode*>children;
	vector<int> value;
	TreeNode* nextpeer;
	bool is_leaf;

	TreeNode(int degree, bool is_leaf);

	bool isRoot();
	bool search(T val, unsigned& index);
	TreeNode* split(T& val);

	unsigned int add_val(T& key, int val);
	unsigned int add_val(T& key);
	bool deleteElement(unsigned int val);
};

template <class T>
class BPlusTree {
	typedef TreeNode<T>* BNodePtr;

	struct searchSupport {
		BNodePtr ptr;
		unsigned int index;
		bool is_find;
	};

public:
	BPlusTree(string name, int keySize, int degree);
	~BPlusTree();

	int search(T& key);
	T searchKey(T& key);
	bool _insert(T& key, int offset);
	bool splayInsert(BNodePtr ptr);
	void findUntilLeaf(BNodePtr ptr, T key, searchSupport &ss);
	bool _delete(T& key);
	bool splayDelete(BNodePtr ptr);

	void freeTree(BNodePtr ptr);

	void readAllFromDisk();
	void readFromOneBlock(Block* tmp);
	void writeAllBackToDisk();

	void init_tree();

private:
	string fileName;
	BNodePtr root;
	BNodePtr leaf;//horizontol start
	unsigned int level;
	unsigned int keyCount;
	unsigned int nodeCount;
	int keySize;
	int degree;
};

template <class T>
TreeNode<T>::TreeNode(int degree, bool is_leaf) :
	keyValSize(0), parent(nullptr), is_leaf(is_leaf), degree(degree), nextpeer(nullptr) {
	//initialize all the elements with the null
	for (int i = 0; i < degree + 1; i++) {
		children.push_back(nullptr);
		keys.push_back(T());
		value.push_back(int());
	}
	children.push_back(nullptr);
}

template <class T>
bool TreeNode<T>::isRoot() {
	//root has no parent
	if (parent == nullptr)
		return true;
	else return false;
}

//search in one node 
template <class T>
bool TreeNode<T>::search(T val, unsigned int& index) {
	//return the first value that is bigger
	//no value stored
	if (keyValSize == 0) {
		index = 0;
		return false;
	}

	if (keys[keyValSize - 1] < val)//largest one
	{
		index = keyValSize;
		return false;
	}

	if (keys[0] > val)//smallest one
	{
		index = 0;
		return false;
	}

	if (keyValSize <= 20)//not too many values
	{
		for (unsigned int i = 0; i < keyValSize; i++) {
			if (keys[i] == val) {
				index = i; return true;
			}
			if (keys[i] < val) {
				continue;
			}
			if (keys[i] > val) {
				index = i;
				return false;
			}
		}
	} else {
		//more than 20 values
		int left = 0;
		int right = keyValSize - 1;
		int mid = 0;

		while (right - left > 1) {
			mid = (left + right) / 2;
			if (keys[mid] == val) {
				index = mid;
				return true;
			} else if (keys[mid] <= val) {
				left = mid;
			} else if (keys[mid] >= val) {
				right = mid;
			}
		}

		//no found
		if (keys[left] >= val) {
			index = left;
			if (keys[left] == val)return true;
			else return false;
		}
		if (keys[right] >= val) {
			index = right;
			if (keys[right] == val)return true;
			else return false;
		}
		if (keys[right] <= val) {
			index = right + 1;
			return false;
		}
	}
	return false;
}

template <class T>
TreeNode<T>* TreeNode<T>::split(T& val) {
	const unsigned int lower_num = (degree - 1) / 2;

	//new node store the same
	TreeNode* Node = new TreeNode(degree, is_leaf);

	if (is_leaf) {
		val = keys[lower_num + 1];
		for (int i = lower_num + 1; i < degree; i++) {
			Node->keys[i - lower_num - 1] = keys[i];//move lower_num + 1
			keys[i] = T();//reset the oringinal value
			Node->value[i - lower_num - 1] = value[i];//move lower_num + 1
			value[i] = int();//reset the original value
		}
		Node->nextpeer = this->nextpeer;
		this->nextpeer = Node;

		Node->parent = this->parent;
		Node->keyValSize = lower_num;//true for 2k and 2k+1
		this->keyValSize = lower_num + 1;
	} else {
		val = keys[lower_num];
		for (int i = lower_num + 1; i < degree + 1; i++) {
			//rearrange the children
			Node->children[i - lower_num - 1] = this->children[i];
			Node->children[i - lower_num - 1]->parent = Node;
			this->children[i] = nullptr;
		}
		for (int i = lower_num + 1; i < degree; i++) {
			Node->keys[i - lower_num - 1] = this->keys[i];
			this->keys[i] = T();
		}
		this->keys[lower_num] = T();

		Node->parent = this->parent;
		Node->keyValSize = lower_num;
		this->keyValSize = lower_num;
	}
	return Node;
}

template <class T>//for leaf
unsigned int TreeNode<T>::add_val(T& key, int val) {
	if (!is_leaf) {
		cerr << "Error" << endl;
		return -1;
	}
	if (keyValSize == 0) {
		keys[0] = key;
		value[0] = val;
		keyValSize++;
		return 0;
	}
	unsigned int index = 0;
	bool is_find = search(key, index);
	if (is_find) {
		cerr << "Already existing" << endl;
		exit(0);
	}
	//moving backwards
	for (unsigned int i = keyValSize; i > index; i--) {
		keys[i] = keys[i - 1];
		value[i] = value[i - 1];
	}
	keys[index] = key;
	value[index] = val;
	keyValSize++;
	return index;
}

template <class T>//for non-leaf
unsigned TreeNode<T>::add_val(T& key) {
	if (keyValSize == 0) {
		keys[0] = key;
		keyValSize++;
		return 0;
	}
	unsigned int index = 0;
	bool is_find = search(key, index);
	if (is_find) {
		cerr << "Already existing" << endl;
		exit(0);
	}
	for (unsigned int i = keyValSize; i > index; i--)
		keys[i] = keys[i - 1];
	keys[index] = key;

	for (unsigned int i = keyValSize + 1; i > index + 1; i--)//the children index is one more
		children[i] = children[i - 1];
	children[index + 1] = nullptr;
	keyValSize++;

	return index;
}

template <class T>//moving one forward is okay
bool TreeNode<T>::deleteElement(unsigned val) {
	if (val > keyValSize) {
		cerr << "Too long index" << endl;
		return false;
	}
	if (is_leaf) {
		for (unsigned int i = val; i < keyValSize - 1; i++) {
			keys[i] = keys[i + 1];//move one forward
			value[i] = value[i + 1];
		}
		keys[keyValSize - 1] = T();//reinitialize
		value[keyValSize - 1] = int();
	} else {
		for (unsigned int i = val; i < keyValSize - 1; i++) {
			keys[i] = keys[i + 1];
		}
		for (unsigned int i = val + 1; i < keyValSize; i++)
			children[i] = children[i + 1];

		keys[keyValSize - 1] = T();
		children[keyValSize] = nullptr;
	}
	keyValSize--;
	return true;
}

template <class T>
BPlusTree<T>::BPlusTree(string name, int keySize, int degree) :
	fileName(name), keyCount(0), level(0), nodeCount(0), root(nullptr),
	leaf(nullptr), keySize(keySize), degree(degree) {
	init_tree();
	readAllFromDisk();
}

template <class T>
BPlusTree<T>::~BPlusTree() {
	freeTree(root);
	nodeCount = 0;
	keyCount = 0;
	root = nullptr;
	level = 0;
}

template <class T>
int BPlusTree<T>::search(T& key) {
	if (root == nullptr)
		return -1;
	searchSupport ss;//nodeptr index is_find
	findUntilLeaf(root, key, ss);
	if (!ss.is_find)return -1;
	else return ss.ptr->value[ss.index];
}

template <class T>
T BPlusTree<T>::searchKey(T& key) {
	if (root == nullptr)
		return -1;
	searchSupport ss;//nodeptr index is_find
	findUntilLeaf(root, key, ss);

	auto findEnd = ss.ptr;
	if (!ss.is_find)return -1;
	else return ss.ptr->keys[ss.index];
}

template <class T>
bool BPlusTree<T>::_insert(T& key, int offset) {
	searchSupport ss;
	if (root == nullptr)
		init_tree();//additional judge
	unsigned int index = 0;
	findUntilLeaf(root, key, ss);
	if (ss.is_find) {
		cerr << "Already exists" << endl;//no duplicate value
		return false;
	}
	ss.ptr->add_val(key, offset);
	if (ss.ptr->keyValSize == degree)
		splayInsert(ss.ptr);

	keyCount++;
	return true;
}

template <class T>//split
bool BPlusTree<T>::splayInsert(BNodePtr ptr) {
	T key;
	auto node = ptr->split(key);//the first node in the generated
	nodeCount++;//after split one more node is added

	if (ptr->isRoot())//root can have 2-M orders & a new root is needed
	{
		BNodePtr root = new TreeNode<T>(degree, false);
		level++;//max level
		nodeCount++;
		this->root = root;
		ptr->parent = root;
		node->parent = root;

		root->add_val(key);
		root->children[0] = ptr;
		root->children[1] = node;
		return true;
	} else {//pop upward
		BNodePtr par = node->parent;
		unsigned int index = par->add_val(key);

		par->children[index + 1] = node;
		node->parent = par;
		if (par->keyValSize == degree)
			return splayInsert(par);

		return true;
	}
	return false;
}

template <class T>
void BPlusTree<T>::findUntilLeaf(BNodePtr ptr, T key, searchSupport& ss) {
	unsigned int index = 0;
	//index is the found node
	//or the last one that is smaller than the found with the number
	if (ptr->search(key, index))//find in the first node first
	{
		if (ptr->is_leaf) {
			ss.ptr = ptr;
			ss.index = index;
			ss.is_find = true;
		} else//have already stored in the index
		{
			ptr = ptr->children[index + 1];
			while (!ptr->is_leaf)
				ptr = ptr->children[0];//it must be the first one
			ss.ptr = ptr;
			ss.index = 0;
			ss.is_find = true;
		}
	} else {
		if (ptr->is_leaf)//to the end
		{
			ss.ptr = ptr;
			ss.index = index;
			ss.is_find = false;
		} else findUntilLeaf(ptr->children[index], key, ss);//children: the one that is in front of the first big one
	}
}

template <class T>
bool BPlusTree<T>::_delete(T& key) {
	searchSupport ss;
	if (!root) {
		cerr << "Error no nodes in the tree" << endl;
		return false;
	} else {
		findUntilLeaf(root, key, ss);
		if (!ss.is_find) {
			cerr << "Error no keys" << endl;
			return false;
		} else {
			if (ss.ptr->isRoot()) {
				ss.ptr->deleteElement(ss.index);
				keyCount--;
				return splayDelete(ss.ptr);
			} else {
				if (ss.index == 0 && leaf != ss.ptr)//the first one does not store any relational data
				{
					unsigned int index = 0;

					BNodePtr parIter = ss.ptr->parent;
					bool is = parIter->search(key, index);
					while (!is)//delete all the keys in the B+ tree
					{
						if (parIter->parent)
							parIter = parIter->parent;
						else break;
						is = parIter->search(key, index);
					}

					parIter->keys[index] = ss.ptr->keys[1];

					ss.ptr->deleteElement(ss.index);
					keyCount--;
					return splayDelete(ss.ptr);
				} else {
					ss.ptr->deleteElement(ss.index);
					keyCount--;
					return splayDelete(ss.ptr);
				}
			}
		}
	}
}

template <class T>
bool BPlusTree<T>::splayDelete(BNodePtr ptr) {
	unsigned int minimumkey = (degree - 1) / 2;
	if (((ptr->is_leaf) && (ptr->keyValSize >= minimumkey)) ||
		((degree != 3) && (!ptr->is_leaf) && (ptr->keyValSize >= minimumkey - 1))
		|| ((degree == 3) && (!ptr->is_leaf) && (ptr->keyValSize < 0)))
		return true;
	//not too less keys
	//minimumkey != 0
	//minimumkey == 0

	//need to merge
	if (ptr->isRoot()) {
		if (ptr->keyValSize > 0)
			return true;
		else {//keyValSize == 0
			if (root->is_leaf)//no nodes reamaining
			{
				delete ptr;
				root = NULL;
				leaf = NULL;
				level--;
				nodeCount--;
			} else {
				root = ptr->children[0];//the only child remaining
				root->parent = NULL;
				delete ptr;
				level--;
				nodeCount--;
			}
		}
	} else {
		BNodePtr par = ptr->parent;
		BNodePtr brother = nullptr;
		if (ptr->is_leaf) {
			unsigned int index = 0;
			par->search(ptr->keys[0], index);//pop off

			if ((par->children[0] != ptr) && (index + 1) == par->keyValSize)
				//left node is available to merge
			{
				brother = par->children[index];
				if (brother->keyValSize > minimumkey)//can not merge directly with move one from the brother to the node
				{
					for (int i = ptr->keyValSize; i > 0; i--) {
						ptr->keys[i] = ptr->keys[i - 1];
						ptr->value[i] = ptr->value[i - 1];
					}
					//move the rightmost node to the node
					ptr->keys[0] = brother->keys[brother->keyValSize - 1];
					ptr->value[0] = brother->value[brother->keyValSize - 1];
					brother->deleteElement(brother->keyValSize - 1);

					++ptr->keyValSize;
					par->keys[index] = ptr->keys[0];//change the index for changing the leftmost value
					return true;
				} else//merge with the same peer
				{
					par->deleteElement(index);

					//merge with the left one
					for (int i = 0; i < (int)ptr->keyValSize; i++) {
						brother->keys[brother->keyValSize + i] = ptr->keys[i];
						brother->value[brother->keyValSize + i] = ptr->value[i];
					}

					brother->keyValSize += ptr->keyValSize;
					brother->nextpeer = ptr->nextpeer;

					delete ptr;
					nodeCount--;

					return splayDelete(par);
				}
			} else {
				//choosing the right one to merge
				if (par->children[0] == ptr)//need to delete upwards
					brother = par->children[1];//won't return -1
				else
					brother = par->children[index + 2];
				//the others remain the same
				if (brother->keyValSize > minimumkey) {
					ptr->keys[ptr->keyValSize] = brother->keys[0];
					ptr->value[ptr->keyValSize] = brother->value[0];
					++ptr->keyValSize;
					brother->deleteElement(0);
					if (par->children[0] == ptr)
						par->keys[0] = brother->keys[0];
					else
						par->keys[index + 1] = brother->keys[0];
					return true;
				} else {
					for (int i = 0; i < (int)brother->keyValSize; i++) {
						ptr->keys[ptr->keyValSize + i] = brother->keys[i];
						ptr->value[ptr->keyValSize + i] = brother->value[i];
					}
					if (ptr == par->children[0])
						par->deleteElement(0);
					else
						par->deleteElement(index + 1);
					ptr->keyValSize += brother->keyValSize;
					ptr->nextpeer = brother->nextpeer;

					delete brother;
					nodeCount--;

					return splayDelete(par);
				}
			}
		} else//not leaf
		{
			unsigned int index = 0;
			par->search(ptr->children[0]->keys[0], index);//it must be the same value from the children leftmost
			if ((par->children[0] != ptr) && (index + 1 == par->keyValSize))//merge with the left
			{
				brother = par->children[index];
				if (brother->keyValSize > minimumkey - 1)//move one or move all
				{
					ptr->children[ptr->keyValSize + 1] = ptr->children[ptr->keyValSize];
					for (unsigned int i = ptr->keyValSize; i > 0; i--) {
						ptr->children[i] = ptr->children[i - 1];
						ptr->keys[i] = ptr->keys[i - 1];
					}
					ptr->children[0] = brother->children[brother->keyValSize];
					ptr->keys[0] = par->keys[index];
					++ptr->keyValSize;

					par->keys[index] = brother->keys[brother->keyValSize - 1];
					if (brother->children[brother->keyValSize])
						brother->children[brother->keyValSize]->parent = ptr;
					brother->deleteElement(brother->keyValSize - 1);

					return true;
				} else {
					brother->keys[brother->keyValSize] = par->keys[index];
					par->deleteElement(index);
					++brother->keyValSize;

					for (int i = 0; i < (int)ptr->keyValSize; i++) {
						brother->children[brother->keyValSize + i] = ptr->children[i];
						brother->keys[brother->keyValSize + i] = ptr->keys[i];
						brother->children[brother->keyValSize + i]->parent = brother;
					}
					brother->children[brother->keyValSize + ptr->keyValSize] = ptr->children[ptr->keyValSize];
					brother->children[brother->keyValSize + ptr->keyValSize]->parent = brother;

					brother->keyValSize += ptr->keyValSize;

					delete ptr;
					nodeCount--;

					return splayDelete(par);
				}
			} else//merge with the right
			{
				if (par->children[0] == ptr)
					brother = par->children[1];
				else
					brother = par->children[index + 2];
				if (brother->keyValSize > minimumkey - 1) {
					ptr->children[ptr->keyValSize + 1] = brother->children[0];
					ptr->keys[ptr->keyValSize] = brother->keys[0];
					ptr->children[ptr->keyValSize + 1]->parent = ptr;
					++ptr->keyValSize;
					if (ptr == par->children[0])
						par->keys[0] = brother->keys[0];
					else par->keys[index + 1] = brother->keys[0];
					brother->children[0] = brother->children[1];
					brother->deleteElement(0);

					return true;
				} else {
					ptr->keys[ptr->keyValSize] = par->keys[index];
					if (ptr == par->children[0])
						par->deleteElement(0);
					else par->deleteElement(index + 1);

					++ptr->keyValSize;

					for (int i = 0; i < (int)brother->keyValSize; i++) {
						ptr->children[ptr->keyValSize + i] = brother->children[i];
						ptr->keys[ptr->keyValSize + i] = brother->keys[i];
						ptr->children[ptr->keyValSize + i]->parent = ptr;
					}
					ptr->children[ptr->keyValSize + brother->keyValSize] = brother->children[brother->keyValSize];
					ptr->children[ptr->keyValSize + brother->keyValSize]->parent = ptr;

					ptr->keyValSize += brother->keyValSize;

					delete brother;
					nodeCount--;

					return splayDelete(par);
				}
			}
		}
	}
	return false;
}

template <class T>
void BPlusTree<T>::freeTree(BNodePtr ptr) {
	if (root == nullptr)return;
	else if (root->is_leaf == false) {
		for (unsigned int i = 0; i <= ptr->keyValSize; i++) {
			freeTree(ptr->children[i]);
			ptr->children[i] = nullptr;
		}
	}
	delete ptr;
	nodeCount--;
}

template <class T>
void BPlusTree<T>::readAllFromDisk() {
	auto& bi = BufferManager::instance();
	unsigned int offset = 0;
	Block* blk;
	while (true) {
		blk = bi.readBlock(fileName, offset);//bplustree name
		auto tmp = new unsigned char;
		memcpy(tmp, &blk->buf[0], sizeof(int));//copy the size value
		auto bSize = reinterpret_cast<int*>(tmp);
		blk->size = *bSize;
		if (blk->size == 0)break;
		readFromOneBlock(blk);
		offset += 4096;
	}
}

template <class T>
void BPlusTree<T>::readFromOneBlock(Block* tmp) {
	unsigned char* bufTmp = tmp->buf;

	int valueSize = sizeof(int);

	//store the offset and the value
	unsigned char* indexStart = bufTmp + sizeof(int);
	unsigned char* valueStart = indexStart + keySize;
	unsigned char* indexIter = indexStart;
	unsigned char* valueIter = valueStart;

	T key;
	int val;

	while (valueIter - indexStart < tmp->size) {
		key = *(T*)indexIter;
		val = *(int*)(valueIter);
		_insert(key, val);
		valueIter += keySize + valueSize;
		indexIter += keySize + valueSize;
	}
}

template <class T>
void BPlusTree<T>::writeAllBackToDisk() {
	auto& bi = BufferManager::instance();
	TreeNode<T>* tmp = leaf;
	unsigned int offset = 0;
	Block* blk = bi.readBlock(this->fileName, offset);
	while (tmp != nullptr) {
		blk->size = 0;
		blk->file_name = this->fileName;
		bi.setDirty(blk);
		int valueSize = sizeof(int);
		for (int i = 0; i < (int)tmp->keyValSize; i++) {
			auto keyPtr = reinterpret_cast<unsigned char*>(&(tmp->keys[i]));
			auto valPtr = reinterpret_cast<unsigned char*>(&(tmp->value[i]));
			memcpy(&blk->buf[sizeof(int) + i * (keySize + valueSize)], keyPtr, keySize);
			memcpy(&blk->buf[sizeof(int) + i * (keySize + valueSize) + keySize], valPtr, valueSize);
			blk->size += keySize + valueSize;
		}
		memcpy(&blk->buf[0], &blk->size, sizeof(int));
		offset += 4096;
		blk = bi.readBlock(this->fileName, offset);
		tmp = tmp->nextpeer;
	}
}

template <class T>
void BPlusTree<T>::init_tree() {
	root = new TreeNode<T>(degree, true);
	keyCount = 0;
	level = 1;
	nodeCount = 1;//root only
	leaf = root;
}