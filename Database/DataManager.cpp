#include "DataManager.h"
#include "facilities.h"
#include <fstream>
#include <iostream>

DataManager::DataManager(const string & _datafilepath, const string & _indexfilepath)
{
	// save the file paths
	data_file_path = _datafilepath;
	index_file_path = _indexfilepath;

	// set the meta file path
	meta_file_path = data_file_path + ".meta";

	// try to open the data file
	ifstream ifs(data_file_path, ios::binary);

	if (!ifs) {
		// if we do not have the data file...
		ifs.close();
		// create a new data file
		create_new_data();
		// save the blank map "erased"
		save_meta();
	}
	else {
		// if we have the data file...
		// try to read the meta file first
		ifstream meta_ifs(meta_file_path, ios::binary);
		bool meta_ifs_valid = true;
		if (!meta_ifs) {
			meta_ifs_valid = false;
		}

		if (meta_ifs_valid) {
			// if the meta file exists, read it
			read_meta();
		}
		else {
			// else, save a new blank meta file
			save_meta();
		}
	}


}

DataManager::~DataManager() {
	// save the meta file
	save_meta();
}

filepos DataManager::insert(const string & value)
{
	// first try to find a space for the string in the erased map
	// +1: including '\0' at the end of the string
	valuelen value_len = get_length(value);

	multimap<valuelen, filepos>::iterator it;
	it = erased.find(value_len);

	filepos new_pos = 0;

	if (it != erased.end()) {
		// if we can find the filepos in erased
		new_pos = it->second;
		overwrite(new_pos, value);
		// delete the overwritten entry from erased
		erased.erase(it);
	}
	else{
		// if we cannot find a filepos in erased, use appendwrite
		new_pos = appendwrite(value);
	}

	return new_pos;
}

bool DataManager::save_meta()
{
	ofstream ofs(meta_file_path, ios::binary | ios::trunc); // trunc: clearing the content
	if (!ofs) {
		return false;
	}
	ofs.seekp(0, ofs.beg);

	unsigned long invalid_entry_count = erased.size();

	// ------- writing header -------
	// unsigned long invalid_entry_count
	ofs.write(as_bytes(invalid_entry_count), sizeof(unsigned long));

	// ------- writing content -------
	multimap<valuelen, filepos>::iterator it = erased.begin();
	for (; it != erased.end(); ++it){
		// single entry:
		valuelen entry_len = it->first;
		filepos entry_pos = it->second;
		// valuelen value_length
		ofs.write(as_bytes(entry_len), sizeof(valuelen));
		// filepos value_pos
		ofs.write(as_bytes(entry_pos), sizeof(filepos));
	}

	ofs.close();

	return true;
}

bool DataManager::read_meta()
{
	ifstream ifs(meta_file_path, ifstream::binary);
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
		valuelen entry_len;
		filepos entry_pos;
		// valuelen value_length
		ifs.read(as_bytes(entry_len), sizeof(valuelen));
		// filepos value_pos
		ifs.read(as_bytes(entry_pos), sizeof(filepos));
		// insert the <valuelen, filepos> pair to the map
		erased.insert(std::pair<valuelen, filepos>(entry_len, entry_pos));
	}

	ifs.close();
	return true;
	
}

bool DataManager::create_new_data()
{
	ofstream ofs(data_file_path, ofstream::binary | ofstream::trunc);
	if (!ofs) return false;

	valuelen index_str_len = get_length(index_file_path);
	//----------- writing data file header ------------
	
	// valuelen index_path_length
	ofs.write(as_bytes(index_str_len), sizeof(valuelen));
	// string index_path
	ofs.write(index_file_path.c_str(), sizeof(char) * index_str_len);

	ofs.close();

	return true;
}

