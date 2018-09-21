#define _CRT_SECURE_NO_WARNINGS
#include "IndexManager.h"
#include "RecordManager.h"
#include "CatalogManager.h"


void IndexManager::createIndex(const string indexName, int type, int size) {
	string fileName = indexName + ".index";

	//type size
	int keySize = 0;
	if (type == INT) keySize = sizeof(int);
	else if (type == DOUBLE) keySize = sizeof(float);
	else if (type == STRING) keySize = size + 1;

	if (type == INT) {
		Zzh::BPlusTree<int>* bpTree = new Zzh::BPlusTree<int>(fileName, keySize);
		intIndex[indexName] = bpTree;
	} else if (type == DOUBLE) {
		Zzh::BPlusTree<float>* bpTree = new Zzh::BPlusTree<float>(fileName, keySize);
		floatIndex[indexName] = bpTree;
	} else if (type == STRING) {
		Zzh::BPlusTree<Zzh::FixedLengthChar>* bpTree = new Zzh::BPlusTree<Zzh::FixedLengthChar>(fileName, keySize);
		stringIndex[indexName] = bpTree;
	}
}

void IndexManager::insertIndex(string indexName, int val, int blockOffset, int type) {
	intMap::iterator iterInt = intIndex.find(indexName);
	if (iterInt == intIndex.end()) {
		cout << "no found about the float key" << endl;
		return;
	} else iterInt->second->Insert(val, blockOffset);
}

void IndexManager::insertIndex(string indexName, float val, int blockOffset, int type) {
	string filePath = indexName;
	floatMap::iterator iterfloat = floatIndex.find(filePath);
	if (iterfloat == floatIndex.end()) {
		cout << "no found about the float key" << endl;
		return;
	} else iterfloat->second->Insert(val, blockOffset);
}

void IndexManager::insertIndex(string indexName, string val, int blockOffset, int type) {
	string filePath = indexName;
	stringMap::iterator iterstring = stringIndex.find(filePath);
	if (iterstring == stringIndex.end()) {
		return;
	} else iterstring->second->Insert(val.c_str(), blockOffset);
}

void IndexManager::deleteIndex(string indexName, int type) {
	if (type == INT) {
		auto iter = intIndex.find(indexName);
		iter->second->Drop();
		delete iter->second;
		intIndex.erase(iter);
	} else if (type == DOUBLE) {
		auto iter = floatIndex.find(indexName);
		iter->second->Drop();
		delete iter->second;
		floatIndex.erase(iter);
	} else if (type == STRING) {
		auto iter = stringIndex.find(indexName);
		iter->second->Drop();
		delete iter->second;
		stringIndex.erase(iter);
	}
}

void IndexManager::deleteIndex(string indexName, int val, int type) {
	string filePath = indexName;
	intMap::iterator iterInt = intIndex.find(filePath);
	if (iterInt == intIndex.end()) return;
	iterInt->second->Delete(val);
}

void IndexManager::deleteIndex(string indexName, float val, int type) {
	string filePath = indexName;
	floatMap::iterator iterfloat = floatIndex.find(filePath);
	if (iterfloat == floatIndex.end()) return;
	iterfloat->second->Delete(val);
}

void IndexManager::deleteIndex(string indexName, string val, int type) {
	string filePath = indexName;
	stringMap::iterator iterstring = stringIndex.find(filePath);
	if (iterstring == stringIndex.end()) return;
	iterstring->second->Delete(val.c_str());
}

int IndexManager::searchIndex(SS s) {
	if (s.type == INT) {
		return searchIndex(s.indexName, s.data._int, INT);
	} else if (s.type == DOUBLE) {
		return searchIndex(s.indexName, s.data._float, DOUBLE);
	} else if (s.type == STRING) {
		return searchIndex(s.indexName, s.str, STRING);
	}
	return -1;
}

int IndexManager::searchIndex(string indexName, int val, int type) {
	string filePath = indexName;
	intMap::iterator iterInt = intIndex.find(filePath);
	if (iterInt == intIndex.end()) return -1;
	else return iterInt->second->FindOffset(val);
}

int IndexManager::searchIndex(string indexName, float val, int type) {
	string filePath = indexName;
	floatMap::iterator iterfloat = floatIndex.find(filePath);
	if (iterfloat == floatIndex.end()) return -1;
	else return iterfloat->second->FindOffset(val);
}

int IndexManager::searchIndex(string indexName, string val, int type) {
	string filePath = indexName;
	stringMap::iterator iterstring = stringIndex.find(filePath);
	if (iterstring == stringIndex.end()) return -1;
	else return iterstring->second->FindOffset(val.c_str());
}

void IndexManager::freeMem() {
	for (auto& iterInt : intIndex) {
		if (iterInt.second) {
			iterInt.second->SaveAll();
			delete iterInt.second;
		}
	}
	for (auto& iterfloat : floatIndex) {
		if (iterfloat.second) {
			iterfloat.second->SaveAll();
			delete iterfloat.second;
		}
	}
	for (auto& iterstring : stringIndex) {
		if (iterstring.second) {
			iterstring.second->SaveAll();
			delete iterstring.second;
		}
	}
}

