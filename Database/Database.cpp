// Database.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <cstdio>
#include <ctime>
#include <chrono>

//#include "bplustree.h"
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
	DB db = "testdb";

	std::time_t startALL, finishALL;
	double duration = 0;

	char buf[50];
	int nrec = 1000;
	vector<int> v;
	for (int i = 0; i<nrec; i++) { v.push_back(i); }

	startALL = clock();

	for (int i = 0; i<nrec; i++) {
		sprintf_s<50>(buf, "%d", i);
		db.db_insert(buf, buf);
		//cout<<i<<endl;
	}
	for (int i = 0; i<nrec; i++) {
		sprintf_s<50>(buf, "%d", i);
		cout << db.db_fetch(buf);
		//cout<<i<<endl;
	}

	cout << "test: part A completed" << endl;

	for (int i = 0; i<5 * nrec; i++) {
		int randnum = rand() % v.size();
		sprintf_s<50>(buf,"%d", v[randnum]);

		if (i % 37 == 0) { db.db_delete(buf); }
		else if (i % 11 == 0) {
			int temp = v[randnum] + nrec;
			sprintf_s<50>(buf, "%d", temp);
			db.db_insert(buf, buf);
			v.push_back(temp);
			cout << db.db_fetch(buf) << endl;
		}
		else if (i % 17 == 0) {
			sprintf_s<50>(buf, "%d", v[randnum]);
			db.db_replace(buf, "??");
		}
	}
	cout << "test: part B completed" << endl;

	for (int i : v) {
		int randnum = rand() % v.size();
		sprintf_s<50>(buf, "%d", v[randnum]);
		db.db_delete(buf);

		for (int i = 0; i<10; i++) {
			randnum = rand() % nrec;
			sprintf_s<50>(buf, "%d", randnum);
			cout << db.db_fetch(buf) << endl;

		}
	}

	cout << "ok";
	finishALL = clock();
	duration = (finishALL - startALL) / (double)CLOCKS_PER_SEC;
	cout << "test used time: " << duration << endl;

}


void test_fetch(unsigned long last_key, DB& database);

void insert_number(unsigned long count, DB& database) {

	unsigned long fail_count = 0;

	auto write_start = std::chrono::steady_clock::now();

	for (unsigned long i = 1; i <= count ; i++) {
		char buf[50];

		sprintf_s<50>(buf, "%lu", i);
		if (database.db_insert(buf, buf) != 0) {
			++fail_count;
		}

		if (i % 10000 == 0) {
			// print out the elapsed time
			auto write_end = std::chrono::steady_clock::now();
			auto write_elapsed = std::chrono::duration_cast<std::chrono::seconds>(write_end - write_start);

			cout << "written: " << i - 9999 << " - " << i << endl;
			cout << "used time: " << write_elapsed.count() << " (seconds)" << endl;
			cout << endl;
			
			string key_str(buf);
			test_fetch(i, database);

			write_start = std::chrono::steady_clock::now();

		}

	}

}

void test_fetch(unsigned long last_key, DB& database) {
	std::cout << "fetching: " << last_key - 100 << " - " << last_key << std::endl;

	unsigned long count = 0;

	auto start = std::chrono::steady_clock::now();

	for (unsigned long i = last_key - 99; i <= last_key; ++i) {
		string key_str = to_string(i);
		string val = database.db_fetch(key_str);
		if (val == "") {
			cout << "cannot fetch: " << key_str;
		}

		count++;
	}

	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start) / count;

	std::cout << "It took me " << elapsed.count() << " microseconds in average." << std::endl;
	cout << endl;


}


int main() {
	//DB database ("database") ;

	//database.db_insert("00", "Miku");
	//
	//database.db_delete("00");

	//database.db_insert("01", "Luka");

	//database.db_replace("01", "hello");

	//cout << database.db_fetch("01") << endl;

	DB database("testdb");

	insert_number(1000000, database);

	system("pause");
}