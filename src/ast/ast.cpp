#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "../../include/ast/ast.hpp"

void astNode::addChild(astNode* child) {
    std::cout << "Adding child node of type: " << child->nodeType << " with value: " << child->value << " to parent node of type: " << nodeType << " with value: " << value << std::endl;
    children.push_back(child);
}

astNode::~astNode() {
    for (auto child : children) {
        delete child;
    }
}

