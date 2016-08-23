#ifndef _DATAMANAGER_H
#define _DATAMANAGER_H

#include <vector>
#include <string>
#include <map>
#include "facilities.h"

using namespace typedefs;
using namespace std;

struct Pair {
	string key;
	string value;
};


class DataManager {
public:
	DataManager(const string & _datafilepath, const string& _indexfilepath);
	~DataManager();
	
	filepos insert(const string & value);
	string get(filepos value_pos);
	// if succeed, return true
	bool erase(filepos value_pos);

	vector<filepos> insert_multiple(vector<Pair>& pair);


private:
	string data_file_path, index_file_path;
	string meta_file_path;
	// save the erased entries
	multimap<valuelen, filepos> erased;


	bool save_meta();
	bool read_meta();

	bool create_new_data();
	
	bool overwrite(filepos value_pos, const string & str);
	filepos appendwrite(const string & str);
	filepos appendwrite(const string & str, ofstream& ofs);

	
	// get the real length of the string (including '\0')
	valuelen get_length(const string & str) { return str.length() + 1; }
};


#endif

