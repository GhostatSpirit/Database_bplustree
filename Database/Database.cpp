// Database.cpp : Defines the entry point for the console application.
//

#include <iostream>

#include "bplusnode.h"

using namespace std;


int main()
{
	Node* p_node = new LeafNode();

	if (p_node->type == Node::LEAF) {
		cout << "true";
	}
	else {
		cout << "false"
	}

	system("pause");
}

