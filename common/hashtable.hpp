/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the eternal demo.

The eternal library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

The eternal demo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the eternal demo; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _HASHTABLE_HPP_
#define _HASHTABLE_HPP_

#include <list>
#include <vector>

template <class KeyT, class ValT> 
struct Pair {
	KeyT key; 
	ValT val;
};

template <class KeyType, class ValType>
class HashTable {
private:

	size_t size;
	std::vector<std::list<Pair<KeyType, ValType> > > table;

	unsigned int (*HashFunc)(const KeyType &key, unsigned long size);
	unsigned int Hash(const KeyType &key) {return (unsigned int)HashFunc(key, (unsigned long)size);}

public:

	HashTable(unsigned long size = 101);
	~HashTable();

	void SetHashFunction(unsigned int (*HashFunc)(const KeyType&, unsigned long));

	void Insert(KeyType key, ValType value);
	void Remove(KeyType key);

	Pair<KeyType, ValType> *Find(KeyType key);
};


// hash table member functions
template <class KeyType, class ValType>
HashTable<KeyType, ValType>::HashTable(unsigned long size) {
	this->size = size;
	table.resize(size);
}

template <class KeyType, class ValType>
HashTable<KeyType, ValType>::~HashTable() {
	for(unsigned long i=0; i<size; i++) {
		table[i].erase(table[i].begin(), table[i].end());
	}
}

template <class KeyType, class ValType>
void HashTable<KeyType, ValType>::SetHashFunction(unsigned int (*HashFunc)(const KeyType&, unsigned long)) {
	this->HashFunc = HashFunc;
}

template <class KeyType, class ValType>
void HashTable<KeyType, ValType>::Insert(KeyType key, ValType value) {
	Pair<KeyType, ValType> newpair;
	newpair.key = key;
	newpair.val = value;

	table[Hash(key)].push_back(newpair);
}

template <class KeyType, class ValType>
void HashTable<KeyType, ValType>::Remove(KeyType key) {

	unsigned int pos = Hash(key);

	typename std::list<Pair<KeyType, ValType> >::iterator iter = table[pos].begin();
	
	while(iter != table[pos].end()) {
		if(iter->key == key) {
			table[pos].erase(iter);
			return;
		}
		iter++;
	}
}

template <class KeyType, class ValType>
Pair<KeyType, ValType> *HashTable<KeyType, ValType>::Find(KeyType key) {

	unsigned int pos = Hash(key);

	typename std::list<Pair<KeyType, ValType> >::iterator iter = table[pos].begin();
	while(iter != table[pos].end()) {
		if(iter->key == key) {
			return &(*iter);
		}
		iter++;
	}

	return 0;
}

#endif	// _HASHTABLE_HPP_
