#define _CRT_SECURE_NO_WARNINGS
#include "RecordManager.h"
#include "CatalogManager.h"
#include "IndexManager.h"

RecordManager::RecordManager() {
}

RecordManager::~RecordManager() {
}

bool RecordManager::check(Num& num, Judge& cond) {
	bool res = true;
	if (cond.operation == Judge::EQUAL)//==
	{
		res = cond.value == num;
	} else if (cond.operation == Judge::NOTEQUAL)//!=
	{
		res = !(cond.value == num);
	} else if (cond.operation == Judge::LESS)//<
	{
		res = num < cond.value;
	} else if (cond.operation == Judge::GREATER)//>
	{
		res = !(cond.value == num) && !(num < cond.value);
	} else if (cond.operation == Judge::LESSOREQUAL)//<=
	{
		res = (cond.value == num) || (num < cond.value);
	} else if (cond.operation == Judge::GREATEROREQUAL)//>=
	{
		res = !(num < cond.value);
	} else {}
	return res;
}

void RecordManager::createTableName(string tableName) {
	FILE* fp = fopen((tableName + ".rec").c_str(), "wb");
	if (fp == NULL)
		cerr << "Error: create file " + tableName << " fail when we are making record file" << endl;
	fclose(fp);

	Block* block = BufferManager::instance().readBlock(tableName + ".rec", 0);
	BufferStream bufstr(block);

	char tt = 'E';//value-initialization value allowed
	bufstr << tt;

	BufferManager::instance().setDirty(block);
}

void RecordManager::deleteTableName(string tableName) {
	BufferManager::instance().deleteFile(tableName + ".rec");
}

void RecordManager::insertRecord(string tableName, Record& record) {
	Table& table = CatalogManager::instance().getTable(tableName);

	unsigned int insertOffset = table.offset;
	unsigned int indexOffset = 0;
	unsigned int freshOffset;
	unsigned int number = insertOffset / 4096;

	char c;
	int fresh = (insertOffset / 4096) * 4096;

	Block* block = BufferManager::instance().readBlock(tableName + ".rec", fresh);
	BufferStream bufstr(block);
	bufstr.setIter(insertOffset - fresh);

	bufstr.front(c);

	if (c == 'Y') {
		char tt = 'N';
		bufstr << tt;
		bufstr >> freshOffset;

		indexOffset = (unsigned int)(fresh + bufstr.getIter());

		for (Num& n : record) {
			if (n.type == INT) {
				bufstr << n.data._int;
			} else if (n.type == DOUBLE) {
				bufstr << n.data._float;
			} else if (n.type == STRING) {
				bufstr << n.str;
				bufstr.addIter(n.data.size - (int)n.str.size() - 1);
			} else {}
		}
	}

	if (c == 'E')//writing availble end of table value
	{
		if (bufstr.available() < table._size + sizeof(char) * 2 + sizeof(unsigned int))//N E offset
																					   //need another block to insert
		{
			number++;
			block = BufferManager::instance().readBlock(tableName + ".rec", number * 4096);
			char tt = 'N';
			bufstr << tt;//set an end to the block
			bufstr = BufferStream(block);
		}

		char tt = 'N';//if availble to 
		bufstr << tt;

		freshOffset = (unsigned int)(number * 4096 + bufstr.getIter() + sizeof(unsigned int) + table._size);
		bufstr << freshOffset;

		indexOffset = (unsigned int)(number * 4096 + bufstr.getIter());
		for (Num& n : record) {
			if (n.type == INT) {
				bufstr << n.data._int;
			} else if (n.type == DOUBLE) {
				bufstr << n.data._float;
			} else if (n.type == STRING) {
				bufstr << n.str;
				bufstr.addIter(n.data.size - (int)n.str.size() - 1);
			} else {}
		}

		char ttt = 'E';
		bufstr << ttt;
	}

	table.offset = freshOffset;
	BufferManager::instance().setDirty(block);

	int i = 0;
	for (auto& val : table.attriList) {
		if (val.index) {
			if (val.type == INT) {
				IndexManager::instance().insertIndex(val.indexName, record[i].data._int, indexOffset, val.type);
			} else if (val.type == DOUBLE) {
				IndexManager::instance().insertIndex(val.indexName, record[i].data._float, indexOffset, val.type);
			} else if (val.type == STRING) {
				IndexManager::instance().insertIndex(val.indexName, record[i].str, indexOffset, val.type);
			} else {}
		}
		++i;
	}
}

