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


void test() {
	DB* a = db_open("/Users/JimLiu/Desktop/project db/testdb/a");
	char buf[10];
	int nrec = 1000;
	vector<int> v;
	for (int i = 0; i<nrec; i++) { v.push_back(i); }

	startALL=clock();

	for (int i = 0; i<nrec; i++) {
		sprintf(buf, "%d", i);
		db_insert(a, buf, buf);
		//cout<<i<<endl;
	}
	for (int i = 0; i<nrec; i++) {
		sprintf(buf, "%d", i);
		cout << db_fetch(a, buf);
		//cout<<i<<endl;
	}
	cout << "1";
	for (int i = 0; i<5 * nrec; i++) {
		int randnum = rand() % v.size();
		sprintf(buf, "%d", v[randnum]);

		if (i % 37 == 0) { db_remove(a, buf); }
		else if (i % 11 == 0) {
			int temp = v[randnum] + nrec;
			sprintf(buf, "%d", temp);
			db_insert(a, buf, buf);
			v.push_back(temp);
			cout << db_fetch(a, buf);
		}
		else if (i % 17 == 0) {
			sprintf(buf, "%d", v[randnum]);
			db_replace(a, buf, "??");
		}
	}
	cout << endl << "2" << endl;;
	for (int i : v) {
		int randnum = rand() % v.size();
		sprintf(buf, "%d", v[randnum]);
		db_remove(a, buf);
		for (int i = 0; i<10; i++) {
			randnum = rand() % nrec;
			sprintf(buf, "%d", randnum);
			cout << db_fetch(a, buf);

		}
	}
	cout << "ok";
	finishALL = clock();
	total = (finishALL - startALL);
	cout << total << endl;

}

int main() {
	IndexHash database ("database") ;

	database.db_store("00", "Miku");
	
	database.db_delete("00");

	database.db_store("01", "Luka");

	cout << database.db_fetch("01") << endl;

	system("pause");
}