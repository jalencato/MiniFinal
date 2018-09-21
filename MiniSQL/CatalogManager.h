#pragma once
#include "tableManager.h"
#include <map>
#include<fstream>
#include<iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::pair;
using std::make_pair;
using std::ifstream;
using std::ofstream;

class CatalogManager
{
public:
	static CatalogManager& instance()
	{
		static CatalogManager instance;
		return instance;
	}
	CatalogManager();
	~CatalogManager();

	Table& getTable(string tableName);

	Table& createTable(string tableName, vector<Value>&valVector);

	bool hasTable(string tableName);

	void deleteTable(string tableName);

	bool hasIndex(string IndexName);

	pair<string, string> mapIndex(string indexName);

	void createIndex(string indexName, string tableName, string valName);

	void deleteIndex(string indexName);
private:
	map<string, Table> tableMap;
	map<string, pair<string, string>>indexMap;
};

