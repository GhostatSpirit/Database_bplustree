//#pragma once

#ifndef _BPLUSNODE_H
#define _BPLUSNODE_H

#include <string>
#include <vector>
#include <array>

using namespace std;

typedef unsigned long POS;		// type for saving the position

typedef unsigned long KEY;
typedef string VALUE;

// define the rank of this B+ tree
const int MAX_ORDER = 64;

// the base class of each B+ tree node
struct Node {
	enum NodeType {BASE, INNER, LAST_INNER, LEAF};

	// Node* parent;
	NodeType type;

	virtual Node* next_node() = 0;

	Node() {
		type = BASE;
	}

	// define a virtual destructor to let the compiler recognize
	// this is an abstract class
	virtual ~Node();

	virtual unsigned size() = 0;
};


class NodePos {
public:
	// constructors for NodePos
	NodePos(POS _filepos, Node* _pnode);
	NodePos(POS _filepos);

private:
	POS file_pos;
	Node* p_node;

	bool is_pointer_valid;			// bool for saving if the pointer is valid
};


struct InnerNode : public Node {
	~InnerNode();

	vector<KEY> keys;
	vector<Node*> p_children;

	InnerNode* parent;  
	InnerNode* next;	  // pointer to the next node is useful for traversing

	// default constructor
	InnerNode();

	// construct the inner using two existing vectors
	InnerNode(vector<KEY>& _keys, vector<Node*> & _pchildren);
	//// construct the inner node using LeafNodeUnit, set the parent of the child nodes automatically
	//InnerNode(LeafNodeUnit& unit);


	Node* next_node() { return next; }
	inline unsigned size() { return keys.size(); }
	bool is_last_inner();
	inline bool is_full() { return (keys.size() == MAX_ORDER); }

	// use reserve() to optimize memory cost and execution time
	void reserve();



};


struct LeafNode :public Node {
	LeafNode();
	~LeafNode();

	vector<KEY> keys;
	// TODO: change VALUE to the a number that stores the position in data file
	vector<VALUE> values;

	// pointer to the next node is useful for traversing
	LeafNode* next;
	InnerNode* parent;

	Node* next_node() { return next; }

	inline unsigned size() { return keys.size(); }

	inline bool is_full() { return (keys.size() == MAX_ORDER); }

	void reserve();


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


// a inner node unit is generated when a inner node is splited
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