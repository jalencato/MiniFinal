#define _CRT_SECURE_NO_WARNINGS
#include"interpreter.h"

int main()
{
	interpreter form;

	form.Start();

	form.MemoryReset();

	system("pause");

	return 0;
}