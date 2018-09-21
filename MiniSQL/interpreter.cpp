#define _CRT_SECURE_NO_WARNINGS

#include "interpreter.h"
#include <fstream>
#include "CatalogManager.h"
#include "IndexManager.h"
#include <ctime>


interpreter::interpreter()
{
	cout << "Success to create a minisql" << endl;
	cout << "Proudly presented by Group 07" << endl;
}

interpreter::~interpreter()
{
	cout << "You are quitting the minisql" << endl;
	cout << "See you again" << endl;
}

void interpreter::MemoryReset()
{
	IndexManager::instance().freeMem();
	BufferManager::instance().flushBlock();
}

void interpreter::Start()
{
	//std::ifstream myIn("./test_perform.sql");

	string sql = "", thisline = "";
	bool is_file = false;

	int state = 0;
	while(true)
	{
		clock_t start, end;
		if (!is_file)
		{
			sql = thisline + " ";
			if (thisline == "")
			{
				cout << "Input the next sentences>";
			}
			else
			{
				cout << "Still some chars are waiting" << " ->";
			}

			while (getline(cin, thisline))
			{
				auto line_end = thisline.find_first_of(';');
				if (line_end == string::npos)//nofound
				{
					sql += " " + thisline;
					cout << "         ->";//read the following sentense
				}
				else
				{
					sql += " " + thisline.substr(0, line_end);
					thisline = thisline.substr(line_end + 1, thisline.size());
					start = clock();
					break;
				}
			}
			pos = 0;
			state = interprete(sql);

			if (state == FILEREAD)
				is_file = true;
			if (state == QUIT)
				break;
		}
		else
		{
			ifstream ifs;
			try
			{
				ifs.open(FileName.c_str());
				if (!ifs.is_open())throw FileOpenExep(FileName);
			}
			catch(FileOpenExep &f)
			{
				f.print();
				is_file = false;
				continue;
			}

			start = clock();
			while(getline(ifs,sql,';'))
			{
				pos = 0;
				interprete(sql);
			}
			ifs.close();
			is_file = false;
		}

		end = clock();
		double last = end - start;
		cout << "The duration time is " << (last / CLOCKS_PER_SEC) << " seconds" << endl;
		if (cin.eof()) break;
	}
}

int interpreter::interprete(string line)
{
	//create,select,drop,delete,insert,execfile,quit
	string op;
	try
	{
 		op = getOp(line, pos);

		if(op == "create")
		{
			_create(line);
		}
		else if(op == "select")
		{
			_select(line);
		}
		else if(op == "drop")
		{
			_drop(line);
		}
		else if(op == "delete")
		{
			_delete(line);
		}
		else if(op == "insert")
		{
			_insert(line);
		}
		else if (op == "execfile")
		{
			_execfile(line);
			FileName = getOp(line, pos);
			return FILEREAD;
		}
		else if(op == "quit")
		{
			return QUIT;
		}
		else throw SyntaxExep("Unavailble op name");
	}
	catch(SyntaxExep& e)
	{
		if (!op.empty()) e.print();
	}
	return 0;
}

string interpreter::getOp(string str, int& tmp)
{
	string op;

	while ((str[tmp] == ' ' || str[tmp] == '\n' || str[tmp] == '\t') && str[tmp] != 0)
		tmp++;

	int start = tmp;	int end = tmp + 1;
	if (str[tmp] == '(' || str[tmp] == ',' || str[tmp] == ')')
	{
		tmp++;
		op = str.substr(start, end - start);
	}
	//" word "
	else if(str[tmp] == '"')
	{
		tmp++;
		while (str[tmp] != '"'&&str[tmp] != 0)
		{
			tmp++;
		}
		if(str[tmp] == '"')
		{
			start++;
			end = tmp;//just load the word in the " "
			tmp++;
			op = str.substr(start, end - start);
		}
		else
		{
			op = "";
		}
	}
	else
	{
		while (str[tmp] != ' ' && str[tmp] != '\n' && str[tmp] != '\t' &&str[tmp] != 0
			&& str[tmp] != '('&&str[tmp] != ')'&&str[tmp] != ',')
		{
			tmp++;
		}
		end = tmp;
		if (start != end)
			op = str.substr(start, end - start);
		else op = "";
	}
	return op;
}

void interpreter::getFile(vector<string>& name)
{
	string postName = "//*.rec";
}

void interpreter::reset()
{
}

bool interpreter::intJudge(string str)
{
	stringstream sin(str);
	int num;
	return !!(sin >> noskipws >> num);
}

bool interpreter::floatJudge(string str)
{
	stringstream sin(str);
	double num;
	return !!(sin >> noskipws >> num);
}

