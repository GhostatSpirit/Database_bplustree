#ifndef _FACILITIES_H
#define _FACILITIES_H

template<class T> char* as_bytes(T& i) {
	void* addr = &i;
	return static_cast<char*>(addr);
}


#endif 