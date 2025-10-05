#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "../../include/parser/parser.hpp"

// Helper function to check if a token matches a specific type and value
bool parser::isToken(const std::pair<std::string, std::string>& token, const std::string& type, const std::string& value = "") {
    return token.first == type && (value.empty() || token.second == value);
}

// Recursive subquery handler
bool parser::handleSubquery(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure& parentQuery, astNode* parentNode) {
    if (i < tokens.size() && tokens[i].second == "(" && i + 1 < tokens.size() && isToken(tokens[i + 1], "keyword", "SELECT")) {
        i++; // Skip '('
        queryStructure subQuery;
        std::cout << "Parsing subquery..." << std::endl;
        parse(tokens, i, subQuery, parentNode); // Recursive parse
        parentQuery.subqueries.push_back(subQuery);

        // Skip until the closing parenthesis
        while (i < tokens.size() && tokens[i].second != ")") i++;
        if (i < tokens.size() && tokens[i].second == ")") i++; // Skip ')'
        return true;
    }
    return false;
}

// ---------------- SELECT ----------------
void parser::parseSelect(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure& storedQuery, astNode* parentNode) {
    std::cout << "Parsing SELECT query..." << std::endl;

    // Add a COLUMNS node to the AST
    astNode* columnsNode = new astNode("COLUMNS", "");
    parentNode->addChild(columnsNode, "keyword"); // "keyword" as the type for the COLUMNS node

    // Parse columns
    while (i < tokens.size() && !isToken(tokens[i], "keyword", "FROM")) {
        if (tokens[i].first == "identifier" && i + 2 < tokens.size() &&
            tokens[i + 1].second == "." && tokens[i + 2].first == "identifier") {
            // Handle dot notation (e.g., u.name)
            std::string aliasOrTable = tokens[i].second;
            std::string columnName = tokens[i + 2].second;
            std::cout << "Alias/Table: " << aliasOrTable << ", Column: " << columnName << std::endl;

            // Add to AST or query structure
            astNode* columnNode = new astNode("COLUMN", aliasOrTable + "." + columnName);
            columnsNode->addChild(columnNode, tokens[i + 2].first); // Pass the token type of the column (e.g., "identifier")
            storedQuery.columns.push_back(aliasOrTable + "." + columnName);

            // Advance the index
            i += 3;
        }else if(tokens[i].first == "identifier") {
            std::string columnName = tokens[i].second;
            std::cout << "Column: " << columnName << std::endl;

            // Add to AST or query structure
            astNode* columnNode = new astNode("COLUMN", columnName);
            columnsNode->addChild(columnNode, tokens[i].first); // Pass the token type of the column (e.g., "identifier")
            storedQuery.columns.push_back(columnName);

            i++;
        } else if (tokens[i].second == ",") {
            i++; // Skip comma
        } 
        else {
            std::cerr << "Error: Invalid column format in SELECT query." << std::endl;
            return;
        }
    }

    // Add a FROM node to the AST
    if (i < tokens.size() && isToken(tokens[i], "keyword", "FROM")) {
        i++; // Skip "FROM"
        astNode* fromNode = new astNode("FROM", "");
        parentNode->addChild(fromNode, "keyword"); // "keyword" as the type for the FROM node

        // Parse table
        if (i < tokens.size() && tokens[i].first == "identifier") {
            std::string tableName = tokens[i].second;
            astNode* tableNode = new astNode("TABLE", tableName);
            fromNode->addChild(tableNode, tokens[i].first); // Pass the token type of the table (e.g., "identifier")
            std::cout << "Table: " << tableName << std::endl;
            i++;
        } else {
            std::cerr << "Error: Missing table name in SELECT query." << std::endl;
            return;
        }
    } else {
        std::cerr << "Error: Missing FROM clause in SELECT query." << std::endl;
        return;
    }

    // Add a WHERE node to the AST (if applicable)
    if (i < tokens.size() && isToken(tokens[i], "keyword", "WHERE")) {
        i++; // Skip "WHERE"
        astNode* whereNode = new astNode("WHERE", "");
        parentNode->addChild(whereNode, "keyword"); // "keyword" as the type for the WHERE node

        // Parse conditions
        parseCondition(tokens, i, storedQuery, whereNode);
    }
}

