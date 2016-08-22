#ifndef _INDEX_HASH_H
#define _INDEX_HASH_H

#include <string>
#include <fstream>
#include <vector>
#include "facilities.h"
#include "DataManager.h"

using namespace std;
using namespace typedefs;

#define MAXTABLESIZE 2000000					// default length for the hash table

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
	IndexHash(const string & _file_name, unsigned long _table_size = MAXTABLESIZE);
	~IndexHash();

	int db_store(const string& key, const string& value, int flag = 0);

	string db_fetch(const string& key);
	int db_delete(const string& key);

private:
	bool db_exist(const string& key);


private:
	// hash the given string, if succeeded, return true and modify the hash value
    bool create_hash(const string & str, hashval& hash_value);

	// search for the hashed value of the given str, 
	// if succeeded, return true and modify the hash value
	bool get_hash(const string & str, hashval& hash_value);


private:
	unsigned long table_size;				// stores the size of the hash table
											// also means the max length for the database

	unsigned long cryptTable[0x500];		// crypt table used for generating hash values

	vector<HashTableUnit> m_hash_table;

	// the dwHashType values used for three hash calculations
	const unsigned long HASH_OFFSET = 0, HASH_A = 1, HASH_B = 2;

private:
	string file_name;							// database

	string m_index_path;						// database.idx
	string m_index_meta_path;					// database.idx.meta

	// used by database.idx.meta:
	multimap<keylen, filepos> erased;
	bool save_meta();
	bool read_meta();

	string m_data_path;							// database.dat

private:
	DataManager data_man;						// class for managing data

	filepos AppendIndex(const string & key, const string & value, filepos next_index_pos = 0);
	bool OverwriteIndex(filepos index_pos, const string& key, const string& value, filepos next_index_pos = 0);
	bool SetNextIndexPos(filepos index_pos, filepos next_index_pos);
	filepos GetNextIndexPos(filepos index_pos);


	bool DeleteEntry(filepos index_pos, const string& deleted_key, filepos last_index_pos = 0);
	filepos GetDataPos(filepos index_pos, const string& target_key);

private:

	unsigned long single_bucket_size;			// stores the size(in bytes) of a single bucket
	filepos first_bucket_pos = 0;				// stores the position of the first bucket

	filepos ConvertToBucketPos(hashval hash_value);	    // calculate the position of a given bucket (hash_value)
	keylen get_length(const string & str) { return str.length() + 1; }

	bool WriteNewHeader();
	bool WriteHeader();
	bool ReadHeader();

	void message(string msg) {
		cout << "> " << msg << endl;
	}


private:
	void InitCryptTable();					// initialize the crypt table, called in constructor
	void InitHashTable();					// initialize the hash table

	hashval HashString(const string & lpszString, unsigned long dwHashType);
											// generate the hash value with the dwHashType value

private:
	char spliter = '\n';
};




#endif
