#include <iostream>
#include <string>
#include <vector>
#include "../../include/parser/parser.hpp"

// Helper function to check if a token matches a specific type and value
bool parser::isToken(const std::pair<std::string, std::string>& token, const std::string& type, const std::string& value = "") {
    return token.first == type && (value.empty() || token.second == value);
}

// Recursive subquery handler
bool parser::handleSubquery(const std::vector<std::pair<std::string, std::string>>& tokens, queryStructure& parentQuery) {
    if (i < tokens.size() && tokens[i].second == "(" && i + 1 < tokens.size() && isToken(tokens[i + 1], "keyword", "SELECT")) {
        i++; // Skip '('
        queryStructure subQuery;
        std::cout << "Parsing subquery..." << std::endl;
        parse(tokens, subQuery); // Recursive parse
        parentQuery.subqueries.push_back(subQuery);

        // Skip until the closing parenthesis
        while (i < tokens.size() && tokens[i].second != ")") i++;
        if (i < tokens.size() && tokens[i].second == ")") i++; // Skip ')'
        return true;
    }
    return false;
}

// ---------------- SELECT ----------------
void parser::parseSelect(const std::vector<std::pair<std::string, std::string>>& tokens, queryStructure& storedQuery) {
    std::cout << "Parsing SELECT query..." << std::endl;

    // Parse columns
    while (i < tokens.size() && !isToken(tokens[i], "keyword", "FROM")) {
        if (tokens[i].first == "identifier" || tokens[i].second == "*") {
            storedQuery.columns.push_back(tokens[i].second);
            std::cout << "Column: " << tokens[i].second << std::endl;
        }
        i++;
    }

    // Parse FROM clause
    if (i < tokens.size() && isToken(tokens[i], "keyword", "FROM")) {
        i++;
        if (!handleSubquery(tokens, storedQuery)) {
            if (i < tokens.size() && tokens[i].first == "identifier") {
                storedQuery.tableName = tokens[i].second;
                std::cout << "Table Name: " << storedQuery.tableName << std::endl;
                i++;
            } else {
                std::cerr << "Error: Expected table name or subquery after FROM." << std::endl;
            }
        }
    }

    // Parse WHERE clause
    if (i < tokens.size() && isToken(tokens[i], "keyword", "WHERE")) {
        i++;
        std::string whereClause;
        while (i < tokens.size() && tokens[i].second != "keyword") {
            if (handleSubquery(tokens, storedQuery)) {
                whereClause += "[SUBQUERY] ";
            } else {
                whereClause += tokens[i].second + " ";
                i++;
            }
        }
        storedQuery.whereClause = whereClause;
        std::cout << "WHERE Clause: " << whereClause << std::endl;
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
void parser::parseInsert(const std::vector<std::pair<std::string, std::string>>& tokens, queryStructure& storedQuery) {
    std::cout << "Parsing INSERT query..." << std::endl;

    if (i + 1 < tokens.size() && isToken(tokens[i], "keyword", "INTO") && tokens[i + 1].first == "identifier") {
        i += 2;
        storedQuery.tableName = tokens[i].second;
        std::cout << "Table Name: " << storedQuery.tableName << std::endl;
        i++;

        // Check if VALUES or subquery
        if (i < tokens.size() && isToken(tokens[i], "keyword", "VALUES")) {
            i++;
            if (i < tokens.size() && tokens[i].second == "(") {
                i++;
                while (i < tokens.size() && tokens[i].second != ")") {
                    if (tokens[i].first == "string" || tokens[i].first == "number") {
                        storedQuery.values.push_back(tokens[i].second);
                        std::cout << "Value: " << tokens[i].second << std::endl;
                    }
                    i++;
                }
                if (i < tokens.size() && tokens[i].second == ")") i++;
            }
        } 
        else if (i < tokens.size() && tokens[i].second == "(" && i + 1 < tokens.size() && isToken(tokens[i + 1], "keyword", "SELECT")) {
            // Subquery instead of VALUES
            handleSubquery(tokens, storedQuery);
        } 
        else {
            std::cerr << "Error: Expected VALUES or subquery in INSERT." << std::endl;
        }
    }
}

// ---------------- UPDATE ----------------
void parser::parseUpdate(const std::vector<std::pair<std::string, std::string>>& tokens, queryStructure& storedQuery) {
    std::cout << "Parsing UPDATE query..." << std::endl;

    if (i < tokens.size() && tokens[i].first == "identifier") {
        storedQuery.tableName = tokens[i].second;
        std::cout << "Table Name: " << storedQuery.tableName << std::endl;
        i++;
    }

    if (i < tokens.size() && isToken(tokens[i], "keyword", "SET")) {
        i++;
        while (i < tokens.size() && !isToken(tokens[i], "keyword", "WHERE")) {
            if (tokens[i].first == "identifier" && i + 2 < tokens.size() && tokens[i + 1].second == "=") {
                storedQuery.columns.push_back(tokens[i].second);
                
                if (!handleSubquery(tokens, storedQuery)) {
                    storedQuery.values.push_back(tokens[i + 2].second);
                } else {
                    storedQuery.values.push_back("[SUBQUERY]");
                }

                std::cout << "Column: " << storedQuery.columns.back()
                          << " Value: " << storedQuery.values.back() << std::endl;
                i += 3;
            } else {
                i++;
            }
        }
    }

    // WHERE clause
    if (i < tokens.size() && isToken(tokens[i], "keyword", "WHERE")) {
        i++;
        std::string whereClause;
        while (i < tokens.size()) {
            if (handleSubquery(tokens, storedQuery)) {
                whereClause += "[SUBQUERY] ";
            } else {
                whereClause += tokens[i].second + " ";
                i++;
            }
        }
        storedQuery.whereClause = whereClause;
        std::cout << "WHERE Clause: " << storedQuery.whereClause << std::endl;
    }
}

// ---------------- DELETE ----------------
void parser::parseDelete(const std::vector<std::pair<std::string, std::string>>& tokens, queryStructure& storedQuery) {
    std::cout << "Parsing DELETE query..." << std::endl;

    if (i < tokens.size() && isToken(tokens[i], "keyword", "FROM")) {
        i++;
        if (!handleSubquery(tokens, storedQuery)) {
            if (i < tokens.size() && tokens[i].first == "identifier") {
                storedQuery.tableName = tokens[i].second;
                std::cout << "Table Name: " << storedQuery.tableName << std::endl;
                i++;
            }
        }
    }

    if (i < tokens.size() && isToken(tokens[i], "keyword", "WHERE")) {
        i++;
        std::string whereClause;
        while (i < tokens.size()) {
            if (handleSubquery(tokens, storedQuery)) {
                whereClause += "[SUBQUERY] ";
            } else {
                whereClause += tokens[i].second + " ";
                i++;
            }
        }
        storedQuery.whereClause = whereClause;
        std::cout << "WHERE Clause: " << storedQuery.whereClause << std::endl;
    }
}

void parser::parseCreate(const std::vector<std::pair<std::string, std::string>>& tokens, queryStructure& storedQuery) {
    std::cout << "Parsing CREATE TABLE query..." << std::endl;

    // Check if the next token is "TABLE"
    if (i + 1 < tokens.size() && isToken(tokens[i], "keyword", "TABLE")) {
        i++;
        if (i < tokens.size() && tokens[i].first == "identifier") {
            storedQuery.tableName = tokens[i].second;
            std::cout << "Table Name: " << storedQuery.tableName << std::endl;
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
                            std::string key = "";
                            if(i+1 < tokens.size() && tokens[i+1].first == "keyword" ){
                                i++;
                                while(i < tokens.size() && tokens[i].second != "," && (tokens[i].second != ")" && (i + 1 < tokens.size() && tokens[i+1].second != ";"))){
                                    key += " " + tokens[i].second;
                                    i++;
                                }
                                std::cout<<tokens[i].second<<std::endl;
                            } 
                            storedQuery.columns.push_back(columnName + " " + columnType);
                            std::cout << "Column: " << columnName << " Type: " << columnType << std::endl;
                            if(!key.empty()){
                                storedQuery.constraints[columnName].push_back(key);
                                std::cout << "Constraint: " << key << " on Column: " << columnName << std::endl;
                            }
                        } else {
                            std::cerr << "Error: Expected datatype for column " << columnName << std::endl;
                            return;
                        }
                    }
                    if (i < tokens.size() && tokens[i].first != "identifier" ) {
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

void parser::parse(const std::vector<std::pair<std::string, std::string>>& tokens, queryStructure& storedQuery) {
    std::cout << "Starting parsing process..." << std::endl;

    while (i < tokens.size()) {
        const auto& token = tokens[i];

        if (token.first == "keyword") {
            storedQuery.commandType = token.second;
            std::cout << "Command Type: " << token.second << std::endl;

            if (token.second == "SELECT") {
                i++;
                parseSelect(tokens, storedQuery);
            } else if (token.second == "INSERT") {
                i++;
                parseInsert(tokens, storedQuery);
            } else if (token.second == "UPDATE") {
                i++;
                parseUpdate(tokens, storedQuery);
            } else if (token.second == "DELETE") {
                i++;
                parseDelete(tokens, storedQuery);
            }else if (token.second == "CREATE") {
                i++;
                parseCreate(tokens, storedQuery);
            } 
            else {
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