#define _CRT_SECURE_NO_WARNINGS
#include "API.h"
#include "CatalogManager.h"
#include "RecordManager.h"
#include "IndexManager.h"
#include <iomanip>


API::API() {
}

API::~API() {
}

void API::dropTable(string tableName) {
	if (!CatalogManager::instance().hasTable(tableName)) {
		cout << "No such table" << endl;
		return;
	}

	Table& table = CatalogManager::instance().getTable(tableName);
	for (auto& iter : table.attriList) {
		if (iter.index)dropIndex(iter.indexName);
	}

	RecordManager::instance().deleteTableName(tableName);
	CatalogManager::instance().deleteTable(tableName);
	cout << "Drop table " << tableName << " successfully" << endl;
}

void API::createTable(string tableName, vector<Value>* valVector) {
	if (CatalogManager::instance().hasTable(tableName)) {
		cerr << "There is a table " << tableName << " already" << endl;
		return;
	}

	RecordManager::instance().createTableName(tableName);

	CatalogManager::instance().createTable(tableName, *valVector);

	cout << "Create table " << tableName << " successfully" << endl;

	for (auto& iter : *valVector) {
		if (iter.primary) {
			createIndex(iter.indexName, tableName, iter.valName);
			break;
		}
	}
}

void API::dropIndex(string indexName) {
	if (!CatalogManager::instance().hasIndex(indexName)) {
		cout << "No such index" << endl;
		return;
	}
	auto iter = CatalogManager::instance().mapIndex(indexName);
	Value& val = getValue(iter.first, iter.second);

	IndexManager::instance().deleteIndex(indexName, val.type);
	CatalogManager::instance().deleteIndex(indexName);
	cout << "Delete Index " << indexName << " Successfully" << endl;
}

void API::createIndex(string indexName, string tableName, string valName) {
	bool hasVal = false;
	int type = 0;
	int size = 0;

	if (CatalogManager::instance().hasIndex(indexName)) {
		cout << "There is a index" << indexName << endl;
		return;
	}

	if (!CatalogManager::instance().hasTable(tableName)) {
		cout << "No such table" << endl;
		return;
	}

	vector<Value> valVector = CatalogManager::instance().getTable(tableName).attriList;

	for (auto& iter : valVector) {
		if (valName == iter.valName) {
			if (!iter.unique) {
				cout << "Not unique" << endl;
				return;
			}
			hasVal = true;
			type = iter.type;
			size = iter.size;
		}
	}
	if (!hasVal) {
		cout << "There is no such value" << endl;
		return;
	}

	IndexManager::instance().createIndex(indexName, type, size);
	CatalogManager::instance().createIndex(indexName, tableName, valName);
	cout << "Create Index " << indexName << " Successfully" << endl;
}

void API::InsertRecord(string tableName, vector<string>* valVector) {
	if (!CatalogManager::instance().hasTable(tableName)) {
		cerr << "No such table" << endl;
		return;
	}

	Table &table = CatalogManager::instance().getTable(tableName);
	vector<Value> valVec = table.attriList;
	Record rec;

	if (valVec.size() != valVector->size()) {
		cout << "Value count does not equal" << endl;
		return;
	}

	int cnt = 0;
	for (auto iter : valVec) {
		if (iter.type == INT) {
			if (isInt((*valVector)[cnt])) {
				Num num(atoi((*valVector)[cnt].c_str()));
				if (iter.unique && !isUnique(tableName, iter, num)) {
					cout << "Duplicate values" << endl;
					return;
				}
				rec.push_back(num);
			} else {
				cout << "Insert value type error" << endl;
				return;
			}
		} else if (iter.type == DOUBLE) {
			if (isFloat((*valVector)[cnt])) {
				Num num(static_cast<float>(atof((*valVector)[cnt].c_str())));
				if (iter.unique && !isUnique(tableName, iter, num)) {
					cout << "Duplicate values" << endl;
					return;
				}
				rec.push_back(num);
			} else {
				cout << "Insert value type error" << endl;
				return;
			}
		} else if (iter.type == STRING) {
			if ((*valVector)[cnt].size() <= iter.size + 2) {
				Num num((*valVector)[cnt], iter.size);
				if (iter.unique && !isUnique(tableName, iter, num)) {
					cout << "Duplicate values" << endl;
					return;
				}
				rec.push_back(num);
			} else {
				cout << "Insert value type error" << endl;
				return;
			}
		} else {}
		cnt++;
	}
	RecordManager::instance().insertRecord(tableName, rec);
	//cout << "Insert Successfully" << endl;
}

