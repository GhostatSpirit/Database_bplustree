#ifndef _FACILITIES_H
#define _FACILITIES_H


namespace typedefs {
	typedef unsigned long filepos;
	typedef unsigned valuelen;
	typedef unsigned keylen;
	typedef unsigned long hashval;
	
}

template<class T> char* as_bytes(T& i) {
	void* addr = &i;
	return static_cast<char*>(addr);
}



#endif 