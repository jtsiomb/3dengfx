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
// Template definitions, included in the bstree.h header file

template <class T>
bool BSTreeNode<T>::operator <(const BSTreeNode<T> &node) const {
	return (data < node.data);
}

template <class T>
bool BSTreeNode<T*>::operator <(const BSTreeNode<T*> &node) const {
	return (*data < *node.data);
}

// --- class BSTree ---

template <class T>
BSTree<T>::BSTree() {
	root = 0;
	elem_count = 0;
}

template <class T>
BSTree<T>::BSTree(const BSTree<T> &tree) {
	elem_count = tree.elem_count;
	tree.Traverse(CopyInsert, PreorderTraversal);
}

template <class T>
BSTree<T>::~BSTree() {
	while(root) Erase(root->data);
}

template <class T>
void BSTree<T>::CopyInsert(BSTreeNode<T> *node) {
	BSTreeNode<T> *newnode = new BSTreeNode<T>;
	*newnode = *node;
	newnode->left = newnode->right = 0;

	RecInsert(root, newnode);
}

template <class T>
void BSTree<T>::RecInsert(BSTreeNode<T> *&tree, BSTreeNode<T> *node) {
	if(!tree) {
		tree = node;
		node->left = node->right = 0;
		elem_count++;
		return;
	}
	if(*node < *tree) {
		RecInsert(tree->left, node);
		return;
	}
	if(*tree < *node) {
		RecInsert(tree->right, node);
		return;
	}
}

template <class T>
inline void BSTree<T>::Insert(BSTreeNode<T> *node) {
	RecInsert(root, node);
}

template <class T>
inline void BSTree<T>::Insert(T data) {
	BSTreeNode<T> *node = new BSTreeNode<T>;
	node->data = data;
	
	RecInsert(root, node);
}

template <class T>
BSTreeNode<T> **BSTree<T>::UtilFindMin(BSTreeNode<T> *&tree) {
	if(!tree) return 0;
	if(!tree->left) return &tree;
	return UtilFindMin(tree->left);
}

template <class T>
BSTreeNode<T> *BSTree<T>::RecRemoveNode(BSTreeNode<T> *&tree, T data) {
	if(!tree) return 0;
	// find the node
	BSTreeNode<T> tmp;
	tmp.data = data;
	if(tmp < *tree) return RecRemoveNode(tree->left, data);
	if(*tree < tmp) return RecRemoveNode(tree->right, data);
	
	// found it
	if(!tree->left && !tree->right) {
		BSTreeNode<T> *tmp = tree;
		tree = 0;
        return tmp;
	}
	if((!tree->left && tree->right) || (tree->left && !tree->right)) {
		BSTreeNode<T> *tmp = tree;
		tree = (tree->left) ? tree->left : tree->right;
		return tmp;
	}
	// else both subtrees exist		
	BSTreeNode<T> **min = UtilFindMin(tree->right);
	tree->data = (*min)->data;
	return RecRemoveNode(*min, (*min)->data);
}

template <class T>
inline BSTreeNode<T> *BSTree<T>::Remove(T data) {
	BSTreeNode<T> *node = RecRemoveNode(root, data);
	if(node) {
		elem_count--;
		return node;
	}
	return 0;
}

template <class T>
inline void BSTree<T>::Erase(T data) {
	BSTreeNode<T> *node = RecRemoveNode(root, data);
	if(node) {
		elem_count--;
		delete node;
	}
}

template <class T>
BSTreeNode<T> *BSTree<T>::Find(T data) {
	BSTreeNode<T> *node = root;
	BSTreeNode<T> tmp;
	tmp.data = data;
	
    while(node) {
		if(tmp < *node) {
			node = node->left;
			continue;
		}
		if(*node < tmp) {
			node = node->right;
			continue;
		}
		return node;
	}
	return 0;
}


template <class T>
void BSTree<T>::RecTraverse(BSTreeNode<T> *tree, void (*action)(BSTreeNode<T> *node), TraversalOrder order) const {
	if(!tree) return;

	if(order == TRAVERSE_PREORDER) action(tree);
	RecTraverse(tree->left, action, order);
	if(order == TRAVERSE_INORDER) action(tree);
	RecTraverse(tree->right, action, order);
	if(order == TRAVERSE_POSTORDER) action(tree);
}

template <class T>
inline void BSTree<T>::Traverse(void (*action)(BSTreeNode<T> *node), TraversalOrder order) const {
	RecTraverse(root, action, order);
}

template <class T>
int BSTree<T>::Size() const {
	return ElementCount;
}
