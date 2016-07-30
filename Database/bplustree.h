//#pragma once
#ifndef _BPLUSTREE_H
#define _BPLUSTREE_H

// we need all kind of nodes:
// Node = base node
// InnerNode = interior node
// LeafNode = leaf node
#include "bplusnode.h"

using namespace std;

class BPlusTree {
public:


private:
	Node* root;				/* stores the root of this B+ tree, using Node* becasue
							 * we don't know whether it is a leaf node or a inner node
							 */
	
	int level;				/* stores the max level (height) of the tree*/

	// finds the leaf node that _should_ contain the entry with the specified key
	LeafNode* find_node(Node* node, const KEY& key);

	// find the next child node (closer to the key) in a given InnerNode
	Node* find_child_node(InnerNode* node, const KEY& key);
};



#endif
