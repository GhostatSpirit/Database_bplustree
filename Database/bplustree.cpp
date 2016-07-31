#include "bplustree.h"
#include <iostream>
using namespace std;

VALUE * BPlusTree::find(const KEY & key)
{
	LeafNode* leaf = find_leaf_node(root, key);

	VALUE* p_value = find_in_leaf(leaf, key);

	return p_value;
}

void BPlusTree::insert(const KEY & key, const VALUE & value)
{
	LeafNode* target_leaf = find_leaf_node(root, key);
	VALUE* p_value = find_in_leaf(target_leaf, key);

	if (p_value != nullptr) {
		// if the key already exist, we can just override the value
		*p_value = value;
	}
	else if (!target_leaf->is_full()) {
		// if the target leaf is not full, we can just insert the {key, value} into the leaf node
		leaf_insert_nonfull(target_leaf, key, value);
	}
	else if (target_leaf->parent == nullptr) {
		// if the target leaf is currently root:
		// - create a new inner root and set the root pointer to it
		// - set the parent of leaf nodes
		// - increase the level of the tree
		// - push the new root to the back of vector<Node*> heads
		LeafNodeUnit unit = leaf_insert_full(target_leaf, key, value);

		// create the new root and set the root pointer
		InnerNode* new_root = new InnerNode();

		inner_insert_nonfull(new_root, unit);			// the parent of leaf nodes are set by this function
		
		set_root(new_root);								// set_root() has done the rest for us

	}
	else if (!target_leaf->parent->is_full()) {
		LeafNodeUnit unit = leaf_insert_full(target_leaf, key, value);
		// insert the leaf node unit to the parent
		InnerNode* current_node = target_leaf->parent;
		inner_insert_nonfull(current_node, unit);
	}
	else {
		// the leaf node is full and its parent node is full
		// let us first create the LeafNodeUnit
		LeafNodeUnit leaf_unit = leaf_insert_full(target_leaf, key, value);
		InnerNode* current_node = target_leaf->parent;
		// and construct the first InnerNodeUnit
		InnerNodeUnit inner_unit = inner_insert_full(current_node, leaf_unit);

		// loop this until the parent node is nullptr(we reached the root)
		// or the parent node if not full
		bool flag;
		if (current_node->parent == nullptr) {
			flag = false;
		}
		else if (current_node->parent->is_full() == false) {
			flag = false;
		}
		else {
			flag = true;
		}

		while (flag == true) {
			current_node = current_node->parent;
			inner_unit = inner_insert_full(current_node, inner_unit);

			if (current_node->parent == nullptr) {
				flag = false;
			}
			else if (current_node->parent->is_full() == false) {
				flag = false;
			}
			else {
				flag = true;
			}
		}

		// now the parent node of current_node is nullptr(we reached the root)
		// or the parent of current_node node is not full
		if (current_node->parent == nullptr) {
			// if we reach the root:
			// - create a new inner root and set the root pointer to it
			// - set the parent of child nodes
			// - increase the level of the tree
			// - push the new root to the back of vector<Node*> heads

			InnerNode* new_root = new InnerNode();
			inner_insert_nonfull(new_root, inner_unit);		// this function set the parent for us
			set_root(new_root);								// ser_root() does the rest
		}
		else {
			// if the parent of current_node node is not full
			current_node = current_node->parent;
			inner_insert_nonfull(current_node, inner_unit); // this function set the parent for us
		}
	}

}

// destory each nodes, from the deepest to the shallowest
BPlusTree::~BPlusTree()
{
	// destory each level, from the deepest to the shallowest
	for (unsigned i = 0; i < heads.size(); ++i) {
		Node* current_node = heads[i];
		Node* next_node;

		int counter = 1;
		while (current_node != nullptr) {
			// store the next node to be deleted first
			next_node = current_node->next_node();
			delete current_node;

			cout << counter << endl;
			counter++;

			current_node = next_node;
		}
	}
}

