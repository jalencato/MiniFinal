#pragma once
#include<string>
#include<vector>
#include<sstream>
#include"tableManager.h"
#include "RecordManager.h"

class API
{
public:
	API();
	~API();

	void dropTable(string tableName);
	void createTable(string tableName,vector<Value>* valVector);

	void dropIndex(string indexName);
	void createIndex(string indexName, string tableName, string valName);

	void InsertRecord(string tableName,vector<string>* valVector);
	void deleteRecord(string tableName,vector<Judge>* judgeVec);

	void selectRecord(string tableName, vector<string>* valSelect, vector<Judge>* vecJudge);

	Value& getValue(string tableName,string valName);

	void print(Res res, vector<string>* valNameVector, vector<int>* valSelectNo);

	bool isInt(string str);
	bool isFloat(string str);

	bool isUnique(string tableName, Value val, Num num);
};