int RecordManager::deleteRecord(const string tableName, vector<Judge>& conditions) {
	Table& table = CatalogManager::instance().getTable(tableName);

	int number = 0;
	int cnt = 0;
	Block* block = BufferManager::instance().readBlock(tableName + ".rec", 0);
	BufferStream bufstr(block);

	Record rec;
	char c;

	while (true) {
		bufstr >> c;

		if (c == 'Y') {
			bufstr.addIter(sizeof(unsigned int) + table._size);
			continue;
		} else if (c == 'E') { break; }

		bufstr.addIter(sizeof(unsigned int));
		if (static_cast<int>(bufstr.available()) <= table._size) {
			number++;
			block = BufferManager::instance().readBlock(tableName + ".rec", number * 4096);
			bufstr = BufferStream(block);
			continue;
		}

		for (Value& val : table.attriList) {
			Num tmp;
			if (val.type == INT) {
				tmp.type = INT;
				bufstr >> tmp.data._int;
			}

			else if (val.type == DOUBLE) {
				tmp.type = DOUBLE;
				bufstr >> tmp.data._float;
			} else if (val.type == STRING) {
				tmp.type = STRING;
				bufstr >> tmp.str;
				tmp.data.size = val.size;
				bufstr.addIter(val.size - (int)tmp.str.size() - 1);
			}

			else {}
			rec.push_back(tmp);
		}

		//table rec conditions
		bool condition = true;
		for (auto& iterOut : conditions) {
			int k = 0;
			for (auto& iterIn : table.attriList) {
				if (iterIn.valName == iterOut.valName) {
					condition = check(rec[k], iterOut);
					break;
				}
				k++;
			}
			if (!condition)
				break;
		}

		if (condition) {
			bufstr.addIter(-static_cast<int>(table._size + sizeof(char) + sizeof(unsigned int)));
			char tt = 'Y';
			bufstr << tt;
			bufstr << table.offset;//move one offset

			table.offset = static_cast<unsigned int>(number * 4096 + bufstr.getIter() - sizeof(char) - sizeof(unsigned int));

			bufstr.addIter(table._size);
			cnt++;

			int l = 0;
			for (auto& val : table.attriList) {
				if (val.index) {
					if (val.type == INT) {
						IndexManager::instance().deleteIndex(val.indexName, rec[l].data._int, val.type);
					}
					if (val.type == DOUBLE) {
						IndexManager::instance().deleteIndex(val.indexName, rec[l].data._float, val.type);
					} else if (val.type == STRING) {
						IndexManager::instance().deleteIndex(val.indexName, rec[l].str, val.type);
					} else {}
				}
				l++;
			}
		}
		rec.clear();
	}
	return cnt;
}

