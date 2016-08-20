#include "bplusnode.h"

// we must provide a definition of the destrcutor here to 
// stop the compiler from complaining
Node:: ~Node() {

}

InnerNode::~InnerNode()
{
}

inline bool InnerNode::is_last_inner() {
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

InnerNode::InnerNode() {
	parent = nullptr;
	next = nullptr;
	// use reserve() to optimize memory cost and execution time
	keys.reserve(MAX_ORDER);
	int max_children_size = MAX_ORDER + 1;
	p_children.reserve(max_children_size);
	// set the type to INNER
	type = INNER;
}

InnerNode::InnerNode(vector<KEY>& _keys, vector<Node*>& _pchildren) {
	// IMPORTANT: set all invalid pointers to nullptr
	parent = nullptr;
	next = nullptr;

	keys = _keys;
	p_children = _pchildren;

	// check if the size of the two vectors exceeds the order
	if (_keys.size() > MAX_ORDER || _pchildren.size() > MAX_ORDER + 1) {
		throw runtime_error("InnerNode constructor: size exceeds order");
	}

	// use reserve() to optimize memory cost and execution time
	keys.reserve(MAX_ORDER);
	int max_children_size = MAX_ORDER + 1;
	p_children.reserve(max_children_size);
	// set the type to INNER
	type = INNER;
}

inline void InnerNode::reserve() {
	// use reserve() to optimize memory cost and execution time
	keys.reserve(MAX_ORDER);
	int max_children_size = MAX_ORDER + 1;
	p_children.reserve(max_children_size);
}

//InnerNode::InnerNode(LeafNodeUnit & unit)
//{
//	parent = nullptr;
//
//	// set the type to INNER
//	type = INNER;
//	// set the parent of the child nodes
//	unit.left->parent = this;
//	unit.right->parent = this;
//
//	// add values to two vectors
//	keys.push_back(unit.key);
//	p_children.push_back(unit.left);
//	p_children.push_back(unit.right);
//
//	// use reserve() to optimize memory cost and execution time
//	keys.reserve(MAX_ORDER);
//	int max_children_size = MAX_ORDER + 1;
//	p_children.reserve(max_children_size);
//
//}

LeafNode::~LeafNode()
{
}
