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
#ifndef _BSTREE_HPP_
#define _BSTREE_HPP_

// Binary Search Tree implementation
// requires operator < to be available for the data type

template <class T>
struct BSTreeNode {
	T data;
	BSTreeNode<T> *left, *right;

	bool operator <(const BSTreeNode<T> &node) const;
};

// partial specialization for pointer types
template <class T>
struct BSTreeNode<T*> {
	T* data;
	BSTreeNode<T*> *left, *right;
	
	bool operator <(const BSTreeNode<T*> &node) const;
};

enum TraversalOrder {TRAVERSE_INORDER, TRAVERSE_PREORDER, TRAVERSE_POSTORDER};

template <class T>
class BSTree {
private:
	BSTreeNode<T> *root;
	int elem_count;

	void RecInsert(BSTreeNode<T> *&tree, BSTreeNode<T> *node);
	BSTreeNode<T> *RecRemoveNode(BSTreeNode<T> *&tree, T data);
	void RecTraverse(BSTreeNode<T> *tree, void (*action)(BSTreeNode<T> *node), TraversalOrder order) const;
	BSTreeNode<T> **UtilFindMin(BSTreeNode<T> *&tree);

	void CopyInsert(BSTreeNode<T> *node);

public:

	BSTree();
	BSTree(const BSTree<T> &tree);
	~BSTree();

	inline void Insert(BSTreeNode<T> *node);
	inline void Insert(T data);
	
	inline BSTreeNode<T> *Remove(T data);
	inline void Erase(T data);

	BSTreeNode<T> *Find(T data);

	void Traverse(void (*action)(BSTreeNode<T> *node), TraversalOrder order = TRAVERSE_INORDER) const;
	int Size() const;
};

#include "bstree.inl"	// template definitions

#endif	// _BSTREE_HPP_
