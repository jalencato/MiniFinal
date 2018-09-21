#pragma once

#define FILEREAD 2
#define QUIT 3

#include <string>
#include <iostream>
#include "API.h"
#include <io.h>
#include <fstream>

using std::string;
using std::ifstream;
using std::getline;
using std::cout;
using std::cin;
using std::endl;

class interpreter
{
public:
	interpreter();
	~interpreter();
	void MemoryReset();

	void Start();
	int interprete(string line);
	string getOp(string str, int& tmp);

	void getFile(vector<string>& name);

	void reset();

	bool intJudge(string str);
	bool floatJudge(string str);
private:
	string FileName;
	API* interface;
	int pos = 0;

	void _create(string line);
	void _select(string line);
	void _drop(string line);
	void _delete(string line);
	void _insert(string line);
	void _execfile(string line);
};

struct FileOpenExep
{
public:
	string word;
	FileOpenExep(string word):word(word){}
	void print()
	{
		cout << "Fail to open the file" << endl;
	}
};

struct SyntaxExep
{
	string word;
	SyntaxExep(string word) :word(word) {}
	void print()
	{
		cout << "Syntax Error DB2018: " << (word.empty() ? "Empty Command or Name" : word) << endl;
	}
};