void API::deleteRecord(string tableName, vector<Judge>* judgeVec) {
	if (!CatalogManager::instance().hasTable(tableName)) {
		cout << "No such table" << endl;
		return;
	}

	Table& table = CatalogManager::instance().getTable(tableName);
	vector<Value> valVec = table.attriList;

	for (auto &iter : *judgeVec) {
		bool isVal = false;
		for (auto iterin : valVec) {
			if (iterin.valName == iter.valName) {
				isVal = true;
				break;
			}
		}
		if (!isVal) {
			cout << "No such value in condition" << endl;
			return;
		}
	}

	int num = RecordManager::instance().deleteRecord(tableName, *judgeVec);
	cout << "deleted records at " << num << endl;
}

void API::selectRecord(string tableName, vector<string>* valSelect, vector<Judge>* vecJudge) {
	Res res;
	if (!CatalogManager::instance().hasTable(tableName)) {
		cerr << "No such table" << endl;
		return;
	}

	Table& table = CatalogManager::instance().getTable(tableName);
	vector<Value> valVector = table.attriList;
	vector<string> passVec;

	vector<int> passVecNum;
	if (valSelect == nullptr) {
		for (auto& iter : valVector) {
			passVec.push_back(iter.valName);
		}
		valSelect = &passVec;
	} else {
		int cnt = 0;
		for (auto& iter : *valSelect)
			for (auto& iterIn : valVector) {
				if (iterIn.valName == iter)
					passVecNum.push_back(cnt);
				cnt++;
			}
	}

	for (auto& iterOut : *valSelect) {
		bool hasVal = false;
		for (auto& iterIn : valVector) {
			if (iterIn.valName == iterOut) {
				hasVal = true;
				break;
			}
		}
		if (!hasVal) {
			cout << "No such attribute in the project" << endl;
			return;
		}
	}

	if (vecJudge == nullptr)
		res = RecordManager::instance().selectAllRecord(tableName);
	else {
		for (auto& iter : *vecJudge) {
			bool hasVal = false;
			for (auto& iterIn : valVector) {
				if (iterIn.valName == iter.valName) {
					hasVal = true;
					break;
				}
			}
			if (!hasVal) {
				cerr << "No this judgement in the condition" << endl;
				return;
			}
		}
		res = RecordManager::instance().searchRecord(tableName, *vecJudge);
	}

	if (passVecNum.empty())
		print(res, valSelect, nullptr);
	else
		print(res, valSelect, &passVecNum);

	cout << res.size() << " are(is) chosen" << endl;
}

Value& API::getValue(string tableName, string valName) {
	if (!CatalogManager::instance().hasTable(tableName)) {
		Value* val = new Value;
		val->valName = "";
		return *val;
	}
	Table& table = CatalogManager::instance().getTable(tableName);
	for (auto iter : table.attriList) {
		if (iter.valName == valName) {
			Value* value = new Value(iter.valName, iter.type, iter.size, iter.unique, iter.index, iter.primary);
			if (value->index)value->indexName = iter.indexName;
			return *value;
		}
	}
	//on once
	auto val = new Value;
	val->valName = "";
	return *val;
}

