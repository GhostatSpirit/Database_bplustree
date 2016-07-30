//#pragma once

#ifndef _BPLUSNODE_H
#define _BPLUSNODE_H

#include <string>
#include <vector>

using namespace std;

typedef long KEY;
typedef string VALUE;

// define the rank of this B+ tree
const int TREE_RANK = 4;

// the base class of each B+ tree node
struct Node {
	enum NodeType {BASE, INNER, LAST_INNER, LEAF};

	Node* parent;
	NodeType type;
	Node() {
		parent = nullptr;
		type = BASE;
	}

	// define a virtual destructor to let the compiler recognize
	// this is an abstract class
	virtual ~Node() = 0;
};


struct InnerNode : public Node {
	vector<KEY> keys;
	vector<Node*> p_children;

	bool is_last_inner = false;

	InnerNode* next;


	InnerNode() {
		// use reserve() to optimize memory cost and execution time
		keys.reserve(TREE_RANK);
		int max_children_size = TREE_RANK + 1;
		p_children.reserve(max_children_size);

		// set the type to INNER
		type = INNER;
	}

	inline int size() {
		return keys.size();
	}

	inline bool is_full() {
		return (keys.size() == TREE_RANK);
	}

	~InnerNode(){}

};

struct LeafNode :public Node {
	vector<KEY> keys;
	// TODO: change VALUE to the a number that stores the position in data file
	vector<string> values;

	LeafNode() {
		// use reserve() to optimize memory cost and execution time
		keys.reserve(TREE_RANK);
		values.reserve(TREE_RANK);

		// set the type to leaf
		type = LEAF;
	}

	inline bool is_full() {
		return (keys.size() == TREE_RANK);
	}

	~LeafNode() {}
};





#endif