// Database.cpp : Defines the entry point for the console application.
//

#include <iostream>

#include "bplustree.h"

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


int main()
{
	BPlusTree tree;
	tree.insert(1, genRandomStr());
	tree.insert(2, genRandomStr());
	tree.insert(3, genRandomStr());
	tree.insert(4, genRandomStr());
	tree.insert(5, genRandomStr());

	system("pause");
}

