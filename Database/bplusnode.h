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

	// Node* parent;
	NodeType type;

	virtual Node* next_node() = 0;

	Node() {
		//parent = nullptr;
		type = BASE;
	}

	// define a virtual destructor to let the compiler recognize
	// this is an abstract class
	virtual ~Node();

	virtual unsigned size() = 0;
};


struct InnerNode : public Node {
	~InnerNode();

	vector<KEY> keys;
	vector<Node*> p_children;

	InnerNode* parent;

	// pointer to the next node is useful for traversing
	InnerNode* next;

	inline bool is_last_inner() {
		if (p_children.size() == 0) {
			return false;
		}

		if (p_children[0]->type == Node::LEAF) {
			return true;
		}
		else {
			return false;
		}
	}

	InnerNode() {
		parent = nullptr;
		next = nullptr;
		// use reserve() to optimize memory cost and execution time
		keys.reserve(MAX_ORDER);
		int max_children_size = MAX_ORDER + 1;
		p_children.reserve(max_children_size);
		// set the type to INNER
		type = INNER;
	}

	// construct the inner using two existing vectors
	InnerNode(vector<KEY>& _keys, vector<Node*> & _pchildren);
	//// construct the inner node using LeafNodeUnit, set the parent of the child nodes automatically
	//InnerNode(LeafNodeUnit& unit);

	Node* next_node() {
		return next;
	}

	inline unsigned size() {
		return keys.size();
	}

	inline bool is_full() {
		return (keys.size() == MAX_ORDER);
	}

	inline void reserve() {
		// use reserve() to optimize memory cost and execution time
		keys.reserve(MAX_ORDER);
		int max_children_size = MAX_ORDER + 1;
		p_children.reserve(max_children_size);
	}



};


struct LeafNode :public Node {
	~LeafNode();

	vector<KEY> keys;
	// TODO: change VALUE to the a number that stores the position in data file
	vector<VALUE> values;

	// pointer to the next node is useful for traversing
	LeafNode* next;

	InnerNode* parent;

	LeafNode() {
		// use reserve() to optimize memory cost and execution time
		keys.reserve(MAX_ORDER);
		values.reserve(MAX_ORDER);
		// set the type to leaf
		type = LEAF;

		// IMPORTANT: set all invalid pointers to nullptr
		parent = nullptr;
		next = nullptr;
	}


	Node* next_node() {
		return next;
	}

	inline unsigned size() {
		return keys.size();
	}

	inline bool is_full() {
		return (keys.size() == MAX_ORDER);
	}

	inline void reserve() {
		keys.reserve(MAX_ORDER);
		values.reserve(MAX_ORDER);
	}


};



// a leaf node unit is generated when a leaf node is splited
// | *left | KEY | *right |
//     |              |
//     v              v
// | LeafNode |  | LeafNode |
struct LeafNodeUnit {
	// constructor of the InnerNodeUnit, automatically set next pointer of the two innernodes
	LeafNodeUnit(const KEY& _key, LeafNode* _left, LeafNode* _right) {
		key = _key;
		left = _left;
		right = _right;

		// set the "next" pointers
		right->next = left->next;
		left->next = right;
	}

	// default constructor
	LeafNodeUnit() {}

	LeafNode* left = nullptr;
	KEY key;
	LeafNode* right = nullptr;
};


// a inner node unit is generated when a leaf node is splited
// | *left | KEY | *right |
//     |              |
//     v              v
// | InnerNode | | InnerNode |
struct InnerNodeUnit {
	// constructor of the InnerNodeUnit, automatically set next pointer of the two innernodes
	InnerNodeUnit(const KEY& _key, InnerNode* _left, InnerNode* _right) {
		key = _key;
		left = _left;
		right = _right;

		// set the "next" pointers
		right->next = left->next;
		left->next = right;
	}
	// default constructor
	InnerNodeUnit() {}

	InnerNode* left = nullptr;
	KEY key;
	InnerNode* right = nullptr;
};



#endif