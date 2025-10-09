#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "../../include/parser/ast.hpp"

void astNode::addChild(astNode* child, std::string tokenType, bool setParent) {
    if (setParent) {
        child->parent = this; // Automatically set the parent node
    }
    child->tokenType = tokenType; // Set the token type
    children.push_back(child);
}

astNode::~astNode() {
    for (auto child : children) {
        delete child;
    }
}

void astNode::validateAST(astNode* node, const DatabaseSchema& schema) {}


void astNode::print(astNode* node, int depth){
    if (!node) return;
    std::string indent(depth * 2, ' ');
    std::cout << indent << node->nodeType << ": " << node->value << std::endl;
    for (auto child : node->children) {
        print(child, depth + 1);
    }

}