LeafNode * BPlusTree::find_leaf_node(Node* node, const KEY& key)
{
	if (node->type == Node::BASE) {
		throw runtime_error("find_node: cannot traverse BASE nodes");
		return nullptr;
	}

	// return the node it self if we found the leaf node
	if (node->type == Node::LEAF) {
		// using reinterpret_cast to cast a pointer
		LeafNode* tempLeaf = reinterpret_cast<LeafNode*>(node);
		return tempLeaf;
	}

	// else, the node is a inner node, so we can cast it
	InnerNode* p_inner_node = reinterpret_cast<InnerNode*>(node);
	// and find the next child node to be traversed
	Node* p_next_node = find_child_node(p_inner_node, key);
	// and find this next node
	find_leaf_node(p_next_node, key);
	
	// if anything goes wrong, return nullptr
	return nullptr;
}

Node * BPlusTree::find_child_node(InnerNode * node, const KEY & key)
{
	if (node->size() == 0) {
		throw runtime_error("find_child_node: InnerNode does not have any key");
		return nullptr;
	}

	// check if the vector size of the two vectors in the InnerNode fits
	if ((node->size() + 1) != (node->p_children.size())) {
		throw runtime_error("find_child_node: sizes of two vectors does not fit");
		return nullptr;
	}

	//TODO: optimize this algorithm
	unsigned int key_index = 0;
	unsigned int p_children_index = 0;

	while (key_index < node->size()) {
		if (key < node->keys[key_index]) {
			return node->p_children[p_children_index];
		}

		key_index++;
		p_children_index++;
	}
	return node->p_children[p_children_index];
}

VALUE * BPlusTree::find_in_leaf(LeafNode * leaf, const KEY & key)
{
	if (leaf->keys.size() == 0) {
		return nullptr;
	}


	// uses binary search
	int low = 0;
	int high = leaf->keys.size() - 1;
	while (low <= high)
	{
		int mid = low + (high - low) / 2;
		if (leaf->keys[mid] == key)
			return &(leaf->values[mid]);
		else if (leaf->keys[mid] > key)
			high = mid - 1;
		else
			low = mid + 1;
	}

	return nullptr;
}

unsigned BPlusTree::leaf_position_for(LeafNode* node, const KEY & key)
{
	// simple linear search, it is faster when N or M is < 100.
	// if returns 0, it means the key should be inserted at the beginning
	// if returns node.size(), it means the key should be inserted at the end
	unsigned key_index = 0;

	if (node->keys.size() == 0) {
		return key_index;
	}

	while (key_index < node->size()) {
		if (key < node->keys[key_index]) {
			// for debugging:
			if (key == node->keys[key_index]) {
				std::cerr << "Leaf_position_for: key value duplicated" << endl;
			}

			return key_index;
		}
		++key_index;
	}
	return key_index;
}


unsigned BPlusTree::inner_position_for(InnerNode * node, const KEY & key)
{
	
	// Simple linear search. Faster for small values of N or M
	unsigned k = 0;

	if (node->keys.size() == 0) {
		return k;
	}

	while ((k < node->size()) && ((node->keys[k]<key) || (node->keys[k] == key))) {
		++k;
	}
	return k;
}

void BPlusTree::set_root(InnerNode * new_root)
{
	if (new_root == this->root) {
		return;
	}
	// set the root pointer to the new root
	this->root = new_root;
	// increase the level of the tree
	this->level += 1;
	// push the new root to the vector
	heads.push_back(new_root);
}

void BPlusTree::leaf_insert_nonfull(LeafNode * node, const KEY & key, const VALUE & value)
{
	// check if this leaf node is not full
	if (node->is_full()) {
		throw runtime_error("leaf_insert_nonfull: leaf node already full");
	}
	// insert the key and the value
	unsigned index = leaf_position_for(node, key);

	node->keys.insert(node->keys.begin() + index, key);
	node->values.insert(node->values.begin() + index, value);

	return;
}

