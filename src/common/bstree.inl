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
	tree.traverse(copy_insert, TRAVERSE_PREORDER);
}

template <class T>
BSTree<T>::~BSTree() {
	while(root) erase(root->data);
}

template <class T>
void BSTree<T>::copy_insert(BSTreeNode<T> *node) {
	BSTreeNode<T> *newnode = new BSTreeNode<T>;
	*newnode = *node;
	newnode->left = newnode->right = 0;

	rec_insert(root, newnode);
}

template <class T>
void BSTree<T>::rec_insert(BSTreeNode<T> *&tree, BSTreeNode<T> *node) {
	if(!tree) {
		tree = node;
		node->left = node->right = 0;
		elem_count++;
		return;
	}
	if(*node < *tree) {
		rec_insert(tree->left, node);
		return;
	}
	if(*tree < *node) {
		rec_insert(tree->right, node);
		return;
	}
}

template <class T>
inline void BSTree<T>::insert(BSTreeNode<T> *node) {
	rec_insert(root, node);
}

template <class T>
inline void BSTree<T>::insert(T data) {
	BSTreeNode<T> *node = new BSTreeNode<T>;
	node->data = data;
	
	rec_insert(root, node);
}

template <class T>
BSTreeNode<T> **BSTree<T>::util_find_min(BSTreeNode<T> *&tree) {
	if(!tree) return 0;
	if(!tree->left) return &tree;
	return util_find_min(tree->left);
}

template <class T>
BSTreeNode<T> *BSTree<T>::rec_remove_node(BSTreeNode<T> *&tree, T data) {
	if(!tree) return 0;
	// find the node
	BSTreeNode<T> tmp;
	tmp.data = data;
	if(tmp < *tree) return rec_remove_node(tree->left, data);
	if(*tree < tmp) return rec_remove_node(tree->right, data);
	
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
	BSTreeNode<T> **min = util_find_min(tree->right);
	tree->data = (*min)->data;
	return rec_remove_node(*min, (*min)->data);
}

template <class T>
inline BSTreeNode<T> *BSTree<T>::remove(T data) {
	BSTreeNode<T> *node = rec_remove_node(root, data);
	if(node) {
		elem_count--;
		return node;
	}
	return 0;
}

template <class T>
inline void BSTree<T>::erase(T data) {
	BSTreeNode<T> *node = rec_remove_node(root, data);
	if(node) {
		elem_count--;
		delete node;
	}
}

template <class T>
BSTreeNode<T> *BSTree<T>::find(T data) {
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
void BSTree<T>::rec_traverse(BSTreeNode<T> *tree, void (*action)(BSTreeNode<T> *node), TraversalOrder order, bool rev) const {
	if(!tree) return;

	if(order == TRAVERSE_PREORDER) action(tree);
	rec_traverse(rev ? tree->right : tree->left, action, order, rev);
	if(order == TRAVERSE_INORDER) action(tree);
	rec_traverse(rev ? tree->left : tree->right, action, order, rev);
	if(order == TRAVERSE_POSTORDER) action(tree);
}

template <class T>
inline void BSTree<T>::traverse(void (*action)(BSTreeNode<T> *node), TraversalOrder order) const {
	rec_traverse(root, action, order, false);
}

template <class T>
inline void BSTree<T>::traverse_rev(void (*action)(BSTreeNode<T> *node), TraversalOrder order) const {
	rec_traverse(root, action, order, true);
}

template <class T>
int BSTree<T>::size() const {
	return elem_count;
}


template <class T>
void BSTree<T>::reset_iterator() const {
	while(!node_stack.empty()) node_stack.pop();

	node_stack.push(root);
}

template <class T>
T *BSTree<T>::next() const {
	if(node_stack.empty()) return 0;

	BSTreeNode<T> *next = 0;
	BSTreeNode<T> *top = node_stack.top();

	if(top->visit_count == 0) {
		if(top->left) top->left->visit_count = 0;
		if(top->right) top->right->visit_count = 0;
	}
	top->visit_count++;

	if(top->left && top->left->visit_count == 0) {
		next = top->left;
	} else if(top->right && top->right->visit_count == 0) {
		next = top->right;
	}

	if(next) {
		node_stack.push(next);
	} else {
		node_stack.pop();
	}

	if(top->visit_count == 1) {
		return &top->data;
	} else {
		return next();
	}
}
