#pragma once
#include <string>
#include <vector>
#include <unordered_map>

class astNode {
public:
    std::string nodeType; // e.g., "SELECT", "INSERT", "TABLE", "COLUMN"
    std::string value;    // e.g., table name, column name, etc.
    std::vector<astNode*> children; // Child nodes

    astNode(const std::string& type, const std::string& val) : nodeType(type), value(val) {}

    void addChild(astNode* child);

    void print(int depth = 0) const ;

    ~astNode();
};


