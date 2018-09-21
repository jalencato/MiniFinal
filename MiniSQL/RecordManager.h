#pragma once
#include"BufferManager.h"
#include"tableManager.h"

typedef vector<Num> Record;
typedef vector<Record> Res;

struct Judge
{
	enum OPERATION {
		EQUAL,
		NOTEQUAL,
		LESS,
		GREATER,
		LESSOREQUAL,
		GREATEROREQUAL,
	};
	string valName;
	Num value;
	OPERATION operation;

	Judge(string valName, Num value, OPERATION operation)
		:valName(valName), value(value), operation(operation)
	{}
};

class RecordManager
{
public:
	RecordManager();
	~RecordManager();

	static RecordManager& instance()
	{
		static RecordManager instance;
		return instance;
	}

	bool check(Num& num, Judge& cond);

	void createTableName(string tableName);

	void deleteTableName(string tableName);

	void insertRecord(string tableName, Record& record);

	int deleteRecord(const string tableName, vector<Judge>& conditions);

	Res searchRecord(string tableName, vector<Judge>& judgement);
	Res searchIndex(string tableName, vector<Judge>& judgement);
	Res selectAllRecord(string tableName);
};

