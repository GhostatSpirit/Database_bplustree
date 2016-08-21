#ifndef _INDEX_HASH_H
#define _INDEX_HASH_H

#include <string>
#include <fstream>
#include <vector>

using namespace std;

#define MAXTABLESIZE 2000000					// default length for the hash table

typedef unsigned long filepos;
typedef unsigned long hashval;

struct HashTableUnit {

	hashval hash_A;
	hashval hash_B;
	filepos index_pos;
	bool used;

	HashTableUnit() {
		hash_A = 0;
		hash_B = 0;
		index_pos = 0;
		used = false;
	}
};

class IndexHash {
public:
	IndexHash(const string & _file_path, unsigned long _table_size = MAXTABLESIZE);


private:
	// hash the given string, if succeeded, return true and modify the hash value

  //  bool hash (const string & str, unsigned long& hash_value);

	// search for the hashed value of the given str, 
	// if succeeded, return true and modify the hash value

//	bool search(const string & str, unsigned long& hash_value);


private:
	unsigned long table_size;				// stores the size of the hash table
											// also means the max length for the database

	unsigned long cryptTable[0x500];		// crypt table used for generating hash values

	vector<HashTableUnit> m_hash_table;

private:
	string m_file_path;						// the path to the index hash file
											// default: database.hash

	unsigned long single_bucket_size;			// stores the size(in bytes) of a single bucket
	filepos first_bucket_pos = 0;				// stores the position of the first bucket

	filepos GetFilepos(hashval hash_value);	// calculate the position of a given bucket (hash_value)

private:
	bool WriteHeader(ofstream & ofs);
	bool ReadHeader(ifstream & ifs);


private:
	void InitCryptTable();					// initialize the crypt table, called in constructor
	void InitHashTable();					// initialize the hash table

	hashval HashString(const string & lpszString, unsigned long dwHashType);
											// generate the hash value with the dwHashType value

private:
	char spliter = '\n';
};




#endif
