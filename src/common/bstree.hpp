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

	void RecInsert(BSTreeNode<T> *&tree, BSTreeNode<T> *node);
	BSTreeNode<T> *RecRemoveNode(BSTreeNode<T> *&tree, T data);
	void RecTraverse(BSTreeNode<T> *tree, void (*action)(BSTreeNode<T> *node), TraversalOrder order, bool rev) const;
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
	void TraverseRev(void (*action)(BSTreeNode<T> *node), TraversalOrder order = TRAVERSE_INORDER) const;
	int Size() const;

	void ResetIterator() const;
	T *Next() const;
};

#include "bstree.inl"	// template definitions

#endif	// _BSTREE_HPP_