LeafNodeUnit BPlusTree::leaf_insert_full(LeafNode * node, const KEY & key, const VALUE & value)
{
	// check if this leaf node is full
	if (node->is_full() == false) {
		throw runtime_error("leaf_insert_full: leaf node is not full");
		LeafNodeUnit empty_unit;
		return empty_unit;
	}

	// The node was full. We must split it.
	unsigned treshold = (MAX_ORDER + 1) / 2;
	unsigned index_for_original = leaf_position_for(node, key);

	// construct a new sibling (**right** to the current LeafNode)
	LeafNode* new_sibling = new LeafNode();
	unsigned sibling_size = node->size() - treshold;

	// add the left half of the target LeafNode to the sibling node
	for (unsigned j = 0; j < sibling_size; ++j) {
		new_sibling->keys.push_back(node->keys[j + treshold]);
		new_sibling->values.push_back(node->values[j + treshold]);
	}
	// erase the right half the target LeafNode
	node->keys.erase(node->keys.begin() + treshold, node->keys.end());
	node->values.erase(node->values.begin() + treshold, node->values.end());

	// try to add the additional {key, value}
	if (index_for_original < treshold) {
		// Inserted element goes to left sibling
		leaf_insert_nonfull(node, key, value);
	}
	else {
		// Inserted element goes to right sibling
		leaf_insert_nonfull(new_sibling, key, value);
	}

	// set the "next" pointers
	new_sibling->next = node->next;
	node->next = new_sibling;


	// construct a LeafNodeUnit, the unit key is keys[0] in the right node
	LeafNodeUnit unit;
	unit.left = node;
	unit.right = new_sibling;
	// the unit key is keys[0] in the right node
	unit.key = new_sibling->keys[0];

	// return the LeafNodeUnit:
	return unit;
}

void BPlusTree::inner_insert_nonfull(InnerNode * node, LeafNodeUnit unit)
{
	// check if the node is not full
	if (node->is_full()) {
		throw runtime_error("inner_insert_nonfull: node is full");
		return;
	}
	// check if the p_children in the node contains leaf nodes (except p_children is empty)
	if (!(node->p_children.empty())) {
		if (node->p_children[0]->type != Node::LEAF) {
			throw runtime_error("inner_insert_nonfull: p_children not pointed to leaf nodes");
			return;
		}
	}

	KEY new_key = unit.key;
	unsigned index = inner_position_for(node, new_key);

	// first add the key to the inner node
	node->keys.insert(node->keys.begin() + index, new_key);
	// then we modify the children vector

	if (node->p_children.empty()) {
		node->p_children.push_back(unit.left);
		node->p_children.push_back(unit.right);
	}
	else {
		node->p_children.erase(node->p_children.begin() + index);
		array<Node*, 2> nodes = { unit.left, unit.right };
		node->p_children.insert(node->p_children.begin() + index, nodes.begin(), nodes.end());
	}

	// set the parent of the children nodes
	unit.left->parent = node;
	unit.right->parent = node;

	return;

}

void BPlusTree::inner_insert_nonfull(InnerNode * node, InnerNodeUnit unit)
{	
	// check if the node is not full
	if (node->is_full()) {
		throw runtime_error("inner_insert_nonfull: node is full");
		return;
	}
	// check if the p_children in the node contains Inner nodes (except p_children is empty)
	if (!(node->p_children.empty())) {
		if (node->p_children[0]->type != Node::INNER) {
			throw runtime_error("inner_insert_nonfull: p_children not pointed to leaf nodes");
			return;
		}
	}

	KEY new_key = unit.key;
	unsigned index = inner_position_for(node, new_key);

	// first add the key to the inner node
	node->keys.insert(node->keys.begin() + index, new_key);
	// then we modify the children vector
	node->p_children[index] = unit.left;
	node->p_children.insert(node->p_children.begin() + index + 1, unit.right);

	return;

}

