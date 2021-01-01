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
#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

template <class T>
struct ListNode {
	T data;
	ListNode<T> *next, *prev;
};


template <class T>
class LinkedList {
private:
	ListNode<T> *head, *tail;
	int sz;

public:

	LinkedList();
	~LinkedList();

	inline ListNode<T> *begin();
	inline const ListNode<T> *begin() const;
	inline ListNode<T> *end();
	inline const ListNode<T> *end() const;

	void push_back(ListNode<T> *node);
	void push_back(T data);

	void insert(ListNode<T> *pos, ListNode<T> *node);
	void insert(ListNode<T> *pos, T data);

	void remove(ListNode<T> *node);
	ListNode<T> *erase(ListNode<T> *node);

	ListNode<T> *find(T key);

	int count_nodes();
	inline int size() const;

	void operator =(LinkedList &rhs);
};


/////////// implementation //////////
template <class T>
LinkedList<T>::LinkedList() {
	head = tail = 0;
	sz = 0;
}


template <class T>
LinkedList<T>::~LinkedList() {

	while(head) {
		erase(head);
	}
}

template <class T>
ListNode<T> *LinkedList<T>::begin() {
	return head;
}

template <class T>
const ListNode<T> *LinkedList<T>::begin() const {
	return head;
}

template <class T>
ListNode<T> *LinkedList<T>::end() {
	return tail;
}

template <class T>
const ListNode<T> *LinkedList<T>::end() const {
	return tail;
}

template <class T>
void LinkedList<T>::push_back(ListNode<T> *node) {

	if(!head) {		// empty list
		head = tail = node;
		node->next = node->prev = 0;
	} else {
		tail->next = node;
		node->prev = tail;
		tail = node;
		node->next = 0;
	}

	sz++;
}

template <class T>
void LinkedList<T>::push_back(T data) {

	ListNode<T> *node = new ListNode<T>;
	node->data = data;

	if(!head) {		// empty list
		head = tail = node;
		node->next = node->prev = 0;
	} else {
		tail->next = node;
		node->prev = tail;
		tail = node;
		node->next = 0;
	}

	sz++;
}

template <class T>
void LinkedList<T>::insert(ListNode<T> *pos, ListNode<T> *node) {

	if(!head) {
		head = tail = node;
		node->next = node->prev = 0;
	} else {
		node->prev = pos->prev;
		pos->prev = node;
		node->next = pos;
		if(head == pos) head = node; else node->prev->next = node;
	}

	sz++;
}

template <class T>
void LinkedList<T>::insert(ListNode<T> *pos, T data) {

	ListNode<T> *node = new ListNode<T>;
	node->data = data;

	if(!head) {
		head = tail = node;
		node->next = node->prev = 0;
	} else {
		node->prev = pos->prev;
		pos->prev = node;
		node->next = pos;
		if(head == pos) head = node; else node->prev->next = node;
	}

	sz++;
}

template <class T>
void LinkedList<T>::remove(ListNode<T> *node) {

	if(!node) return;	// e.g. remove(head) on an empty list

	if(node->prev) {
		node->prev->next = node->next;
	} else {
		head = node->next;
	}

	if(node->next) {
		node->next->prev = node->prev;
	} else {
		tail = node->prev;
	}

	sz--;
}

template <class T>
ListNode<T> *LinkedList<T>::erase(ListNode<T> *node) {

	if(!node) return 0;	// e.g. erase(head) on an empty list

	if(node->prev) {
		node->prev->next = node->next;
	} else {
		head = node->next;
	}

	if(node->next) {
		node->next->prev = node->prev;
	} else {
		tail = node->prev;
	}

	ListNode<T> *destr = node;
	node = node->next;
	
	delete destr;

	sz--;

	return node;
}

template <class T>
ListNode<T> *LinkedList<T>::find(T key) {
	
	ListNode<T> *iter = head;
	while(iter) {
		if(iter->data == key) return iter;
		iter = iter->next;
	}

	return 0;	// null pointer if not found
}

template <class T>
int LinkedList<T>::count_nodes() {

	sz = 0;

	ListNode<T> *iter = head;
	while(iter) {
		sz++;
		iter = iter->next;
	}

	return sz;
}

template <class T>
int LinkedList<T>::size() const {
	return sz;
}


template <class T>
void LinkedList<T>::operator =(LinkedList<T> &rhs) {
	
	ListNode<T> *src = rhs.begin();
	while(src) {
		push_back(src->data);
		src = src->next;
	}
}


#endif	// _LINKEDLIST_H_
