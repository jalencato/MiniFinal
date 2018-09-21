#pragma once

#include<string>
#include<vector>
#include<stdio.h>
#include<cassert>

using std::string;
using std::vector;
using std::stringstream;
using std::noskipws;

enum {
	INT,
	DOUBLE,
	STRING
};

struct Num {
	int type;
	union {
		int _int;
		float _float;
		int size;
	}data;
	string str;

	Num() {};
	Num(int i) :type(INT) {
		data._int = i;
	}
	Num(float i) :type(DOUBLE) {
		data._float = i;
	}
	Num(string _str, size_t size_) :type(STRING), str(_str) {
		data.size = (int)size_;
	}

	bool operator==(Num& num) {
		bool res = true;
		if (type == INT) {
			res = data._int == num.data._int;
		} else if (type == DOUBLE) {
			res = data._float == num.data._float;
		} else if (type == STRING) {
			res = str == num.str;
		} else {}
		return res;
	}

	bool operator <(Num& num) {
		bool res = true;
		if (type == INT) {
			res = data._int < num.data._int;
		} else if (type == DOUBLE) {
			res = data._float < num.data._float;
		} else if (type == STRING) {
			res = str < num.str;
		} else {}
		return res;
	}
};

struct Value {
	string valName;
	int type;
	int size;
	bool unique;
	bool primary;
	bool index;
	string indexName;

	Value() {}

	Value(string valName, int type, int size, bool unique, bool index, bool primary) :
		valName(valName), type(type), size(size), unique(unique), index(index), primary(primary), indexName("_") {
	}
};

struct Table {
	string tableName;
	int _size;
	vector<Value> attriList;
	unsigned int offset;

	Table() {}
	Table(string& tableName, vector<Value>& attriList)
		:tableName(tableName), attriList(attriList), offset(0), _size(0) {
		for (auto& c : attriList)
			_size += c.size;
	}

};