void API::print(Res res, vector<string>* valNameVector, vector<int>* valSelectNo) {
	string str;

	int length[10086] = { 0 };

	if (res.empty())return;

	for (auto i = 0; i < (*valNameVector).size(); i++) {
		length[i] = (int)(*valNameVector)[i].size();
	}

	int k = 0;
	for (auto& iterOut : res) {
		int l1 = 0;
		int cnt = 0;

		for (auto &iterIn : iterOut) {
			vector<int>::iterator f;
			bool c = false;
			if (valSelectNo != nullptr) {
				f = std::find((*valSelectNo).begin(), (*valSelectNo).end(), cnt);
				if (f != (*valSelectNo).end())c = true;
			}

			if (c || valSelectNo == nullptr) {
				if (iterIn.type == INT) {
					int tmp = static_cast<int>(to_string(iterIn.data._int).size());
					length[l1] = length[l1] > tmp ? length[l1] : tmp;
				} else if (iterIn.type == DOUBLE) {
					length[l1] = length[l1] > 5 ? length[k] : 5;
				} else if (iterIn.type == STRING) {
					int tmp = (int)iterIn.str.size();
					length[l1] = length[l1] > tmp ? length[l1] : tmp;
				} else {}
				l1++;
			}
			cnt++;
		}
	}

	for (auto i = 0; i < (*valNameVector).size(); i++) {
		str += "+-";
		for (int j = 0; j < length[i]; j++)str += '-';
		str += "-";
	}
	str += "+";
	str += "\n";

	for (auto i = 0; i < (*valNameVector).size(); i++) {
		str += "| ";
		str += (*valNameVector)[i];
		for (int j = 0; j < length[i] - (*valNameVector)[i].size(); j++)
			str += " ";
		str += " ";
	}

	str += "|";
	str += "\n";
	for (auto i = 0; i < (*valNameVector).size(); i++) {
		str += "+-";
		for (int j = 0; j < length[i]; j++)str += '-';
		str += "-";
	}
	str += "+";
	str += "\n";

	int cnt = 0;
	bool find = false;
	for (auto &iterOut : res) {
		k = 0;
		cnt = 0;
		for (auto &iterIn : iterOut) {
			find = false;
			if (valSelectNo != nullptr) {
				for (int i = 0; i < (*valSelectNo).size(); i++) {
					if (cnt == (*valSelectNo)[i]) {
						find = true;
						break;
					}
				}
			}

			if (valSelectNo == nullptr || find) {
				str += "| ";
				if (iterIn.type == INT) {
					str += to_string(iterIn.data._int);
					for (int j = 0; j < length[k] - (int)to_string(iterIn.data._int).size(); j++)
						str += " ";
				} else if (iterIn.type == DOUBLE) {
					char temp[256];
					sprintf(temp, "%.4f", iterIn.data._float);
					str += temp;
				} else if (iterIn.type == STRING) {
					str += iterIn.str;
					for (auto j = 0; j < length[k] - iterIn.str.size(); j++)
						str += " ";
				} else {}
				str += " ";
				k++;
			}
			cnt++;
		}
		str += "|";
		str += "\n";
	}
	for (auto i = 0; i < (*valNameVector).size(); i++) {
		str += "+-";
		for (auto j = 0; j < length[i]; j++)str += '-';
		str += "-";
	}
	str += "+";
	str += "\n";
	cout << str;
	return;
}

bool API::isInt(string str) {
	stringstream sin(str);
	int num;
	return !!(sin >> noskipws >> num);
}

bool API::isFloat(string str) {
	stringstream sin(str);
	double num;
	return !!(sin >> noskipws >> num);
}

bool API::isUnique(string tableName, Value val, Num num) {
	Judge j(val.valName, num, Judge::EQUAL);
	vector<Judge> judVec;
	judVec.push_back(j);
	Res res = RecordManager::instance().searchRecord(tableName, judVec);
	if (res.size() > 0) return false;
	return true;
}
