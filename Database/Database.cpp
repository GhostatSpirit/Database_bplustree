// Database.cpp : Defines the entry point for the console application.
//

#include <iostream>

#include "bplustree.h"
#include "IndexHash.h"

using namespace std;

static const char alphanum[] =
"0123456789"
"!@#$%^&*"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz";

int stringLength = sizeof(alphanum) - 1;

char genRandom()
{
	return alphanum[rand() % stringLength];
}

string genRandomStr(unsigned length = 20) {
	string Str;
	for (unsigned int i = 0; i < length; ++i)
	{
		Str += genRandom();

	}
	return Str;
}

//
//int main()
//{
//	BPlusTree tree;
//	for (unsigned long i = 0; i < 10000; ++i) {
//		tree.insert(i, genRandomStr());
//
//	}
//
//	unsigned long target = 4;
//
//	tree.print();
//
//	VALUE* p_value = tree.find(target);
//
//	cout << *p_value << endl;
//	
//
//	system("pause");
//}

int main() {
	IndexHash("database.idx");
}