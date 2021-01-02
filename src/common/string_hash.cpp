// This code belongs to the public domain.

#include <string>
#include <string.h>

/*
 * Hashing algorithm for strings from:
 * Sedgewick's "Algorithms in C++, third edition" 
 * parts 1-4, Chapter 14 (hashing) p.593
 * 
 * Modified to work on C++ string class objects by John Tsiombikas
 */
unsigned int string_hash(const std::string &key, unsigned long size) {
	int hash = 0, a = 31415, b = 27183;
	char *str = new char[key.length() + 1];
	strcpy(str, key.c_str());

	char *sptr = str;
	
	while(*sptr) {
		hash = (a * hash + *sptr++) % size;
		a = a * b % (size - 1);
	}
	
	delete [] str;
	
	return (unsigned int)(hash < 0 ? (hash + size) : hash);
}
