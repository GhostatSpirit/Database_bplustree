#include "bplustree.h"
using namespace std;

LeafNode * BPlusTree::find_node(Node* node, const KEY& key)
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
	find_node(p_next_node, key);
	
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

	while (key_index <= node->size()) {
		if (key < node->keys[key_index]) {
			return node->p_children[p_children_index];
		}

		key_index++;
		p_children_index++;
	}
	return node->p_children[p_children_index];
}
