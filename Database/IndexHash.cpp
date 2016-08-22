#include "IndexHash.h"

#include <fstream>
#include <iostream>
#include <vector>
#include "facilities.h"





IndexHash::IndexHash(const string & _file_name, unsigned long _table_size):

	// initialize data manager
	data_man(_file_name + ".dat", _file_name + ".idx")
{
	// initialize crypt table
	InitCryptTable();
	// save the table length
	table_size = _table_size;
	// save the file name
	file_name = _file_name;
	// generate the name for the different files
	m_index_path = file_name + ".idx";						// database.idx
	m_index_meta_path = m_index_path + ".meta";				// database.idx.meta
	m_data_path = file_name + ".dat";

	first_bucket_pos = 0;

	m_hash_table.reserve(_table_size);

	// try to open the hash table file
	ifstream ifs(m_index_path, ios::binary);
	if (!ifs) {
		// if we do not have the hash table file...
		// initialize a new hash table and write it
		ifs.close();
		InitHashTable();
		WriteNewHeader();
		// save the blank map "erased"
		save_meta();
	}
	else {
		// if we have the hash table file,
		// we need to read it to the memory
		ifs.close();
		ReadHeader();
		// try to read the meta file, if failed, create a new one
		ifstream meta_ifs(m_index_meta_path, ios::binary);
		if (meta_ifs) {
			// if the meta file exists, read it
			read_meta();
		}
		else {
			// else, save a new blank meta file
			save_meta();
		}
		meta_ifs.close();
	}



}

IndexHash::~IndexHash() {
	WriteHeader();
	save_meta();
}

string IndexHash::db_fetch(const string & key)
{
	hashval hash_value;
	bool succeeded = get_hash(key, hash_value);
	if (!succeeded) {
		return "";
	}

	// if the bucket is being used
	// get the index pos
	filepos index_pos = m_hash_table[hash_value].index_pos;
	filepos data_pos = GetDataPos(index_pos, key);
	if (data_pos != 0) {
		return data_man.get(data_pos);
	}
	else {
		return "";
	}
		
}

int IndexHash::db_delete(const string & key)
{
	// get hash_value of the key
	hashval hash_value;
	bool gethash_succeeded = get_hash(key, hash_value);
	if (!gethash_succeeded) return -1;
	// get index_pos of the key (first index in the list)
	filepos index_pos = m_hash_table[hash_value].index_pos;
	if (index_pos == 0) return -2;
	// try to delete the key
	bool delete_succeeded = DeleteEntry(index_pos, key);
	if (!delete_succeeded) return -3;
	return 0;
}

bool IndexHash::db_exist(const string & key)
{
	hashval hash_value;
	bool succeeded = get_hash(key, hash_value);
	if (!succeeded) {
		return false;
	}
	// if the bucket is being used
	// get the index pos
	filepos index_pos = m_hash_table[hash_value].index_pos;
	filepos data_pos = GetDataPos(index_pos, key);
	if (data_pos != 0) {
		return true;
	}
	else {
		return false;
	}
}



int IndexHash::db_store(const string& key, const string& value, int flag) {
	bool key_exists = db_exist(key);
	if (key_exists) {
		return -1;
	}
	else {
		// try to find an erased index in the multimap erased
		keylen key_len = get_length(key);

		multimap<keylen, filepos>::iterator it;
		it = erased.find(key_len);

		filepos new_pos = 0;

		if (it != erased.end()) {
			// if we can find the filepos in erased
			new_pos = it->second;
			bool flag = OverwriteIndex(new_pos, key, value);
			// delete the overwritten entry from erased
			erased.erase(it);
			if (flag != true) {
				return -2;
			}
		}
		else {
			// if we cannot find a filepos in erased, use AppendIndex
			new_pos = AppendIndex(key, value);
		}

		hashval hash_value;
		bool collision = get_hash(key, hash_value);
		if (!collision) {
			// if we cannot get the hash value of the given key
			// means no collision has happened
			bool create_succeeded = create_hash(key, hash_value);
			if (!create_succeeded) {
				return -3;
			}
			m_hash_table[hash_value].index_pos = new_pos;
		}
		else {
			// if we succeeded in getting the hash value,
			// means there is a collision
			// add the new index to the end of the list
			filepos current_index_pos = m_hash_table[hash_value].index_pos;
			while (true) {
				filepos next_index_pos = GetNextIndexPos(current_index_pos);
				if (next_index_pos == 0) {
					break;
				}
				current_index_pos = next_index_pos;
			}
			SetNextIndexPos(current_index_pos, new_pos);
		}

		return 0;

	  }

	
	return -4;

}


