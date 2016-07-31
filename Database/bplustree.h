//#pragma once
#ifndef _BPLUSTREE_H
#define _BPLUSTREE_H

// we need all kind of nodes:
// Node = base node
// InnerNode = interior node
// LeafNode = leaf node
#include "bplusnode.h"
#include <iostream>

using namespace std;

class BPlusTree {
public:

	/* ----------- public interface for the tree -------------*/
	
	// find the value, if found, returns the pointer to that value
	// else, return nullptr
	VALUE* find(const KEY& key);

	// Inserts a pair (key, value). If there is a previous pair with
	// the same key, the old value is overwritten with the new one.
	void insert(const KEY& key, const VALUE& value);

	// constructor of the tree
	BPlusTree() {
		root = new LeafNode();
		level = 0;
		heads.push_back(root);
	}

	// destructor of the tree, clear up memory
	~BPlusTree();


private:
	Node* root;				/* stores the root of this B+ tree, using Node* becasue
							 * we don't know whether it is a leaf node or a inner node
							 */
	
	int level;				/* stores the max level (height) of the tree*/

	vector<Node*> heads;	/* stores the head node (the most left node) of each level of the tree
							 * 0 is the deepest level (leafnodes)
							 * (make it easier to traverse and destroy the tree)
							 */

	/* ----------- private functions for finding -------------*/

	// finds the leaf node that _should_ contain the entry with the specified key
	LeafNode* find_leaf_node(Node* node, const KEY& key);
	// find the next child node which is closer to the leaf node containing the key
	Node* find_child_node(InnerNode* node, const KEY& key);
	// find the pointer to the value in a given leaf node,
	// if not found, return nullptr
	VALUE* find_in_leaf(LeafNode* leaf, const KEY& key);

	/* ----------- private functions for inserting -------------*/

	void leaf_insert_nonfull(LeafNode* node, const KEY& key, const VALUE& value);
	LeafNodeUnit leaf_insert_full(LeafNode* node, const KEY& key, const VALUE& value);

	void inner_insert_nonfull(InnerNode* node, LeafNodeUnit unit);
	void inner_insert_nonfull(InnerNode* node, InnerNodeUnit unit);
	InnerNodeUnit inner_insert_full(InnerNode* node, LeafNodeUnit unit);
	InnerNodeUnit inner_insert_full(InnerNode* node, InnerNodeUnit unit);

	// Returns the position where 'key' should be inserted in a leaf node
	// that has the given keys.
	// if returns 0, it means the key should be inserted at the beginning
	// if returns node.size(), it means the key should be inserted at the end
	unsigned leaf_position_for(LeafNode* node, const KEY& key);

	// Returns the position where 'key' should be inserted in an inner node
	// that has the given keys.
	unsigned inner_position_for(InnerNode* node, const KEY& key);

	// set the new root of the tree:
	// - set the root pointer to the new root
	// - increase the level of the tree
	// - push the new root to the back of vector<Node*> heads
	void set_root(InnerNode* new_root);
};


//void test() {
//	leaf_insert_nonfull(reinterpret_cast<LeafNode*>(root), 3, "See you");
//	leaf_insert_nonfull(reinterpret_cast<LeafNode*>(root), 1, "Hello");
//	leaf_insert_nonfull(reinterpret_cast<LeafNode*>(root), 4, "Goodbye");
//	leaf_insert_nonfull(reinterpret_cast<LeafNode*>(root), 2, "Hi");

//	try {
//		leaf_insert_nonfull(reinterpret_cast<LeafNode*>(root), 2, "Hi");
//	}
//	catch (exception& ex) {
//		cerr << ex.what() << endl;
//	}

//	LeafNode * leaf = reinterpret_cast<LeafNode*>(root);
//	for (unsigned i = 0; i < leaf->size(); ++i) {
//		cout << leaf->keys[i] << '\t' << leaf->values[i] << endl;
//	}
//}



#endif
