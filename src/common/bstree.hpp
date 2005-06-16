#ifndef _BSTREE_HPP_
#define _BSTREE_HPP_

// Binary Search Tree implementation
// requires operator < to be available for the data type


#include <stack>

template <class T>
struct BSTreeNode {
	T data;
	BSTreeNode<T> *left, *right;
	int visit_count;	// used for non-recursive traversal

	bool operator <(const BSTreeNode<T> &node) const;
};

// partial specialization for pointer types
template <class T>
struct BSTreeNode<T*> {
	T* data;
	BSTreeNode<T*> *left, *right;
	int visit_count;	// used for non-recursive traversal
	
	bool operator <(const BSTreeNode<T*> &node) const;
};

enum TraversalOrder {TRAVERSE_INORDER, TRAVERSE_PREORDER, TRAVERSE_POSTORDER};

template <class T>
class BSTree {
private:
	BSTreeNode<T> *root;
	int elem_count;

	// stack for the iterator-like traversal
	mutable std::stack<BSTreeNode<T>*> node_stack;

	void rec_insert(BSTreeNode<T> *&tree, BSTreeNode<T> *node);
	BSTreeNode<T> *rec_remove_node(BSTreeNode<T> *&tree, T data);
	void rec_traverse(BSTreeNode<T> *tree, void (*action)(BSTreeNode<T> *node), TraversalOrder order, bool rev) const;
	BSTreeNode<T> **util_find_min(BSTreeNode<T> *&tree);

	void copy_insert(BSTreeNode<T> *node);

public:

	BSTree();
	BSTree(const BSTree<T> &tree);
	~BSTree();

	inline void insert(BSTreeNode<T> *node);
	inline void insert(T data);
	
	inline BSTreeNode<T> *remove(T data);
	inline void erase(T data);

	BSTreeNode<T> *find(T data);

	void traverse(void (*action)(BSTreeNode<T> *node), TraversalOrder order = TRAVERSE_INORDER) const;
	void traverse_rev(void (*action)(BSTreeNode<T> *node), TraversalOrder order = TRAVERSE_INORDER) const;
	int size() const;

	void reset_iterator() const;
	T *next() const;
};

#include "bstree.inl"	// template definitions

#endif	// _BSTREE_HPP_
