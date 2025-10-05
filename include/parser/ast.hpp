#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "../schema/schema.hpp"

class astNode {
public:
    std::string nodeType; // e.g., "SELECT", "INSERT", "TABLE", "COLUMN"
    std::string value;    // e.g., table name, column name, etc.
    std::string tokenType; // e.g., "keyword", "identifier", "operator"
    std::vector<astNode*> children; // Child nodes
    astNode* parent = nullptr; // Pointer to parent node

    astNode(const std::string& type, const std::string& val) : nodeType(type), value(val) {}

    void addChild(astNode* child, std::string tokenType, bool setParent = true);

    void validateAST(astNode* node, const DatabaseSchema& schema);

    void print(astNode* node, int depth = 0);

    ~astNode();
};