// ---------------- INSERT ----------------
void parser::parseInsert(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure& storedQuery, astNode* parentNode) {
    std::cout << "Parsing INSERT query..." << std::endl;

    if (i + 1 < tokens.size() && isToken(tokens[i], "keyword", "INTO") && tokens[i + 1].first == "identifier") {
        i++;
        std::string tableName = tokens[i].second;
        storedQuery.tableName = tableName;
        astNode* tableNode = new astNode("TABLE", tableName);
        parentNode->addChild(tableNode, "identifier"); // Provide the missing argument(s)
        std::cout << "Table Name: " << tableName << std::endl;
        i++;

        // Parse column list
        if (i < tokens.size() && tokens[i].second == "(") {
            i++; // Skip '('
            astNode* columnsNode = new astNode("COLUMNS", "");
            tableNode->addChild(columnsNode, "keyword"); // Provide the missing argument(s)
            while (i < tokens.size() && tokens[i].second != ")") {
                if (tokens[i].first == "identifier") {
                    columnsNode->addChild(new astNode("COLUMN", tokens[i].second), "identifier");
                    storedQuery.columns.push_back(tokens[i].second);
                    std::cout << "Column: " << tokens[i].second << std::endl;
                }
                i++;
            }
            if (i < tokens.size() && tokens[i].second == ")") i++; // Skip ')'
        }

        // Parse VALUES
        if (i < tokens.size() && isToken(tokens[i], "keyword", "VALUES")) {
            i++;
            if (i < tokens.size() && tokens[i].second == "(") {
                i++; // Skip '('
                astNode* valuesNode = new astNode("VALUES", "");
                tableNode->addChild(valuesNode, "keyword"); // Provide the missing argument(s)
                while (i < tokens.size() && tokens[i].second != ")") {
                    if (tokens[i].first == "string" || tokens[i].first == "number" || tokens[i].first == "double") {
                        valuesNode->addChild(new astNode("VALUE", tokens[i].second), tokens[i].first); // Pass token type
                        storedQuery.values.push_back(tokens[i].second);
                        std::cout << "Value: " << tokens[i].second << std::endl;
                        i++;
                    }
                }
                if (i < tokens.size() && tokens[i].second == ")") i++; // Skip ')'
            }
        }
    } else {
        std::cerr << "Error: Expected INTO keyword and table name in INSERT query." << std::endl;
    }
}

