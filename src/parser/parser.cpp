#include <iostream>
#include <string>
#include <vector>
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
    parentNode->addChild(columnsNode);

    // Parse columns
    while (i < tokens.size() && !isToken(tokens[i], "keyword", "FROM")) {
        if (tokens[i].first == "identifier" && i + 2 < tokens.size() &&
            tokens[i + 1].second == "." && tokens[i + 2].first == "identifier") {
            // Handle dot notation (e.g., u.name)
            std::string aliasOrTable = tokens[i].second;
            std::string columnName = tokens[i + 2].second;
            std::cout << "Alias/Table: " << aliasOrTable << ", Column: " << columnName << std::endl;

            // Add to AST or query structure
            columnsNode->addChild(new astNode("COLUMN", aliasOrTable + "." + columnName));
            storedQuery.columns.push_back(aliasOrTable + "." + columnName);

            // Advance the index
            i += 3;
        } else if (tokens[i].first == "identifier" || tokens[i].second == "*") {
            // Handle standalone columns or wildcard (*)
            storedQuery.columns.push_back(tokens[i].second);
            columnsNode->addChild(new astNode("COLUMN", tokens[i].second));
            std::cout << "Column: " << tokens[i].second << std::endl;
            i++;
        } else {
            i++;
        }
    }

    // Parse FROM clause
    if (i < tokens.size() && isToken(tokens[i], "keyword", "FROM")) {
        i++;
        astNode* fromNode = new astNode("FROM", "");
        parentNode->addChild(fromNode);

        // Parse table and alias
        if (i < tokens.size() && tokens[i].first == "identifier") {
            std::string tableName = tokens[i].second;
            i++;

            std::string alias = "";
            if (i < tokens.size() && tokens[i].first == "identifier") {
                alias = tokens[i].second;
                i++;
            }

            // Add table and alias to AST
            if (!alias.empty()) {
                fromNode->addChild(new astNode("TABLE", tableName + " AS " + alias));
                storedQuery.tableName = tableName + " AS " + alias;
                std::cout << "Table: " << tableName << ", Alias: " << alias << std::endl;
            } else {
                fromNode->addChild(new astNode("TABLE", tableName));
                storedQuery.tableName = tableName;
                std::cout << "Table: " << tableName << std::endl;
            }
        } else {
            std::cerr << "Error: Expected table name after FROM." << std::endl;
        }

        // Parse JOIN clauses
        while (i < tokens.size() && (isToken(tokens[i], "keyword", "INNER") || isToken(tokens[i], "keyword", "LEFT") ||
                                     isToken(tokens[i], "keyword", "RIGHT") || isToken(tokens[i], "keyword", "FULL") ||
                                     isToken(tokens[i], "keyword", "JOIN"))) {
            std::string joinType = tokens[i].second;
            if (joinType == "INNER" || joinType == "LEFT" || joinType == "RIGHT" || joinType == "FULL") {
                i++; // Skip join type
                if (i < tokens.size() && isToken(tokens[i], "keyword", "JOIN")) {
                    joinType += " JOIN";
                    i++; // Skip JOIN keyword
                }
            } else if (joinType == "JOIN") {
                i++; // Skip JOIN keyword
            }

            if (i < tokens.size() && tokens[i].first == "identifier") {
                std::string joinTable = tokens[i].second;
                i++;

                std::string joinAlias = "";
                if (i < tokens.size() && tokens[i].first == "identifier") {
                    joinAlias = tokens[i].second;
                    i++;
                }

                // Add join table and alias to AST
                astNode* joinNode = new astNode("JOIN", joinType);
                fromNode->addChild(joinNode);
                if (!joinAlias.empty()) {
                    joinNode->addChild(new astNode("TABLE", joinTable + " AS " + joinAlias));
                    std::cout << "Join Table: " << joinTable << ", Alias: " << joinAlias << ", Type: " << joinType << std::endl;
                } else {
                    joinNode->addChild(new astNode("TABLE", joinTable));
                    std::cout << "Join Table: " << joinTable << ", Type: " << joinType << std::endl;
                }

                // Parse ON clause
                if (i < tokens.size() && isToken(tokens[i], "keyword", "ON")) {
                    i++;
                    astNode* onNode = new astNode("ON", "");
                    joinNode->addChild(onNode);

                    // Parse join condition
                    parseCondition(tokens, i, storedQuery, onNode);
                } else {
                    std::cerr << "Error: Expected ON clause after JOIN." << std::endl;
                }
            } else {
                std::cerr << "Error: Expected table name after JOIN." << std::endl;
            }
        }
    } else {
        std::cerr << "Error: Missing FROM clause in SELECT query." << std::endl;
    }

    // Parse WHERE clause
    if (i < tokens.size() && isToken(tokens[i], "keyword", "WHERE")) {
        i++;
        astNode* whereNode = new astNode("WHERE", "");
        parentNode->addChild(whereNode);

        // Parse the condition
        parseCondition(tokens, i, storedQuery, whereNode);
    }

    // Parse additional clauses (e.g., GROUP BY, ORDER BY)
    while (i < tokens.size()) {
        if (isToken(tokens[i], "keyword", "GROUP")) {
            i++;
            if (i < tokens.size() && isToken(tokens[i], "keyword", "BY")) {
                i++;
                while (i < tokens.size() && tokens[i].first != "keyword") {
                    storedQuery.groupBy.push_back(tokens[i].second);
                    std::cout << "GROUP BY: " << tokens[i].second << std::endl;
                    i++;
                }
            }
        } else if (isToken(tokens[i], "keyword", "ORDER")) {
            i++;
            if (i < tokens.size() && isToken(tokens[i], "keyword", "BY")) {
                i++;
                while (i < tokens.size() && tokens[i].first != "keyword") {
                    storedQuery.orderBy.push_back(tokens[i].second);
                    std::cout << "ORDER BY: " << tokens[i].second << std::endl;
                    i++;
                }
            }
        } else {
            break; // Stop parsing if no additional clauses are found
        }
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
        parentNode->addChild(tableNode);
        std::cout << "Table Name: " << tableName << std::endl;
        i++;

        // Parse column list
        if (i < tokens.size() && tokens[i].second == "(") {
            i++; // Skip '('
            astNode* columnsNode = new astNode("COLUMNS", "");
            tableNode->addChild(columnsNode);
            while (i < tokens.size() && tokens[i].second != ")") {
                if (tokens[i].first == "identifier") {
                    columnsNode->addChild(new astNode("COLUMN", tokens[i].second));
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
                tableNode->addChild(valuesNode);
                while (i < tokens.size() && tokens[i].second != ")") {
                    if (tokens[i].first == "string" || tokens[i].first == "number") {
                        valuesNode->addChild(new astNode("VALUE", tokens[i].second));
                        storedQuery.values.push_back(tokens[i].second);
                        std::cout << "Value: " << tokens[i].second << std::endl;
                    }
                    i++;
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

    if (i < tokens.size() && tokens[i].first == "identifier") {
        std::string tableName = tokens[i].second;
        storedQuery.tableName = tableName;
        astNode* tableNode = new astNode("TABLE", tableName);
        parentNode->addChild(tableNode);
        std::cout << "Table Name: " << tableName << std::endl;
        i++;
    }

    if (i < tokens.size() && isToken(tokens[i], "keyword", "SET")) {
        i++;
        astNode* setNode = new astNode("SET", "");
        parentNode->addChild(setNode);
        while (i < tokens.size() && !isToken(tokens[i], "keyword", "WHERE")) {
            if (tokens[i].first == "identifier" && i + 2 < tokens.size() && tokens[i + 1].second == "=") {
                std::string columnName = tokens[i].second;
                std::string value = tokens[i + 2].second;
                setNode->addChild(new astNode("ASSIGN", columnName + " = " + value));
                storedQuery.columns.push_back(columnName);
                storedQuery.values.push_back(value);
                std::cout << "Set: " << columnName << " = " << value << std::endl;
                i += 3;
            } else {
                i++;
            }
        }
    }

    // Parse WHERE clause
    if (i < tokens.size() && isToken(tokens[i], "keyword", "WHERE")) {
        i++;
        astNode* whereNode = new astNode("WHERE", "");
        parentNode->addChild(whereNode);

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
            parentNode->addChild(tableNode);
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
        parentNode->addChild(whereNode);

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
            parentNode->addChild(tableNode);
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
                            tableNode->addChild(columnNode);
                            std::cout << "Adding child node of type: COLUMN with value: " << columnName + " " + columnType
                                      << " to parent node of type: TABLE with value: " << tableNode->value << std::endl;
                            std::cout << "Column: " << columnName << " Type: " << columnType << std::endl;
                            i++;

                            // Parse constraints
                            while (i < tokens.size() && tokens[i].first == "keyword") {
                                std::string constraint = tokens[i].second;
                                columnNode->addChild(new astNode("CONSTRAINT", constraint));
                                std::cout << "Adding child node of type: CONSTRAINT with value: " << constraint
                                            << " to parent node of type: COLUMN with value: " << columnNode->value << std::endl;
                                std::cout << "Constraint: " << constraint << std::endl;
                                i++;
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
    parentNode->addChild(conditionNode);

    // Parse the left-hand side of the condition
    if (i < tokens.size() && tokens[i].first == "identifier") {
        if (i + 2 < tokens.size() && tokens[i + 1].second == "." && tokens[i + 2].first == "identifier") {
            // Handle dot notation (e.g., o.user_id)
            std::string aliasOrTable = tokens[i].second;
            std::string columnName = tokens[i + 2].second;
            conditionNode->addChild(new astNode("LEFT", "COLUMN(" + aliasOrTable + "." + columnName + ")"));
            i += 3;
        } else {
            // Handle standalone identifiers
            conditionNode->addChild(new astNode("LEFT", "COLUMN(" + tokens[i].second + ")"));
            i++;
        }
    }

    // Parse the operator
    if (i < tokens.size() && (tokens[i].first == "operator" || isToken(tokens[i], "keyword", "IN") || isToken(tokens[i], "keyword", "LIKE") || isToken(tokens[i], "keyword", "BETWEEN"))) {
        conditionNode->addChild(new astNode("OPERATOR", tokens[i].second));
        i++;
    }

    // Parse the right-hand side of the condition
    if (i < tokens.size()) {
        if (tokens[i].second == "(") {
            // Handle subquery on the right-hand side
            astNode* rightNode = new astNode("RIGHT", "QUERY");
            conditionNode->addChild(rightNode);
            handleSubquery(tokens, i, storedQuery, rightNode);
        } else if (tokens[i].first == "number" || tokens[i].first == "string") {
            conditionNode->addChild(new astNode("RIGHT", "VALUE(" + tokens[i].second + ")"));
            i++;
        } else if (tokens[i].first == "identifier") {
            if (i + 2 < tokens.size() && tokens[i + 1].second == "." && tokens[i + 2].first == "identifier") {
                // Handle dot notation (e.g., u.id)
                std::string aliasOrTable = tokens[i].second;
                std::string columnName = tokens[i + 2].second;
                conditionNode->addChild(new astNode("RIGHT", "COLUMN(" + aliasOrTable + "." + columnName + ")"));
                i += 3;
            } else {
                // Handle standalone identifiers
                conditionNode->addChild(new astNode("RIGHT", "COLUMN(" + tokens[i].second + ")"));
                i++;
            }
        }
    }
}

void parser::parse(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure& storedQuery, astNode* parentNode) {
    std::cout << "Starting parsing process..." << std::endl;
    while (i < tokens.size()) {
        const auto& token = tokens[i];

        if (token.first == "keyword") {
            storedQuery.commandType = token.second;
            std::cout << "Command Type: " << token.second << std::endl;

            astNode* queryNode = new astNode("QUERY", token.second);
            parentNode->addChild(queryNode);

            if (token.second == "SELECT") {
                i++;
                parseSelect(tokens, i, storedQuery, queryNode);
            } else if (token.second == "INSERT") {
                i++;
                parseInsert(tokens, i, storedQuery, queryNode);
            } else if (token.second == "UPDATE") {
                i++;
                parseUpdate(tokens, i, storedQuery, queryNode);
            } else if (token.second == "DELETE") {
                i++;
                parseDelete(tokens, i, storedQuery, queryNode);
            } else if (token.second == "CREATE") {
                i++;
                parseCreate(tokens, i, storedQuery, queryNode);
            } else {
                std::cerr << "Error: Unsupported command type: " << token.second << std::endl;
            }
        }
        i++;
    }

    if (storedQuery.commandType.empty() || storedQuery.tableName.empty()) {
        std::cerr << "Error: Invalid query." << std::endl;
    }
    std::cout << "Parsing completed." << std::endl;
}