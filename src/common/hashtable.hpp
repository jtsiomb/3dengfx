/*
Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

This is a hash table implementation with chaining collision resolution.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* Hash table with chaining.
 * 
 * Author: John Tsiombikas 2004
 */

// TODO: create a decent hash table and get rid of this mess.

#ifndef _HASHTABLE_HPP_
#define _HASHTABLE_HPP_

#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

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

	unsigned int (*hash_func)(const KeyType &key, unsigned long size);
	unsigned int hash(const KeyType &key) {return (unsigned int)hash_func(key, (unsigned long)size);}

	void (*data_destructor)(ValType);
public:

	HashTable(unsigned long size = 101);
	~HashTable();

	void set_hash_function(unsigned int (*hash_func)(const KeyType&, unsigned long));

	void insert(KeyType key, ValType value);
	void remove(KeyType key);

	Pair<KeyType, ValType> *find(KeyType key);

	void set_data_destructor(void (*destructor)(ValType));
};


// hash table member functions
template <class KeyType, class ValType>
HashTable<KeyType, ValType>::HashTable(unsigned long size) {
	this->size = size;
	table.resize(size);
	data_destructor = 0;
}

template <class KeyType, class ValType>
HashTable<KeyType, ValType>::~HashTable() {
	for(unsigned long i=0; i<size; i++) {
		if(data_destructor) {
			std::list<ValType> hacklist;

			typename std::list<Pair<KeyType, ValType> >::iterator iter = table[i].begin();
			while(iter != table[i].end()) {
				if(std::find(hacklist.begin(), hacklist.end(), iter->val) == hacklist.end()) {
					hacklist.push_back(iter->val);
					data_destructor((iter++)->val);
				}
			}
		}
			
		table[i].clear();
	}
}

template <class KeyType, class ValType>
void HashTable<KeyType, ValType>::set_hash_function(unsigned int (*hash_func)(const KeyType&, unsigned long)) {
	this->hash_func = hash_func;
}

template <class KeyType, class ValType>
void HashTable<KeyType, ValType>::insert(KeyType key, ValType value) {
	Pair<KeyType, ValType> newpair;
	newpair.key = key;
	newpair.val = value;

	table[hash(key)].push_back(newpair);
}

template <class KeyType, class ValType>
void HashTable<KeyType, ValType>::remove(KeyType key) {

	unsigned int pos = hash(key);

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
Pair<KeyType, ValType> *HashTable<KeyType, ValType>::find(KeyType key) {

	unsigned int pos = hash(key);

	typename std::list<Pair<KeyType, ValType> >::iterator iter = table[pos].begin();
	while(iter != table[pos].end()) {
		if(iter->key == key) {
			return &(*iter);
		}
		iter++;
	}

	return 0;
}

template <class KeyType, class ValType>
void HashTable<KeyType, ValType>::set_data_destructor(void (*destructor)(ValType)) {
	data_destructor = destructor;
}

#endif	// _HASHTABLE_HPP_