// ---------------- UPDATE ----------------
void parser::parseUpdate(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure& storedQuery, astNode* parentNode) {
    std::cout << "Parsing UPDATE query..." << std::endl;

    // Parse table name
    if (i < tokens.size() && tokens[i].first == "identifier") {
        std::string tableName = tokens[i].second;
        storedQuery.tableName = tableName;
        astNode* tableNode = new astNode("TABLE", tableName);
        parentNode->addChild(tableNode, tokens[i].first); // Pass token type
        std::cout << "Table Name: " << tableName << std::endl;
        i++;
    } else {
        std::cerr << "Error: Expected table name in UPDATE query." << std::endl;
        return;
    }

    // Parse SET clause
    if (i < tokens.size() && isToken(tokens[i], "keyword", "SET")) {
        i++; // Skip "SET"
        astNode* setNode = new astNode("SET", "");
        parentNode->addChild(setNode, "keyword"); // "keyword" as the type for the SET node

        // Parse assignments
        while (i < tokens.size() && !isToken(tokens[i], "keyword", "WHERE")) {
            if (tokens[i].first == "identifier" && i + 1 < tokens.size() && tokens[i + 1].second == "=") {
                // Parse column (LEFT)
                std::string columnName = tokens[i].second;
                astNode* leftNode = new astNode("LEFT", "COLUMN(" + columnName + ")");
                setNode->addChild(leftNode, tokens[i].first); // Pass token type for column
                i++; // Skip column name

                // Parse operator (OPERATOR)
                if (tokens[i].second == "=") {
                    astNode* operatorNode = new astNode("OPERATOR", "=");
                    setNode->addChild(operatorNode, "operator"); // "operator" as the type
                    i++; // Skip "="
                } else {
                    std::cerr << "Error: Expected '=' in SET clause." << std::endl;
                    return;
                }

                // Parse value (RIGHT)
                if (i < tokens.size() && (tokens[i].first == "number" || tokens[i].first == "string")) {
                    std::string value = tokens[i].second;
                    astNode* rightNode = new astNode("RIGHT", "VALUE(" + value + ")");
                    setNode->addChild(rightNode, tokens[i].first); // Pass token type for value
                    storedQuery.values.push_back(value);
                    i++; // Skip value
                } else {
                    std::cerr << "Error: Expected value in SET clause." << std::endl;
                    return;
                }

                // Handle comma (if multiple assignments)
                if (i < tokens.size() && tokens[i].second == ",") {
                    i++; // Skip ","
                }
            } else {
                std::cerr << "Error: Invalid assignment in SET clause." << std::endl;
                return;
            }
        }
    } else {
        std::cerr << "Error: Missing SET clause in UPDATE query." << std::endl;
        return;
    }

    // Parse WHERE clause
    if (i < tokens.size() && isToken(tokens[i], "keyword", "WHERE")) {
        i++; // Skip "WHERE"
        astNode* whereNode = new astNode("WHERE", "");
        parentNode->addChild(whereNode, "keyword"); // "keyword" as the type for the WHERE node

        // Parse the condition
        parseCondition(tokens, i, storedQuery, whereNode);
    }
}

// ---------------- DELETE ----------------
void parser::parseDelete(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure& storedQuery, astNode* parentNode) {
    std::cout << "Parsing DELETE query..." << std::endl;

    if (i < tokens.size() && isToken(tokens[i], "keyword", "FROM")) {
        i++;
        if (i < tokens.size() && tokens[i].first == "identifier") {
            std::string tableName = tokens[i].second;
            storedQuery.tableName = tableName;
            astNode* tableNode = new astNode("TABLE", tableName);
            parentNode->addChild(tableNode, "identifier"); // Provide the missing argument(s)
            std::cout << "Table Name: " << tableName << std::endl;
            i++;
        } else {
            std::cerr << "Error: Expected table name after FROM." << std::endl;
        }
    }

    // Parse WHERE clause
    if (i < tokens.size() && isToken(tokens[i], "keyword", "WHERE")) {
        i++;
        astNode* whereNode = new astNode("WHERE", "");
        parentNode->addChild(whereNode, "keyword"); // "keyword" as the type for the WHERE node

        // Parse the condition
        parseCondition(tokens, i, storedQuery, whereNode);
    }
}