void interpreter::_create(string line)
{
	string tableName;
	try
	{
		//get the name "table"
		string op = getOp(line, pos);
		if (op == "table")
		{
			//get table name
			op = getOp(line, pos);
			if (!op.empty())tableName = op;
			else throw SyntaxExep(op);

			//get (
			op = getOp(line, pos);
			if (op.empty() || op != "(")throw SyntaxExep(op);

			//get the value name or the primary or the end
			op = getOp(line, pos);
			vector<Value> valVector;
			string valName;
			int type = 0; int size = 0;
			while (!op.empty() && op != "primary"&&op != ")")
			{
				valName = op;
				op = getOp(line, pos);

				if (op == "int")
				{
					type = INT;
					size = sizeof(int);
				}
				else if (op == "float")
				{
					type = DOUBLE;
					size = sizeof(float);
				}
				else if (op == "char")
				{
					type = STRING;
					op = getOp(line, pos);
					if (op != "(")throw SyntaxExep("Wrong Type");
					op = getOp(line, pos);

					if (!intJudge(op))throw SyntaxExep("not int number");
					size = atoi(op.c_str());
					op = getOp(line, pos);
					if (op != ")")throw SyntaxExep("not the end");
				}
				else throw SyntaxExep("Unsupportive type");

				op = getOp(line, pos);
				bool unique = false;
				if (op == "unique")
				{
					unique = true;
					op = getOp(line, pos);
				}
				//or we may miss one element
				Value valElement(valName, type, size, unique, false, false);
				valVector.push_back(valElement);
				if (op != ",")
				{
					if (op != ")")throw SyntaxExep("emmmm");
					else break;
				}
				op = getOp(line, pos);
			}
			if (op == "primary")
			{
				op = getOp(line, pos);
				if (op != "key")throw SyntaxExep(op);
				op = getOp(line, pos);
				if (op == "(")
				{
					op = getOp(line, pos); size_t cnt = 0;
					for (auto& i : valVector)
					{
						if (op == i.valName)
						{
							i.primary = true;
							i.index = true;
							i.unique = true;
							i.indexName = i.valName;
							break;
						}
						cnt++;
					}
					if (cnt == valVector.size())throw SyntaxExep(op);
					op = getOp(line, pos);
					if (op != ")")throw SyntaxExep(op);
				}
				else throw SyntaxExep(op);
				op = getOp(line, pos);
				if (op != ")")throw SyntaxExep(op);
			}
			interface->createTable(tableName, &valVector);
			return;
		}
		else if (op == "index")
		{
			string valName = "";
			string indexName;
			op = getOp(line, pos);
			if (!op.empty())
				indexName = op;
			else throw SyntaxExep(op);

			op = getOp(line, pos);
			if (op != "on")throw SyntaxExep(op);

			op = getOp(line, pos);
			if (op.empty())throw SyntaxExep(op);

			tableName = op;

			op = getOp(line, pos);
			if (op != "(")throw SyntaxExep(op);

			op = getOp(line, pos);
			if (op.empty())throw SyntaxExep(op);
			valName = op;

			op = getOp(line, pos);
			if (op != ")")throw SyntaxExep(op);

			interface->createIndex(indexName, tableName, valName);
		}
		else throw SyntaxExep(op);
	}
	catch(SyntaxExep& synExp)
	{
		synExp.print();
		return;
	}
}

void interpreter::_select(string line)
{
	//select ... from tableName where ... operator ... and ... operator ...
	try
	{
		//selecting names or *
		string op = getOp(line, pos);
		vector<string> valSelect;
		if(op != "*")
		{
			while(op != "from")
			{
				valSelect.push_back(op);
				op = getOp(line, pos);
				if (op == ",") op = getOp(line, pos);
			}
		}
		else op = getOp(line, pos);

		//from
		if (op != "from")throw SyntaxExep(op);

		//tablename
		string tableName;
		op = getOp(line, pos);
		if (!op.empty())
			tableName = op;
		else throw SyntaxExep(op);

		//where or no where
		op = getOp(line, pos);
		if(op.empty())//no where
		{
			if (valSelect.empty())
				interface->selectRecord(tableName, nullptr, nullptr);
			else
				interface->selectRecord(tableName, &valSelect, nullptr);
			return;
		}
		else if(op == "where")//where
		{
			Num num;
			vector<Judge> vecJudge;//table value operator
			op = getOp(line, pos);//valuename

			Judge::OPERATION oper;
			string selectValName;
			while(true)
			{
				if (op.empty())
					throw SyntaxExep("no judgements");
				selectValName = op;

				op = getOp(line, pos);//operator
				if (op == "=")oper = Judge::EQUAL;
				else if (op == "<>" || op == "!=")oper = Judge::NOTEQUAL;
				else if (op == "<")oper = Judge::LESS;
				else if (op == ">")oper = Judge::GREATER;
				else if (op == "<=")oper = Judge::LESSOREQUAL;
				else if (op == ">=")oper = Judge::GREATEROREQUAL;
				else throw SyntaxExep(op);

				op = getOp(line, pos);
				if (op.empty())throw SyntaxExep(op);
				if (intJudge(op) && op.find(".") == string::npos)num = Num(atoi(op.c_str()));
				else if (floatJudge(op))num = Num((float)atof(op.c_str()));
				else //must be a string without exception
				{
					auto tmp = interface->getValue(tableName, selectValName);
					if (tmp.valName.empty())throw SyntaxExep("no value in the list");
					num = Num(op, tmp.size);
				}

				Judge judgement(selectValName, num, oper);
				vecJudge.push_back(judgement);
				op = getOp(line, pos);
				//the ending of the finding 
				if (op.empty())break;
				else if (op != "and")throw SyntaxExep(op);
				op = getOp(line, pos);
			}
			if (valSelect.empty())
				interface->selectRecord(tableName, nullptr, &vecJudge);
			else interface->selectRecord(tableName, &valSelect, &vecJudge);
		}
		else throw(SyntaxExep(op));
	}
	catch (SyntaxExep& e)
	{
		e.print();
	}
}