bool IndexHash::save_meta()
{

	ofstream ofs(m_index_meta_path, ios::binary | ios::trunc); // trunc: clearing the content
	if (!ofs) {
		return false;
	}
	ofs.seekp(0, ofs.beg);

	unsigned long invalid_entry_count = erased.size();

	// ------- writing header -------
	// unsigned long invalid_entry_count
	ofs.write(as_bytes(invalid_entry_count), sizeof(unsigned long));

	// ------- writing content -------
	multimap<keylen, filepos>::iterator it = erased.begin();
	for (; it != erased.end(); ++it) {
		// single entry:
		keylen entry_len = it->first;
		filepos entry_pos = it->second;
		// keylen value_length
		ofs.write(as_bytes(entry_len), sizeof(keylen));
		// filepos value_pos
		ofs.write(as_bytes(entry_pos), sizeof(filepos));
	}

	ofs.close();

	return true;
}


bool IndexHash::read_meta()
{
	ifstream ifs(m_index_meta_path, ifstream::binary);
	if (!ifs) {
		return false;
	}

	ifs.seekg(0, ifs.beg);

	unsigned long invalid_entry_count;

	// ------- reading header -------
	// unsigned long invalid_entry_count
	ifs.read(as_bytes(invalid_entry_count), sizeof(unsigned long));

	// clear the map first
	erased.clear();

	// ------- reading content -------
	for (unsigned long i = 0; i < invalid_entry_count; ++i) {
		// single entry:
		keylen entry_len;
		filepos entry_pos;
		// keylen value_length
		ifs.read(as_bytes(entry_len), sizeof(keylen));
		// filepos value_pos
		ifs.read(as_bytes(entry_pos), sizeof(filepos));
		// insert the <keylen, filepos> pair to the map
		erased.insert(std::pair<keylen, filepos>(entry_len, entry_pos));
	}

	ifs.close();
	return true;

}

filepos IndexHash::AppendIndex(const string & key, const string & value, filepos next_index_pos)
{
	// insert the value string into the 
	filepos value_pos = data_man.insert(value);
	if (value_pos == 0) {
		throw runtime_error("AppendEntry: cannot insert value string");
		return 0;
	}
	ofstream ofs(m_index_path, ofstream::binary | ofstream::app);
	if (!ofs) {
		return 0;
	}
	ofs.seekp(0, ofs.end);
	// saving cursor pos as new_value_pos
	filepos new_index_pos = ofs.tellp();
	// write the new index entry
	bool valid = true;
	// bool valid
	ofs.write(as_bytes(valid), sizeof(bool));
	// filepos next_index_pos
	ofs.write(as_bytes(next_index_pos), sizeof(filepos));
	// keylen key_length
	keylen key_length = get_length(key);
	ofs.write(as_bytes(key_length), sizeof(keylen));
	string temp_key = key;
	ofs.write(temp_key.c_str(), sizeof(char) * key_length);
	// filepos value_pos
	ofs.write(as_bytes(value_pos), sizeof(filepos));
	// char spliter
	ofs.write(as_bytes(spliter), sizeof(char));

	ofs.close();
	return new_index_pos;

}

bool IndexHash::OverwriteIndex(filepos index_pos, const string& key, const string& value, filepos next_index_pos) {
	fstream fs(m_index_path, ios::binary | ios::in | ios::out);
	fs.seekg(index_pos);
	// examine the old entry first
	bool old_valid;
	filepos old_next_pos;
	keylen old_key_len = 0;
	keylen new_key_len = get_length(key);

	fs.read(as_bytes(old_valid), sizeof(bool));
	fs.read(as_bytes(old_next_pos), sizeof(filepos));
	fs.read(as_bytes(old_key_len), sizeof(keylen));
	if (old_valid == true) {
		cerr << "OverwriteIndex: overwriting valid index" << endl;
		return false;
	}
	if (new_key_len != old_key_len) {
		cerr << "OverwriteIndex: old/new key length not match" << endl;
		return false;
	}
	
	// now we can safely overwrite this index entry
	fs.seekg(index_pos);
	fs.seekp(index_pos);

	filepos value_pos = data_man.insert(value);
	if (value_pos == 0) {
		throw runtime_error("OverwriteIndex: cannot insert value string");
		return 0;
	}

	bool valid = true;
	// bool valid
	fs.write(as_bytes(valid), sizeof(bool));
	// filepos next_index_pos
	fs.write(as_bytes(next_index_pos), sizeof(filepos));
	// keylen key_length
	fs.write(as_bytes(new_key_len), sizeof(keylen));
	// string key
	string temp_key = key;
	fs.write(temp_key.c_str(), sizeof(char) * new_key_len);
	// filepos value_pos
	fs.write(as_bytes(value_pos), sizeof(filepos));
	// char spliter
	fs.write(as_bytes(spliter), sizeof(char));

	fs.close();
	return true;
}