bool DataManager::overwrite(filepos value_pos, const string & str)
{
	valuelen new_val_len = get_length(str);
	valuelen old_val_len = 0;
	bool new_valid = true;
	bool old_valid;

	// check if the size of the old entry fits the new string
	fstream fs(data_file_path, ios::binary | ios::in | ios::out);
	fs.seekg(value_pos);
	// bool valid
	fs.read(as_bytes(old_valid), sizeof(bool));
	// valuelen value_length
	fs.read(as_bytes(old_val_len), sizeof(valuelen));

	if (old_val_len != new_val_len) {
		cerr << "overwrite: new old val len not match" << endl;
		return false;
	}
	// set the write cursor pos to the read cursor pos
	fs.seekp(fs.tellg());
	// write the string
	string temp_str = str;
	fs.write(temp_str.c_str(), sizeof(char) * new_val_len);

	// return to the beginning of the entry and rewrite bool valid as 'true'
	fs.seekp(value_pos);
	fs.write(as_bytes(new_valid), sizeof(bool));

	// save data file
	fs.close();

	return true;
}

filepos DataManager::appendwrite(const string & str)
{
	valuelen value_length = get_length(str);
	ofstream ofs(data_file_path, ofstream::binary | ofstream::app);
	if (!ofs) {
		return 0;
	}
	ofs.seekp(0, ofs.end);
	// saving cursor pos as new_value_pos
	filepos new_value_pos = ofs.tellp();

	// writing data entry
	bool valid = true;
	// bool valid
	ofs.write(as_bytes(valid), sizeof(bool));
	// valuelen value_length
	ofs.write(as_bytes(value_length), sizeof(valuelen));
	// string value (including '\0')
	string temp_str = str;
	ofs.write(temp_str.c_str(), sizeof(char) * value_length);

	// saving data file
	ofs.close();

	return new_value_pos;
}

filepos DataManager::appendwrite(const string & str, ofstream& ofs) {
	if (!ofs) {
		return 0;
	}
	ofs.seekp(0, ofs.end);
	// saving cursor pos as new_value_pos
	filepos new_value_pos = ofs.tellp();

	valuelen value_length = get_length(str);
	// writing data entry
	bool valid = true;
	// bool valid
	ofs.write(as_bytes(valid), sizeof(bool));
	// valuelen value_length
	ofs.write(as_bytes(value_length), sizeof(valuelen));
	// string value (including '\0')
	string temp_str = str;
	ofs.write(temp_str.c_str(), sizeof(char) * value_length);

	return new_value_pos;
}


string DataManager::get(filepos value_pos) {
	ifstream ifs(data_file_path, ios::binary);
	if (!ifs) {
		cerr << "DataManger::get: cannot open data file" << endl;
		return "";
	}
	
	ifs.seekg(value_pos);

	bool valid;
	valuelen length;

	// bool valid
	ifs.read(as_bytes(valid), sizeof(bool));
	if (valid == false) {
		cerr << "DataManger::get: entry invalid" << endl;
	}
	// valuelen value_length
	ifs.read(as_bytes(length), sizeof(valuelen));
	// string value
	char* p_cstring = new char[length];
	ifs.read(p_cstring, sizeof(char) * length);
	string value(p_cstring);
	delete[] p_cstring;

	ifs.close();
	return value;
}

bool DataManager::erase(filepos value_pos)
{
	fstream fs(data_file_path, ios::binary|ios::in|ios::out);
	if (!fs) {
		return false;
	}
	fs.seekg(value_pos);

	bool old_valid;
	valuelen length = 0;

	fs.read(as_bytes(old_valid), sizeof(bool));
	if (old_valid == false) {
		cerr << "erase: erase mupltiple times" << endl;
		return false;
	}
	fs.read(as_bytes(length), sizeof(valuelen));

	// now the value is no longer valid
	bool new_valid = false;
	fs.seekp(value_pos);
	fs.write(as_bytes(new_valid), sizeof(bool));

	fs.close();

	// insert the new <valuelen, filepos> pair to the multimap
	erased.insert(std::pair<valuelen, filepos>(length, value_pos));

	return true;
}

