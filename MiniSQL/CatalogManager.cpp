#define _CRT_SECURE_NO_WARNINGS
#include "CatalogManager.h"
#include "IndexManager.h"

CatalogManager::CatalogManager()
{

}

CatalogManager::~CatalogManager()
{
	for(auto& c:tableMap)
	{
		ofstream out(c.second.tableName + ".cal");

		out << c.second.tableName << " " << c.second._size << " " << c.second.offset << " ";
		out << static_cast<int>(c.second.attriList.size()) << " ";
		for(auto ci:c.second.attriList)
		{
			out << ci.valName << " " << ci.type << " " << ci.size << " "
				<< ci.unique << " " << ci.index << " "
				<< ci.indexName << " " << ci.primary << " ";
		}
		out.close();
	}
}

Table& CatalogManager::getTable(string tableName)
{
	if(!static_cast<bool>(tableMap.count(tableName)))
	{
		Table table;
		ifstream in(tableName + ".cal");
		int valCount;

		in >> table.tableName >> table._size >> table.offset;
		in >> valCount;
		for (int i = 0; i < valCount; i++)
		{
			Value val;
			in >> val.valName >> val.type >> val.size >>
				val.unique >> val.index >> val.indexName >> val.primary;
			table.attriList.push_back(val);
			if (val.index) {
				string fileName = val.indexName + ".index";
				if (val.type == INT) {
					Zzh::BPlusTree<int>* bpTree = new Zzh::BPlusTree<int>(fileName);
					IndexManager::instance().intIndex[val.indexName] = bpTree;
				} else if (val.type == DOUBLE) {
					Zzh::BPlusTree<float>* bpTree = new Zzh::BPlusTree<float>(fileName);
					IndexManager::instance().floatIndex[val.indexName] = bpTree;
				} else if (val.type == STRING) {
					Zzh::BPlusTree<Zzh::FixedLengthChar>* bpTree = new Zzh::BPlusTree<Zzh::FixedLengthChar>(fileName);
					IndexManager::instance().stringIndex[val.indexName] = bpTree;
				}
			}
		}
		in.close();
		tableMap.insert(make_pair(tableName, table));
	}
	return tableMap.at(tableName);
}

Table& CatalogManager::createTable(string tableName, vector<Value>& valVector)
{
	tableMap.insert(make_pair(tableName, Table(tableName, valVector)));
	return tableMap.at(tableName);
}

bool CatalogManager::hasTable(string tableName)
{
	if (tableMap.count(tableName))
		return true;

	bool f = true;
	ifstream in(tableName + ".cal");
	if (!in)f = false;
	in.close();
	return f;
}

void CatalogManager::deleteTable(string tableName)
{
	auto iter = tableMap.find(tableName);
	if (iter != tableMap.end())
		tableMap.erase(iter);

	remove((tableName + ".cal").c_str());
}

bool CatalogManager::hasIndex(string IndexName)
{
	return static_cast<bool>(indexMap.count(IndexName));
}

pair<string, string> CatalogManager::mapIndex(string indexName)
{
	return indexMap[indexName];
}

void CatalogManager::createIndex(string indexName, string tableName, string valName)
{
	Table &table = getTable(tableName);

	for(auto& c:table.attriList)
	{
		if(c.valName == valName)
		{
			c.index = true;
			c.indexName = indexName;
			indexMap.insert(make_pair(indexName, make_pair(tableName, valName)));
			break;
		}
	}
}

void CatalogManager::deleteIndex(string indexName)
{
	auto iter = indexMap.find(indexName);

	string tableName = (*iter).second.first;
	string valName = (*iter).second.second;

	indexMap.erase(iter);

	Table& table = getTable(tableName);

	for(auto& c:table.attriList)
	{
		if(c.valName == valName)
		{
			c.index = false;
			c.indexName.clear();
			break;
		}
	}
}
