//#pragma once

#ifndef _BPLUSNODE_H
#define _BPLUSNODE_H

#include <string>
#include <vector>
#include <array>

using namespace std;

typedef long KEY;
typedef string VALUE;

// define the rank of this B+ tree
const int MAX_ORDER = 4;

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
	array<KEY, MAX_ORDER> keys;
	array<Node*, MAX_ORDER + 1> p_children;

	// size = how many keys are being used
	// 0 <= size <= MAX_ORDER
	int fill_count;

	// pointer to the next node is useful for traversing
	InnerNode* next;

	bool is_last_inner = false;

	InnerNode() {
		// use reserve() to optimize memory cost and execution time

		// set the type to INNER
		type = INNER;
	}

	inline int size() {
		return fill_count;
	}

	inline bool is_full() {
		return (fill_count == keys.size());
	}

	~InnerNode(){}

};

struct LeafNode :public Node {
	array<KEY, MAX_ORDER> keys;
	// TODO: change VALUE to the a number that stores the position in data file
	array<VALUE, MAX_ORDER> values;

	// size = how many keys are being used
	// 0 <= size <= MAX_ORDER
	int fill_count;

	// pointer to the next node is useful for traversing
	LeafNode* next;

	LeafNode() {
		// set the type to leaf
		type = LEAF;
	}

	inline int size() {
		return fill_count;
	}

	inline bool is_full() {
		return (fill_count == keys.size());
	}

	~LeafNode() {}
};





#endif