// ---------------- CREATE ----------------
void parser::parseCreate(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure& storedQuery, astNode* parentNode) {
    std::cout << "Parsing CREATE TABLE query..." << std::endl;

    if (i + 1 < tokens.size() && isToken(tokens[i], "keyword", "TABLE")) {
        i++;
        if (i < tokens.size() && tokens[i].first == "identifier") {
            std::string tableName = tokens[i].second;
            storedQuery.tableName = tableName;
            astNode* tableNode = new astNode("TABLE", tableName);
            parentNode->addChild(tableNode, "identifier"); // Provide the missing argument(s)
            std::cout << "Table Name: " << tableName << std::endl;
            i++;

            // Parse column definitions
            if (i < tokens.size() && tokens[i].second == "(") {
                i++; // Skip '('
                while (i < tokens.size() && tokens[i].second != ")") {
                    if (tokens[i].first == "identifier") {
                        std::string columnName = tokens[i].second;
                        i++;
                        if (i < tokens.size() && tokens[i].first == "datatype") {
                            std::string columnType = tokens[i].second;
                            astNode* columnNode = new astNode("COLUMN", columnName + " " + columnType);
                            tableNode->addChild(columnNode, "identifier"); // Provide the missing argument(s)
                            std::cout << "Adding child node of type: COLUMN with value: " << columnName + " " + columnType
                                      << " to parent node of type: TABLE with value: " << tableNode->value << std::endl;
                            std::cout << "Column: " << columnName << " Type: " << columnType << std::endl;
                            i++;

                            // Parse constraints
                            while (i < tokens.size() && tokens[i].first == "keyword") {
                                std::string constraint = tokens[i].second;

                                if (constraint == "FOREIGN_KEY") {
                                    astNode* constraintNode = new astNode("CONSTRAINT", "FOREIGN_KEY");
                                    columnNode->addChild(constraintNode, "constraint"); // Provide the missing argument(s)
                                    std::cout << "Adding child node of type: CONSTRAINT with value: FOREIGN_KEY to parent node of type: COLUMN with value: " << columnNode->value << std::endl;
                                    i++;

                                    // Check for reference table and column
                                    if (i < tokens.size() && tokens[i].second == "REFERENCES") {
                                        i++; // Skip "REFERENCES"
                                        if (i < tokens.size() && tokens[i].first == "identifier") {
                                            std::string referenceTable = tokens[i].second;
                                            i++;
                                            if (i < tokens.size() && tokens[i].second == "(") {
                                                i++; // Skip '('
                                                if (i < tokens.size() && tokens[i].first == "identifier") {
                                                    std::string referenceColumn = tokens[i].second;
                                                    i++;
                                                    if (i < tokens.size() && tokens[i].second == ")") {
                                                        i++; // Skip ')'
                                                        constraintNode->addChild(new astNode("REFERENCE", referenceTable + "." + referenceColumn), "reference");
                                                        std::cout << "Adding child node of type: REFERENCE with value: " << referenceTable + "." + referenceColumn << " to parent node of type: CONSTRAINT with value: FOREIGN_KEY" << std::endl;
                                                    } else {
                                                        std::cerr << "Error: Missing closing parenthesis for FOREIGN_KEY reference." << std::endl;
                                                    }
                                                } else {
                                                    std::cerr << "Error: Missing column name in FOREIGN_KEY reference." << std::endl;
                                                }
                                            } else {
                                                std::cerr << "Error: Missing opening parenthesis for FOREIGN_KEY reference." << std::endl;
                                            }
                                        } else {
                                            std::cerr << "Error: Missing table name in FOREIGN_KEY reference." << std::endl;
                                        }
                                    } else {
                                        std::cerr << "Error: FOREIGN_KEY constraint must include REFERENCES clause." << std::endl;
                                    }
                                } else if (constraint == "PRIMARY_KEY" || constraint == "NOT_NULL") {
                                    columnNode->addChild(new astNode("CONSTRAINT", constraint), "constraint");
                                    std::cout << "Adding child node of type: CONSTRAINT with value: " << constraint << " to parent node of type: COLUMN with value: " << columnNode->value << std::endl;
                                    i++;
                                } else {
                                    break; // Stop parsing constraints if an unrecognized keyword is encountered
                                }
                            }
                        } else {
                            std::cerr << "Error: Expected datatype for column " << columnName << std::endl;
                            return;
                        }
                    }
                    if (i < tokens.size() && tokens[i].second == ",") {
                        i++; // Skip comma
                    }
                }
                if (i < tokens.size() && tokens[i].second == ")") {
                    i++; // Skip ')'
                } else {
                    std::cerr << "Error: Missing closing parenthesis in column definitions." << std::endl;
                }
            } else {
                std::cerr << "Error: Expected '(' after table name." << std::endl;
            }
        } else {
            std::cerr << "Error: Expected table name after CREATE TABLE." << std::endl;
        }
    } else {
        std::cerr << "Error: Expected TABLE keyword after CREATE." << std::endl;
    }
}