void interpreter::_drop(string line)
{
	string op;

	try
	{
		op = getOp(line, pos);
		if(op == "table")
		{
			op = getOp(line, pos);
			if(!op.empty())
			{
				interface->dropTable(op);
				return;
			}
			else throw SyntaxExep(op);
		}
		else if(op == "index")
		{
			op = getOp(line, pos);
			if (!op.empty())
			{
				interface->dropIndex(op);
				return;
			}
			else throw SyntaxExep(op);
		}
		else throw SyntaxExep(op);
	}
	catch (SyntaxExep& e)
	{
		e.print();
	}
}

void interpreter::_delete(string line)
{
	//delete 
	try
	{
		string op = getOp(line, pos);
		if (op != "from")throw SyntaxExep(op);

		op = getOp(line, pos);
		string tableName;
		if (op.empty())throw SyntaxExep(op);
		tableName = op;

		op = getOp(line, pos);
		if(op.empty())
		{
			vector<Judge> tmp;
			interface->deleteRecord(tableName, &tmp);
		}
		else if (op == "where")
		{
			Num num;
			Judge::OPERATION oper;
			op = getOp(line, pos);
			vector<Judge> judgement;
			string valName;
			while(true)
			{
				if (op.empty())throw SyntaxExep(op);
				valName = op;

				op = getOp(line, pos);
				if (op == "=") oper = Judge::EQUAL;
				else if (op == "<>" || op == "!=")oper = Judge::NOTEQUAL;
				else if (op == "<")oper = Judge::LESS;
				else if (op == ">")oper = Judge::GREATER;
				else if (op == "<=")oper = Judge::LESSOREQUAL;
				else if (op == ">=")oper = Judge::GREATEROREQUAL;
				else throw SyntaxExep(op);

				op = getOp(line, pos);
				if (op.empty())throw SyntaxExep(op);
				if (intJudge(op))num = Num(atoi(op.c_str()));
				else if (floatJudge(op))num = Num((float)atof(op.c_str()));
				else
				{
					auto tmp = interface->getValue(tableName, valName);
					if (tmp.valName.empty())throw SyntaxExep("no value in the list");
					num = Num(op, tmp.size);
				}

				Judge judge(valName, num, oper);
				judgement.push_back(judge);
				op = getOp(line, pos);
				if (op.empty())break;
				else if (op != "and")throw SyntaxExep(op);

				op = getOp(line, pos);
			}
			interface->deleteRecord(tableName, &judgement);
		}
	}
	catch (SyntaxExep &e)
	{
		e.print();
	}
}

void interpreter::_insert(string line)
{
	try
	{
		//insert into tableNAme value(...,...,...)
		string op = getOp(line, pos);
		if (op != "into"&&op != "INTO")throw SyntaxExep(op);

		op = getOp(line, pos);
		//table name
		if (op.empty())throw SyntaxExep(op);
		string tableName = op;
		//value
		op = getOp(line, pos);
		if (op != "values"&&op != "value"&&op != "VALUE"&&op != "VALUES")
			throw SyntaxExep(op);
		//(
		op = getOp(line, pos);
		if (op != "(")throw SyntaxExep(op);
		//load the word inside the " "
		op = getOp(line, pos);
		vector<string> strVector;
		while(!op.empty()&&op!=")")
		{
			strVector.push_back(op);
			op = getOp(line, pos);
			if (op == ",")
				op = getOp(line, pos);
		}
		if (op != ")")throw SyntaxExep(op);
		interface->InsertRecord(tableName, &strVector);
	}
	catch (SyntaxExep &s)
	{
		s.print();
	}
}

void interpreter::_execfile(string line)
{
	cout << "we will invoke a sql file" << endl;
}
