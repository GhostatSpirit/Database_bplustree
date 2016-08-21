#include "bplusnode.h"

// we must provide a definition of the destrcutor here to 
// stop the compiler from complaining
Node:: ~Node() {

}


/*--------------------InnerNode Definitions---------------------*/

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

void InnerNode::reserve() {
	// use reserve() to optimize memory cost and execution time
	keys.reserve(MAX_ORDER);
	int max_children_size = MAX_ORDER + 1;
	p_children.reserve(max_children_size);
}



/*--------------------LeafNode Definitions---------------------*/

LeafNode::LeafNode() {
	// use reserve() to optimize memory cost and execution time
	keys.reserve(MAX_ORDER);
	values.reserve(MAX_ORDER);
	// set the type to leaf
	type = LEAF;

	// IMPORTANT: set all invalid pointers to nullptr
	parent = nullptr;
	next = nullptr;
}


void LeafNode::reserve() {
	keys.reserve(MAX_ORDER);
	values.reserve(MAX_ORDER);
}

LeafNode::~LeafNode()
{
}