Res RecordManager::searchRecord(string tableName, vector<Judge>& judgement) {
	Table& table = CatalogManager::instance().getTable(tableName);

	Block* block;
	int number = 0;

	
	for (int i = 0; i < judgement.size(); i++) {
		if (judgement[i].operation != Judge::EQUAL) continue;
		Value valX;
		for (Value& val : table.attriList) {
			if (val.valName != judgement[i].valName) continue;
			valX = val;
			break;
		}
		if (!valX.index) continue;
		//到这里说明找到了等值查找，并且该属性有索引
		{
			SS support(valX.indexName, judgement[i].value);
			int offset = IndexManager::instance().searchIndex(support);
			if (offset >= 0) {
				number = offset / 4096;
				block = BufferManager::instance().readBlock(tableName + ".rec", number * 4096);
				BufferStream bufstr(block);
				offset = offset - number * 4096;
				bufstr.addIter(offset);
				Record record;
				for (Value& val : table.attriList) {
					Num tmp;
					if (val.type == INT) {
						tmp.type = INT;
						bufstr >> tmp.data._int;
					}
					else if (val.type == DOUBLE) {
						tmp.type = DOUBLE;
						bufstr >> tmp.data._float;
					} else if (val.type == STRING) {
						tmp.type = STRING;
						bufstr >> tmp.str;
						tmp.data.size = val.size;
						bufstr.addIter(val.size - (int)tmp.str.size() - 1);
					} else {}
					record.push_back(tmp);
				}

				bool condition = true;
				for (auto& iterOut : judgement) {
					int k = 0;
					for (auto& iterIn : table.attriList) {
						if (iterIn.valName == iterOut.valName) {
							condition = check(record[k], iterOut);
							break;
						}
						k++;
					}
					if (!condition)
						break;
				}

				Res res;
				if (condition) {
					res.push_back(record);
				}
				return res;
			} else {
				Res res;
				return res;
			}
		}
	}

	{
		number = 0;
		block = BufferManager::instance().readBlock(tableName + ".rec", 0);
		BufferStream bufstr(block);

		Res res;

		while (true) {
			Record record;
			char c;
			bufstr >> c;
			if (c == 'Y') {
				bufstr.addIter(sizeof(unsigned int) + table._size);
				continue;
			} else if (c == 'E') {
				break;
			}

			bufstr.addIter(sizeof(unsigned int));

			if (static_cast<int>(bufstr.available()) <= table._size) {
				number++;
				block = BufferManager::instance().readBlock(tableName + ".rec", number * 4096);
				bufstr = BufferStream(block);
				continue;
			}

			for (Value& val : table.attriList) {
				Num tmp;
				if (val.type == INT) {
					tmp.type = INT;
					bufstr >> tmp.data._int;
				}

				else if (val.type == DOUBLE) {
					tmp.type = DOUBLE;
					bufstr >> tmp.data._float;
				} else if (val.type == STRING) {
					tmp.type = STRING;
					bufstr >> tmp.str;
					tmp.data.size = val.size;
					bufstr.addIter(val.size - (int)tmp.str.size() - 1);
				} else {}
				record.push_back(tmp);
			}

			bool condition = true;
			for (auto& iterOut : judgement) {
				int k = 0;
				for (auto& iterIn : table.attriList) {
					if (iterIn.valName == iterOut.valName) {
						condition = check(record[k], iterOut);
						break;
					}
					k++;
				}
				if (!condition)
					break;
			}

			if (condition)
				res.push_back(record);
			record.clear();
		}
		return res;
	}
}

Res RecordManager::searchIndex(string tableName, vector<Judge>& judgement) {
	Table& table = CatalogManager::instance().getTable(tableName);
	auto& indexFinder = IndexManager::instance();

	Res res;

	while (true) {
		for (auto iterOut : judgement) {
			auto valName = iterOut.valName;

		}
	}

	return res;
}

Res RecordManager::selectAllRecord(string tableName) {
	Table& table = CatalogManager::instance().getTable(tableName);

	int number = 0;
	Block* block = BufferManager::instance().readBlock(tableName + ".rec", 0);
	BufferStream bufstr(block);

	Res res;
	Record record;
	char c;

	while (true) {
		bufstr >> c;
		if (c == 'Y') {
			bufstr.addIter(sizeof(unsigned int) + table._size);
			continue;
		} else if (c == 'E') {
			break;
		}

		bufstr.addIter(sizeof(unsigned int));

		if (static_cast<int>(bufstr.available()) <= table._size) {
			number++;
			block = BufferManager::instance().readBlock(tableName + ".rec", number * 4096);
			bufstr = BufferStream(block);
			continue;
		}

		for (Value& val : table.attriList) {
			Num tmp;
			if (val.type == INT) {
				tmp.type = INT;
				bufstr >> tmp.data._int;
			}

			else if (val.type == DOUBLE) {
				tmp.type = DOUBLE;
				bufstr >> tmp.data._float;
			} else if (val.type == STRING) {
				tmp.type = STRING;
				bufstr >> tmp.str;
				tmp.data.size = val.size;
				bufstr.addIter(val.size - (int)tmp.str.size() - 1);
			}

			else {}
			record.push_back(tmp);
		}

		res.push_back(record);
		record.clear();
	}
	return res;
}
