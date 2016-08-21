#include "IndexHash.h"

#include <fstream>
#include <iostream>
#include <vector>
#include "facilities.h"

IndexHash::IndexHash(const string & _file_path, unsigned long _table_size)
{
	// initialize crypt table
	InitCryptTable();
	// save the table length
	table_size = _table_size;
	// save the file path
	m_file_path = _file_path;

	first_bucket_pos = 0;

	// try to open the hash table file
	ifstream ifs(m_file_path, ios::binary);
	
	if (!ifs) {
		// if we do not have the hash table file...
		// initialize a new hash table and write it
		ifs.close();
		InitHashTable();
		ofstream ofs(m_file_path, ios::binary);
		WriteHeader(ofs);
		ofs.close();
	}
	else {
		// if we have the hash table file,
		// we need to read it to the memory
		ReadHeader(ifs);
		ifs.close();
	}

}

filepos IndexHash::GetFilepos(hashval hash_value)
{
	if (first_bucket_pos == 0) {
		throw runtime_error("GetFilepos: first_bucket_pos not defined");
		return 0;
	}

	return first_bucket_pos + hash_value * single_bucket_size;

}



void IndexHash::InitHashTable()
{
	for (unsigned long i = 0; i < table_size; ++i) {
		HashTableUnit unit;
		m_hash_table.push_back(unit);
	}
	// shrink_to_fit() to save memory
	m_hash_table.shrink_to_fit();

	single_bucket_size = sizeof(bool) + sizeof(hashval) + sizeof(hashval) + sizeof(filepos) + sizeof(char);
	cout << single_bucket_size << endl;
}



bool IndexHash::WriteHeader(ofstream& ofs) {
	if (!ofs) {
		return false;
	}
	// put the cursor to the start of the file
	ofs.seekp(0, ofs.beg);
	// ------- writing header --------
	// unsigned long hash_table_size
	ofs.write(as_bytes(table_size), sizeof(unsigned long));
	// unsigned long single_bucket_size
	single_bucket_size = sizeof(bool) + sizeof(hashval) + sizeof(hashval) + sizeof(filepos) + sizeof(char);
	ofs.write(as_bytes(single_bucket_size), sizeof(unsigned long));
	// write a fake first_bucket_pos first
	filepos back_pos = ofs.tellp();
	filepos temp_pos = 0;
	ofs.write(as_bytes(temp_pos), sizeof(filepos));
	// write the spliter
	ofs.write(as_bytes(spliter), sizeof(char));
	// now we can save cursor pos as first_bucket_pos
	first_bucket_pos = ofs.tellp();
	// write the first_bucket_pos again
	ofs.seekp(back_pos);
	// filepos first_bucket_pos (real one)
	ofs.write(as_bytes(first_bucket_pos), sizeof(filepos));
	// get back to first_bucket_pos
	ofs.seekp(first_bucket_pos);

	// -------- writing buckets --------
	// checking if size fits
	if (m_hash_table.size() != table_size) {
		throw runtime_error("WriteHeader: vector size not fit table size");
	}
	for (unsigned long i = 0; i < table_size; ++i) {
		// writing a single bucket
		HashTableUnit& this_unit = m_hash_table[i];
		ofs.write(as_bytes(this_unit.used), sizeof(bool));
		ofs.write(as_bytes(this_unit.hash_A), sizeof(hashval));
		ofs.write(as_bytes(this_unit.hash_B), sizeof(hashval));
		ofs.write(as_bytes(this_unit.index_pos), sizeof(filepos));
		ofs.write(as_bytes(spliter), sizeof(char));
	}
	ofs.close();

	return true;
}

bool IndexHash::ReadHeader(ifstream& ifs) {
	if (!ifs) {
		return false;
	}
	// put the cursor to the start of the file
	ifs.seekg(0, ifs.beg);

	// ------- reading header --------
	// unsigned long hash_table_size
	ifs.read(as_bytes(table_size), sizeof(unsigned long));
	// unsigned long single_bucket_size
	ifs.read(as_bytes(single_bucket_size), sizeof(unsigned long));
	// filepos first_bucket_pos
	ifs.read(as_bytes(first_bucket_pos), sizeof(filepos));
	// char spliter (of no use)
	char temp_char;
	ifs.read(as_bytes(temp_char), sizeof(char));

	// check if the first_bucket_pos fits
	filepos temp_pos = ifs.tellg();
	if (first_bucket_pos != temp_pos) {
		throw runtime_error("ReadHeader: first_bucket_pos not fit");
	}

	// ------- reading buckets --------
	for (unsigned long i = 0; i < table_size; ++i) {
		// reading and constructing a single bucket
		HashTableUnit new_unit;
		ifs.read(as_bytes(new_unit.used), sizeof(bool));
		ifs.read(as_bytes(new_unit.hash_A), sizeof(hashval));
		ifs.read(as_bytes(new_unit.hash_B), sizeof(hashval));
		ifs.read(as_bytes(new_unit.index_pos), sizeof(filepos));
		ifs.read(as_bytes(temp_char), sizeof(char));
		// add the new bucket to the vector m_hash_table
		m_hash_table.push_back(new_unit);
	}
	// shrink_to_fit to save memory
	m_hash_table.shrink_to_fit();

	return true;
}



void IndexHash::InitCryptTable()
{
	unsigned long seed = 0x00100001, index1 = 0, index2 = 0, i;

	for (index1 = 0; index1 < 0x100; index1++)
	{
		for (index2 = index1, i = 0; i < 5; i++, index2 += 0x100)
		{
			unsigned long temp1, temp2;
			seed = (seed * 125 + 3) % 0x2AAAAB;
			temp1 = (seed & 0xFFFF) << 0x10;
			seed = (seed * 125 + 3) % 0x2AAAAB;
			temp2 = (seed & 0xFFFF);
			cryptTable[index2] = (temp1 | temp2);
		}
	}
}

hashval IndexHash::HashString(const string & lpszString, unsigned long dwHashType)
{
	unsigned char *key = (unsigned char *)(const_cast<char*>(lpszString.c_str()));
	unsigned long seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;
	int ch;

	while (*key != 0)
	{
		ch = toupper(*key++);

		seed1 = cryptTable[(dwHashType << 8) + ch] ^ (seed1 + seed2);
		seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
	}
	return seed1;
}