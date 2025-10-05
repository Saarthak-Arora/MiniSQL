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

void astNode::validateAST(astNode* node, const DatabaseSchema& schema) {
    if (!node) return;

    if(node->nodeType == "ROOT") {
        if(node->children.empty()) {
            std::cerr << "Error: AST is empty." << std::endl;
            return;
        }
        else if(node->children[0]->nodeType != "QUERY") {
            std::cerr << "Error: Invalid root node type '" << node->children[0]->nodeType << "'." << std::endl;
            return;
        }

        for (astNode* child : node->children) {
            validateAST(child, schema);
        }

    } 

    if (node->nodeType == "TABLE") {
        // Validate table existence
        if (!schema.tableExists(node->value)) {
            std::cerr << "Error: Table '" << node->value << "' does not exist." << std::endl;
            return;
        }
    } else if (node->nodeType == "COLUMN") {
        // Validate column existence and constraints
        astNode* tableNode = node->parent; // Assuming parent node is the table
        if (tableNode && schema.tableExists(tableNode->value)) {
            const Table* table = schema.getTable(tableNode->value);
            std::string columnName = node->value.substr(0, node->value.find(' ')); // Extract column name
            if (table->columns.find(columnName) == table->columns.end()) {
                std::cerr << "Error: Column '" << columnName << "' does not exist in table '" << tableNode->value << "'." << std::endl;
            } else {
                const Column& column = table->columns.at(columnName);

                // Validate based on token type
                if ((column.datatype == "INT" && node->tokenType != "number") ||
                    (column.datatype == "VARCHAR" && node->tokenType != "string") ||
                    (column.datatype == "DOUBLE" && node->tokenType != "double")) {
                    std::cerr << "Error: Invalid value '" << node->value << "' for column '" << columnName
                              << "' of type " << column.datatype << ". Found token type: " << node->tokenType << "." << std::endl;
                }
            }
        }
    } else if (node->nodeType == "CONDITION") {
        // Validate conditions (e.g., age < "abc")
        astNode* left = nullptr;
        astNode* right = nullptr;
        std::string operatorValue;

        for (astNode* child : node->children) {
            if (child->nodeType == "LEFT") left = child;
            else if (child->nodeType == "RIGHT") right = child;
            else if (child->nodeType == "OPERATOR") operatorValue = child->value;
        }

        if (left && right) {
            std::string leftValue = left->value;
            std::string rightValue = right->value;

            // Check if the left side is a valid column
            if (leftValue.find("COLUMN(") == 0) {
                std::string columnName = leftValue.substr(7, leftValue.size() - 8); // Extract column name
                std::cout << "Validating condition on column: " << columnName <<"with value" << right->tokenType<< std::endl;
                astNode* tableNode = node->parent->parent; // Assuming parent is WHERE, and its parent is TABLE
                if (tableNode && schema.tableExists(tableNode->value)) {
                    const Table* table = schema.getTable(tableNode->value);
                    if (table->columns.find(columnName) == table->columns.end()) {
                        std::cerr << "Error: Column '" << columnName << "' does not exist in table '" << tableNode->value << "'." << std::endl;
                    } else {
                        // Check if the right side matches the column's datatype
                        const Column& column = table->columns.at(columnName);
                        std::cout << "Column '" << columnName << "' of type '" << column.datatype << "' found in table '" << tableNode->value << "'." << std::endl;
                         // Validate based on token type
                        if ((column.datatype == "INT" && right->tokenType != "number") ||
                            (column.datatype == "VARCHAR" && right->tokenType != "string") ||
                            (column.datatype == "DOUBLE" && right->tokenType != "double")) {
                            std::cerr << "Error: Invalid value '" << node->value << "' for column '" << columnName
                                    << "' of type " << column.datatype << ". Found token type: " << node->tokenType << "." << std::endl;
                        }

                        std::cout << "Condition on column '" << columnName << "' with operator '" << operatorValue
                                  << "' and value '" << right->tokenType << "' is valid." << std::endl;
                    }
                }
                else{
                        // parent is not WHERE, so we assume it's a direct child of query e.g UPDATE users SET age = 25 WHERE id = 'abc';
                        const Table* table = schema.getTable(tableNode->value);
                        if (table->columns.find(columnName) == table->columns.end()) {
                            std::cerr << "Error: Column '" << columnName << "' does not exist in table '" << tableNode->value << "'." << std::endl;
                        } else {
                            // Check if the right side matches the column's datatype
                            const Column& column = table->columns.at(columnName);
                            std::cout << "Column '" << columnName << "' of type '" << column.datatype << "' found in table '" << tableNode->value << "'." << std::endl;
                             if ((column.datatype == "INT" && right->tokenType != "number") ||
                            (column.datatype == "VARCHAR" && right->tokenType != "string") ||
                            (column.datatype == "DOUBLE" && right->tokenType != "double")) {
                            std::cerr << "Error: Invalid value '" << node->value << "' for column '" << columnName
                             << "' of type " << column.datatype << ". Found token type: " << node->tokenType << "." << std::endl;
                        }

                        std::cout << "Condition on column '" << columnName << "' with operator '" << operatorValue
                                    << "' and value '" << right->tokenType << "' is valid." << std::endl;
                    
                        
                        }

                }
            }
        }
    } else if (node->nodeType == "CONSTRAINT") {
        // Validate constraints (e.g., FOREIGN_KEY, UNIQUE)
        if (node->value == "FOREIGN_KEY") {
            bool hasReference = false;
            for (astNode* child : node->children) {
                if (child->nodeType == "REFERENCE") {
                    hasReference = true;
                    std::string reference = child->value;
                    size_t dotPos = reference.find('.');
                    if (dotPos == std::string::npos) {
                        std::cerr << "Error: Invalid FOREIGN_KEY reference '" << reference << "'." << std::endl;
                    } else {
                        std::string refTable = reference.substr(0, dotPos);
                        std::string refColumn = reference.substr(dotPos + 1);
                        if (!schema.tableExists(refTable) ||
                            schema.getTable(refTable)->columns.find(refColumn) == schema.getTable(refTable)->columns.end()) {
                            std::cerr << "Error: FOREIGN_KEY reference '" << reference << "' is invalid." << std::endl;
                        }
                    }
                }
            }
            if (!hasReference) {
                std::cerr << "Error: FOREIGN_KEY constraint is missing REFERENCES clause." << std::endl;
            }
        }
    }

    // Recursively validate child nodes
    for (astNode* child : node->children) {
        validateAST(child, schema);
    }
}


void astNode::print(astNode* node, int depth){
    if (!node) return;
    std::string indent(depth * 2, ' ');
    std::cout << indent << node->nodeType << ": " << node->value << std::endl;
    for (auto child : node->children) {
        print(child, depth + 1);
    }

}