InnerNodeUnit BPlusTree::inner_insert_full(InnerNode * node, LeafNodeUnit unit)
{
	// check if this inner node is full
	if (node->is_full() == false) {
		throw runtime_error("inner_insert_full: inner node is not full");
		InnerNodeUnit empty_unit;
		return empty_unit;
	}
	// check if this inner node's children are leaf nodes
	if (node->p_children[0]->type != Node::LEAF) {
		throw runtime_error("inner_insert_full: inner node not containing leaf nodes");
		InnerNodeUnit empty_unit;
		return empty_unit;
	}

	KEY new_key = unit.key;

	// create two vectors, vector<KEY> and vector<Node*>,
	// the size of the two vectors is larger than MAX_ORDER (by one)
	// they are only temporary
	vector<KEY> left_keys(node->keys);
	vector<Node*> left_pchildren(node->p_children);

	unsigned index = inner_position_for(node, new_key);
	// add the key to the temporary vector<KEY>
	left_keys.insert(left_keys.begin() + index, new_key);
	// add the two pointers to the temporary vector<Node*>
	left_pchildren[index] = unit.left;
	left_pchildren.insert(left_pchildren.begin() + index + 1, unit.right);


	unsigned threshold = (MAX_ORDER + 1) / 2;
	// save the middle key
	KEY middle_key = left_keys[threshold];

	// split the left node (create two vectors for the right node)
	vector<KEY> right_keys(left_keys.begin() + threshold + 1, left_keys.end());
	vector<Node*> right_pchildren(left_pchildren.begin() + threshold + 1, left_pchildren.end());
	// truncate the left node
	left_keys.erase(left_keys.begin() + threshold, left_keys.end());
	left_pchildren.erase(left_pchildren.begin() + threshold + 1, left_pchildren.end());

	// save the four vectors to two different nodes,
	// the original node is the left one
	node->keys = left_keys;
	node->p_children = left_pchildren;
	// use reserve to save memory
	node->reserve();
	// the new node is the right one
	InnerNode* right_sibling = new InnerNode(right_keys, right_pchildren);

	// construct the InnerNodeUnit struct, it will automatically set the next pointer
	InnerNodeUnit new_unit(middle_key, node, right_sibling);

	return new_unit;
}

InnerNodeUnit BPlusTree::inner_insert_full(InnerNode * node, InnerNodeUnit unit)
{
	// check if this inner node's children are inner nodes
	if (node->p_children[0]->type != Node::INNER) {
		throw runtime_error("inner_insert_full: inner node not containing inner nodes");
		InnerNodeUnit empty_unit;
		return empty_unit;
	}


	// check if this inner node is full
	if (node->is_full() == false) {
		throw runtime_error("inner_insert_full: inner node is not full");
		InnerNodeUnit empty_unit;
		return empty_unit;
	}

	KEY new_key = unit.key;

	// create two vectors, vector<KEY> and vector<Node*>,
	// the size of the two vectors is larger than MAX_ORDER (by one)
	// they are only temporary
	vector<KEY> left_keys(node->keys);
	vector<Node*> left_pchildren(node->p_children);

	unsigned index = inner_position_for(node, new_key);
	// add the key to the temporary vector<KEY>
	left_keys.insert(left_keys.begin() + index, new_key);
	// add the two pointers to the temporary vector<Node*>
	left_pchildren[index] = unit.left;
	left_pchildren.insert(left_pchildren.begin() + index + 1, unit.right);


	unsigned threshold = (MAX_ORDER + 1) / 2;
	// save the middle key
	KEY middle_key = left_keys[threshold];

	// split the left node (create two vectors for the right node)
	vector<KEY> right_keys(left_keys.begin() + threshold + 1, left_keys.end());
	vector<Node*> right_pchildren(left_pchildren.begin() + threshold + 1, left_pchildren.end());
	// truncate the left node
	left_keys.erase(left_keys.begin() + threshold, left_keys.end());
	left_pchildren.erase(left_pchildren.begin() + threshold + 1, left_pchildren.end());

	// save the four vectors to two different nodes,
	// the original node is the left one
	node->keys = left_keys;
	node->p_children = left_pchildren;
	// use reserve to save memory
	node->reserve();
	// the new node is the right one
	InnerNode* right_sibling = new InnerNode(right_keys, right_pchildren);

	// construct the InnerNodeUnit struct, it will automatically set the next pointer
	InnerNodeUnit new_unit(middle_key, node, right_sibling);

	return new_unit;
}

