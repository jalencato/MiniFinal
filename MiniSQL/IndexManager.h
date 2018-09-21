#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "Zzh.h"
#include"tableManager.h"
#include "RecordManager.h"

struct SS {
	string indexName;
	int type;
	union {
		int _int;
		float _float;
	}data;
	string str;

	SS() {};
	SS(string indexName, int i) :indexName(indexName), type(INT) {

		data._int = i;
	}
	SS(string indexName, float i) :indexName(indexName), type(DOUBLE) {
		data._float = i;
	}
	SS(string indexName, string _str) :indexName(indexName), type(STRING), str(_str) {
	}
	SS(string indexName, Num num) :indexName(indexName), type(num.type) {
		if (type == INT)
			data._int = num.data._int;
		else if (type == DOUBLE)
			data._float = num.data._float;
		else if (type == STRING)
			str = num.str;
	}
};

class IndexManager
{
public:
	static IndexManager& instance()
	{
		static IndexManager instance;
		return instance;
	}

	void freeMem();

	void createIndex(const string indexName,int type,int size);

	void insertIndex(string indexName, int val, int blockOffset, int type);
	void insertIndex(string indexName, float val, int blockOffset, int type);
	void insertIndex(string indexName, string val, int blockOffset, int type);

	void deleteIndex(string indexName, int type);
	void deleteIndex(string indexName, int val, int type);
	void deleteIndex(string indexName, float val, int type);
	void deleteIndex(string indexName, string val, int type);

	int searchIndex(SS s);
	int searchIndex(string indexName, int val,int type);
	int searchIndex(string indexName, float val, int type);
	int searchIndex(string indexName, string val, int type);

	using intMap = map<string, Zzh::BPlusTree<int>*>;
	using floatMap = map<string, Zzh::BPlusTree<float>*>;
	using stringMap = map<string, Zzh::BPlusTree<Zzh::FixedLengthChar>*>;
	intMap  intIndex;
	floatMap floatIndex;
	stringMap stringIndex;
	
private:
	IndexManager() {}

};