// ---------------- Condition Parsing ----------------
void parser::parseCondition(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure& storedQuery, astNode* parentNode) {
    astNode* conditionNode = new astNode("CONDITION", "");
    parentNode->addChild(conditionNode, "condition"); // Provide the missing argument(s)

    // Parse the left-hand side of the condition
    if (i < tokens.size() && tokens[i].first == "identifier") {
        if (i + 2 < tokens.size() && tokens[i + 1].second == "." && tokens[i + 2].first == "identifier") {
            // Handle dot notation (e.g., o.user_id)
            std::string aliasOrTable = tokens[i].second;
            std::string columnName = tokens[i + 2].second;
            conditionNode->addChild(new astNode("LEFT", "COLUMN(" + aliasOrTable + "." + columnName + ")"), "identifier");
            i += 3;
        } else {
            // Handle standalone identifiers
            conditionNode->addChild(new astNode("LEFT", "COLUMN(" + tokens[i].second + ")"), "identifier");
            i++;
        }
    }

    // Parse the operator
    if (i < tokens.size() && (tokens[i].first == "operator" || isToken(tokens[i], "keyword", "IN") || isToken(tokens[i], "keyword", "LIKE") || isToken(tokens[i], "keyword", "BETWEEN"))) {
        conditionNode->addChild(new astNode("OPERATOR", tokens[i].second), "operator");
        i++;
    }

    // Parse the right-hand side of the condition
    if (i < tokens.size()) {
        if (tokens[i].second == "(") {
            // Handle subquery on the right-hand side
            astNode* rightNode = new astNode("RIGHT", "QUERY");
            conditionNode->addChild(rightNode, "query"); // Specify the type or relationship of the child node
            handleSubquery(tokens, i, storedQuery, rightNode);
        } else if (tokens[i].first == "number" || tokens[i].first == "string" || tokens[i].first == "double") {
            conditionNode->addChild(new astNode("RIGHT", "VALUE(" + tokens[i].second + ")"), tokens[i].first); // Pass token type
            i++;
        } else if (tokens[i].first == "identifier") {
            if (i + 2 < tokens.size() && tokens[i + 1].second == "." && tokens[i + 2].first == "identifier") {
                // Handle dot notation (e.g., u.id)
                std::string aliasOrTable = tokens[i].second;
                std::string columnName = tokens[i + 2].second;
                conditionNode->addChild(new astNode("RIGHT", "COLUMN(" + aliasOrTable + "." + columnName + ")"), "identifier");
                i += 3;
            } else {
                // Handle standalone identifiers
                conditionNode->addChild(new astNode("RIGHT", "COLUMN(" + tokens[i].second + ")"), "identifier");
                i++;
            }
        }
    }
}

void parser::parse(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure& storedQuery, astNode* parentNode) {
    std::cout << "Starting parsing process..." << std::endl;

    // Define the function map
    std::unordered_map<std::string, std::function<void(const std::vector<std::pair<std::string, std::string>>&, size_t&, queryStructure&, astNode*)>> functionMap = {
        {"SELECT", std::bind(&parser::parseSelect, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)},
        {"INSERT", std::bind(&parser::parseInsert, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)},
        {"UPDATE", std::bind(&parser::parseUpdate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)},
        {"DELETE", std::bind(&parser::parseDelete, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)},
        {"CREATE", std::bind(&parser::parseCreate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)}
    };

    while (i < tokens.size()) {
        const auto& token = tokens[i];

        if (token.first == "keyword") {
            storedQuery.commandType = token.second;
            std::cout << "Command Type: " << token.second << std::endl;

            astNode* queryNode = new astNode("QUERY", token.second);
            parentNode->addChild(queryNode, "query"); // Provide the missing argument(s)

            auto it = functionMap.find(token.second);
            if (it != functionMap.end()) {
                i++;
                it->second(tokens, i, storedQuery, queryNode); // Call the appropriate function
            } else {
                std::cerr << "Error: Unsupported command type: " << token.second << std::endl;
            }
        }
        i++;
    }


    std::cout << "Parsing completed." << std::endl;
}