bool IndexHash::SetNextIndexPos(filepos index_pos, filepos next_index_pos) {
	if (index_pos == 0) {
		cerr << "SetNextIndexPos: index_pos is null" << endl;
		return false;
	}
	fstream fs(m_index_path, ios::binary | ios::in | ios::out);
	fs.seekg(index_pos);
	// check if the entry is valid
	bool valid;
	fs.read(as_bytes(valid), sizeof(bool));
	if (!valid) {
		cerr << "SetNextIndexPos: reading invalid entry" << endl;
		return false;
	}
	fs.seekp(fs.tellg());
	fs.write(as_bytes(next_index_pos), sizeof(filepos));
	fs.close();
	return true;
}

filepos IndexHash::GetNextIndexPos(filepos index_pos)
{
	ifstream ifs(m_index_path, ios::binary);
	ifs.seekg(index_pos);
	// save the two important values
	bool valid;
	filepos next_pos;
	ifs.read(as_bytes(valid), sizeof(bool));
	ifs.read(as_bytes(next_pos), sizeof(filepos));
	ifs.close();
	if (valid == false) {
		cerr << "GetNextIndexPos: reading invalid entry" << endl;
		return 0;
	}
	else {
		return next_pos;
	}
}


bool IndexHash::DeleteEntry(filepos index_pos, const string& deleted_key, filepos last_index_pos)
{
	ifstream ifs(m_index_path, ios::binary);
	if (!ifs) {
		return false;
	}
	ifs.seekg(index_pos);
	
	// store all useful index entry info
	bool valid;
	filepos next_index_pos;
	keylen key_length;
	filepos value_pos;

	ifs.read(as_bytes(valid), sizeof(bool));
	// filepos next_index_pos
	ifs.read(as_bytes(next_index_pos), sizeof(filepos));
	// keylen key_length
	ifs.read(as_bytes(key_length), sizeof(keylen));

	// string key
	char* p_cstring = new char[key_length];
	ifs.read(p_cstring, sizeof(char) * key_length);
	string key(p_cstring);
	delete[] p_cstring;

	// filepos value_pos
	ifs.read(as_bytes(value_pos), sizeof(filepos));
	ifs.close();

	// find recursively 
	if (deleted_key == key && valid == true) {
		// if we found the entry we want to delete
		// 1. erase the correspoding value string
		data_man.erase(value_pos);
		// 2. reset the next_index_pos relation
		if (last_index_pos != 0) {
			SetNextIndexPos(last_index_pos, next_index_pos);
		}
		// 3. set this entry as invalid and add it to the erased map
		fstream fs(m_index_path, ios::binary | ios::in | ios::out);
		fs.seekp(index_pos);
		bool new_valid = false;
		fs.write(as_bytes(new_valid), sizeof(bool));
		fs.close();
		erased.insert(std::pair<keylen, filepos>(key_length, index_pos));
		// 4. if this index is the last index in the list, free the corresponding bucket
		if (next_index_pos == 0 && last_index_pos == 0) {
			hashval hash_pos;
			bool succeeded = get_hash(deleted_key, hash_pos);
			if (!succeeded) {
				cerr << "cannot free up bucket" << endl;
				return false;
			}
			else {
				m_hash_table[hash_pos].used = false;
			}
		}
		// 5. if this index is the first in the list and has the next index,
		//    reset the index_pos pointer in the bucket
		if (last_index_pos == 0 && next_index_pos != 0) {
			hashval hash_pos;
			bool succeeded = get_hash(deleted_key, hash_pos);
			if (!succeeded) {
				cerr << "cannot free up bucket" << endl;
				return false;
			}
			else {
				m_hash_table[hash_pos].index_pos = next_index_pos;
			}
		}
		return true;
	}
	else if (next_index_pos != 0 && valid == true) {
		// if the key does not match but we have a next_index_pos
		// call this function recursively
		return DeleteEntry(next_index_pos, deleted_key, index_pos);
	}
	else {
		// we cannot find the matching key
		if (valid == false) {
			cerr << "FindEntry: reached invalid entry" << endl;
		}
		return false;
	}

	return false;

}

