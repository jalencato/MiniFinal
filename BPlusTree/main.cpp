#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "BPlusTree.h"

using namespace std;

void SmallToBig() {
	BPlusTree<int> bpTree("data.txt", sizeof(int));
	for (int i = 1; i <= 10; i++) {
		bpTree.Insert(i, i);
		bpTree.Print();
	}
	for (int i = 1; i <= 10; i++) {
		int j = bpTree.FindOffset(i);
		if (i != j) {
			cout << "wtf" << endl;
			while (true) {}
		}
	}
	BufferManager::GetInstance().FreeAll();
	bpTree.Print();
	for (int i = 1; i <= 10; i++) {
		int j = bpTree.FindOffset(i);
		if (i != j) {
			cout << "wtf" << endl;
			while (true) {}
		}
	}
}

void BigToSmall() {
	BPlusTree<int> bpTree("data.txt", sizeof(int));
	for (int i = 10; i > 0; i--) {
		bpTree.Insert(i, i);
		bpTree.Print();
	}
	for (int i = 1; i <= 10; i++) {
		int j = bpTree.FindOffset(i);
		if (i != j) {
			cout << "wtf" << endl;
			while (true) {}
		}
	}
	BufferManager::GetInstance().FreeAll();
	bpTree.Print();
	for (int i = 1; i <= 10; i++) {
		int j = bpTree.FindOffset(i);
		if (i != j) {
			cout << "wtf" << endl;
			while (true) {}
		}
	}
}

void Random() {
	BPlusTree<int> bpTree("data.txt", sizeof(int));
	bpTree.Insert(9, 9);
	bpTree.Print();
	bpTree.Insert(4, 4);
	bpTree.Print();
	bpTree.Insert(7, 7);
	bpTree.Print();
	bpTree.Insert(1, 1);
	bpTree.Print();
	bpTree.Insert(6, 6);
	bpTree.Print();
	bpTree.Insert(8, 8);
	bpTree.Print();
	bpTree.Insert(3, 3);
	bpTree.Print();
	bpTree.Insert(2, 2);
	bpTree.Print();
	bpTree.Insert(5, 5);
	bpTree.Print();
	bpTree.Insert(10, 10);
	bpTree.Print();
	for (int i = 1; i <= 10; i++) {
		int j = bpTree.FindOffset(i);
		if (i != j) {
			cout << "wtf" << endl;
			while (true) {}
		}
	}
	BufferManager::GetInstance().FreeAll();
	bpTree.Print();
	for (int i = 1; i <= 10; i++) {
		int j = bpTree.FindOffset(i);
		if (i != j) {
			cout << "wtf" << endl;
			while (true) {}
		}
	}
}

void BulkLoad() {
	vector<int> keys = { 1,2,3,4,5, 6,7,8,9,10 };
	vector<int> offsets = { 1,2,3,4,5, 6,7,8,9,10 };
	BPlusTree<int> bpTree("data.txt", sizeof(int), keys, offsets);
	bpTree.Print();
	BufferManager::GetInstance().FreeAll();
	bpTree.Print();
	for (int i = 1; i <= 10; i++) {
		int j = bpTree.FindOffset(i);
		if (i != j) {
			cout << "wtf" << endl;
			while (true) {}
		}
	}
}

void TestFloat() {
	BPlusTree<float> bpTree("data.txt", sizeof(float));
	for (int i = 10; i > 0; i--) {
		bpTree.Insert((float)i + 0.1f, i);
		bpTree.Print();
	}
	for (int i = 1; i <= 10; i++) {
		int j = bpTree.FindOffset((float)i + 0.1f);
		if (i != j) {
			cout << "wtf" << endl;
			while (true) {}
		}
	}
	BufferManager::GetInstance().FreeAll();
	bpTree.Print();
	for (int i = 1; i <= 10; i++) {
		int j = bpTree.FindOffset((float)i + 0.1f);
		if (i != j) {
			cout << "wtf" << endl;
			while (true) {}
		}
	}
}

void TestStr() {
	BPlusTree<FixedLengthChar> bpTree("data.txt", 10 + 1);
	bpTree.Insert("a", 1);
	bpTree.Print();
	bpTree.Insert("b", 2);
	bpTree.Print();
	bpTree.Insert("c", 3);
	bpTree.Print();
	bpTree.FindOffset("a");
	bpTree.FindOffset("b");
	bpTree.FindOffset("c");
	bpTree.Insert("d", 4);
	bpTree.Print();
	bpTree.Delete("c");
	bpTree.Print();
	BufferManager::GetInstance().FreeAll();
	bpTree.Print();
	bpTree.FindOffset("a");
	bpTree.FindOffset("b");
	bpTree.FindOffset("c");
	bpTree.FindOffset("d");
	bpTree.SaveSchemaToFile();
	BufferManager::GetInstance().FreeAll();
}

int main() {
	TestStr();
	BPlusTree<FixedLengthChar> bpTree("data.txt");
	bpTree.Print();
	return 0;
}