filepos IndexHash::GetDataPos(filepos index_pos, const string & target_key)
{
	if (index_pos == 0) {
		cout << "error1";
		return 0;
	}
	ifstream ifs(m_index_path, ios::binary);
	if (!ifs) {
		cout << "error2";
		return 0;
	}
	ifs.seekg(index_pos);

	// store all useful index entry info
	bool valid;
	filepos next_index_pos;
	keylen key_length;
	filepos value_pos;

	ifs.read(as_bytes(valid), sizeof(bool));
	// filepos next_index_pos
	ifs.read(as_bytes(next_index_pos), sizeof(filepos));
	// keylen key_length
	ifs.read(as_bytes(key_length), sizeof(keylen));

	// string key
	char* p_cstring = new char[key_length];
	ifs.read(p_cstring, sizeof(char) * key_length);
	string key(p_cstring);
	delete[] p_cstring;

	// filepos value_pos
	ifs.read(as_bytes(value_pos), sizeof(filepos));
	ifs.close();

	// find recursively 
	if (target_key == key && valid == true) {
		// if we find the key, return the filepos for the value
		return value_pos;
	}
	else if (next_index_pos != 0 && valid == true) {
		// if the key does not match but we have a next_index_pos
		// call this function recursively
		return GetDataPos(next_index_pos, target_key);
	}
	else {
		// we cannot find the matching key, return null value
		if (valid == false) {
			cerr << "FindEntry: reached invalid entry" << endl;
		}
		return 0;
	}

	return 0;
}


filepos IndexHash::ConvertToBucketPos(hashval hash_value)
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

bool IndexHash::WriteNewHeader() {
	ofstream ofs(m_index_path, ios::binary | ios::trunc);
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

	ofs.write(as_bytes(spliter), sizeof(char));

	ofs.close();

	return true;
}

bool IndexHash::WriteHeader() {
	// using basic fstream to prevent truncating file
	fstream ofs(m_index_path, ios::binary | ios::in | ios::out);

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

	ofs.write(as_bytes(spliter), sizeof(char));

	ofs.close();


	return true;
}

bool IndexHash::ReadHeader() {
	ifstream ifs(m_index_path, ios::binary);
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

	ifs.close();

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

bool IndexHash::get_hash(const string & str, hashval& hash_value)
{
	// calculate the three hash values of the given string
	hashval nHash = HashString(str, HASH_OFFSET);
	hashval nHashA = HashString(str, HASH_A);
	hashval nHashB = HashString(str, HASH_B);
	// get the bucket start position of the given string
	hashval nHashStart = nHash % table_size, nHashPos = nHashStart;
	// examine the next bucket in the hash table until found
	while (m_hash_table[nHashPos].used)
	{
		if (m_hash_table[nHashPos].hash_A == nHashA && m_hash_table[nHashPos].hash_B == nHashB) {
			hash_value = nHashPos;
			return true;
		}
		else {
			nHashPos = (nHashPos + 1) % table_size;
		}
		// if we reach the start pos again, then we cannot find it
		if (nHashPos == nHashStart)
			break;
	}
	hash_value = 0;
	return false;
}

bool IndexHash::create_hash(const string & str, hashval & hash_value) {
	// calculate the three hash values of the given string
	hashval nHash = HashString(str, HASH_OFFSET);
	hashval nHashA = HashString(str, HASH_A);
	hashval nHashB = HashString(str, HASH_B);
	hashval nHashStart = nHash % table_size;
	hashval nHashPos = nHashStart;
	while (m_hash_table[nHashPos].used)
	{
		nHashPos = (nHashPos + 1) % table_size;
		if (nHashPos == nHashStart)
		{
			// no spare space in hash table (hash table is full)
			hash_value = 0;
			return false;
		}
	}
	// now we have an unused bucket in the hash table
	// save the two hash values in it
	m_hash_table[nHashPos].used = true;
	m_hash_table[nHashPos].hash_A = nHashA;
	m_hash_table[nHashPos].hash_B = nHashB;
	hash_value = nHashPos;
	return true;
}