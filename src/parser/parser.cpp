#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "../../include/parser/parser.hpp"

bool isTableColumn(const std::vector<Token>& tokens, int index) {
    if (index + 2 < tokens.size() &&
        tokens[index].type == TokenType::IDENTIFIER &&
        tokens[index + 1].type == TokenType::PUNCTUATION && tokens[index + 1].value == "." &&
        tokens[index + 2].type == TokenType::IDENTIFIER) {
        return true;
    }
    return false;
    
}

// Helper function to check if a token matches a specific type and value
bool parser::isToken(const Token& token, TokenType type, const std::string& value = "") {}

// Recursive subquery handler
bool parser::handleSubquery(const std::vector<Token>& tokens, astNode* parentNode) {

}

// ---------------- SELECT ----------------
bool parser::parseSelect(const std::vector<Token>& tokens, astNode* parentNode) {
    std::cout << "Parsing SELECT statement..." << std::endl;
    
    astNode* selectNode = new astNode("SELECT", "");
    parentNode->addChild(selectNode);
    
    itr += 1; // Skip SELECT keyword
    
    // Handle DISTINCT
    if (itr.getVal() < tokens.size() && 
        tokens[itr.getVal()].type == TokenType::KEYWORD && 
        tokens[itr.getVal()].value == "DISTINCT") {
        std::cout << "DISTINCT found" << std::endl;
        selectNode->addChild(new astNode("DISTINCT", "DISTINCT"));
        itr += 1;
    }
    
    // Parse SELECT list
    astNode* selectListNode = new astNode("SELECT_LIST", "");
    selectNode->addChild(selectListNode);
    
    if (!parseSelectList(tokens, selectListNode)) {
        std::cout << "Error: Failed to parse SELECT list" << std::endl;
        return false;
    }
    
    // Expect FROM keyword
    if (itr.getVal() >= tokens.size() || 
        tokens[itr.getVal()].type != TokenType::KEYWORD || 
        tokens[itr.getVal()].value != "FROM") {
        std::cout << "Error: Expected FROM keyword" << std::endl;
        return false;
    }
    
    itr += 1; // Skip FROM
    
    // Parse FROM clause
    astNode* fromNode = new astNode("FROM", "");
    selectNode->addChild(fromNode);
    
    if (!parseFromClause(tokens, fromNode)) {
        std::cout << "Error: Failed to parse FROM clause" << std::endl;
        return false;
    }
    
    // Parse optional clauses
    while (itr.getVal() < tokens.size()) {
        const Token& currentToken = tokens[itr.getVal()];
        
        if (currentToken.type == TokenType::KEYWORD) {
            if (currentToken.value == "WHERE") {
                std::cout << "Parsing WHERE clause..." << std::endl;
                astNode* whereNode = new astNode("WHERE", "");
                selectNode->addChild(whereNode);
                itr += 1;
                
                if (!parseCondition(tokens, whereNode)) {
                    return false;
                }
            } else if (currentToken.value == "GROUP") {
                if (!parseGroupBy(tokens, selectNode)) {
                    return false;
                }
            } else if (currentToken.value == "HAVING") {
                if (!parseHaving(tokens, selectNode)) {
                    return false;
                }
            } else if (currentToken.value == "ORDER") {
                if (!parseOrderBy(tokens, selectNode)) {
                    return false;
                }
            } else if (currentToken.value == "LIMIT") {
                if (!parseLimit(tokens, selectNode)) {
                    return false;
                }
            } else {
                break; // Unknown keyword, stop parsing
            }
        } else {
            break; // Not a keyword, stop parsing
        }
    }
    
    return true;
}

// Parse SELECT list (columns, expressions, functions)
bool parser::parseSelectList(const std::vector<Token>& tokens, astNode* parentNode) {
    do {
        astNode* columnNode = new astNode("COLUMN_EXPR", "");
        parentNode->addChild(columnNode);
        
        if (!parseSelectExpression(tokens, columnNode)) {
            std::cout << "Error: Failed to parse SELECT expression" << std::endl;
            return false;
        }
        
        // Check for alias (AS keyword or direct identifier)
        if (itr.getVal() < tokens.size()) {
            const Token& nextToken = tokens[itr.getVal()];
            
            if (nextToken.type == TokenType::KEYWORD && nextToken.value == "AS") {
                itr += 1;
                if (itr.getVal() < tokens.size() && 
                    tokens[itr.getVal()].type == TokenType::IDENTIFIER) {
                    std::cout << "Column alias: " << tokens[itr.getVal()].value << std::endl;
                    columnNode->addChild(new astNode("ALIAS", tokens[itr.getVal()].value));
                    itr += 1;
                }
            } else if (nextToken.type == TokenType::IDENTIFIER && 
                       !isTableColumn(tokens, itr.getVal()) &&
                       (itr.getVal() + 1 >= tokens.size() || 
                        tokens[itr.getVal() + 1].value == "," || 
                        tokens[itr.getVal() + 1].value == "FROM")) {
                // Direct alias without AS
                std::cout << "Column alias (no AS): " << nextToken.value << std::endl;
                columnNode->addChild(new astNode("ALIAS", nextToken.value));
                itr += 1;
            }
        }
        
        // Check for comma
        if (itr.getVal() < tokens.size() && 
            tokens[itr.getVal()].type == TokenType::PUNCTUATION && 
            tokens[itr.getVal()].value == ",") {
            itr += 1;
        } else {
            break; // No more columns
        }
    } while (itr.getVal() < tokens.size());
    
    return true;
}

// Parse individual SELECT expression (column, function, literal, etc.)
bool parser::parseSelectExpression(const std::vector<Token>& tokens, astNode* parentNode) {
    if (itr.getVal() >= tokens.size()) {
        return false;
    }
    
    const Token& currentToken = tokens[itr.getVal()];
    
    // Handle wildcard (*)
    if (currentToken.type == TokenType::OPERATOR && currentToken.value == "*") {
        std::cout << "Wildcard (*) found" << std::endl;
        parentNode->addChild(new astNode("WILDCARD", "*"));
        itr += 1;
        return true;
    }
    
    // Handle aggregate functions
    if (currentToken.type == TokenType::IDENTIFIER && 
        (currentToken.value == "COUNT" || currentToken.value == "SUM" || 
         currentToken.value == "AVG" || currentToken.value == "MIN" || 
         currentToken.value == "MAX")) {
        return parseAggregateFunction(tokens, parentNode);
    }
    
    // Handle table.column or simple column
    if (isTableColumn(tokens, itr.getVal())) {
        std::cout << "Table.column: " << tokens[itr.getVal()].value << "." << tokens[itr.getVal() + 2].value << std::endl;
        parentNode->addChild(new astNode("COLUMN", tokens[itr.getVal()].value + "." + tokens[itr.getVal() + 2].value));
        itr += 3;
        return true;
    }
    
    // Handle simple column or expression
    if (!parseValue(tokens, parentNode)) {
        std::cout << "Error: Expected column name or expression" << std::endl;
        return false;
    }
    
    return true;
}

// Parse aggregate functions
bool parser::parseAggregateFunction(const std::vector<Token>& tokens, astNode* parentNode) {
    const Token& functionToken = tokens[itr.getVal()];
    std::cout << "Aggregate function: " << functionToken.value << std::endl;
    
    astNode* functionNode = new astNode("FUNCTION", functionToken.value);
    parentNode->addChild(functionNode);
    itr += 1;
    
    // Expect opening parenthesis
    if (itr.getVal() >= tokens.size() || 
        tokens[itr.getVal()].type != TokenType::PUNCTUATION || 
        tokens[itr.getVal()].value != "(") {
        std::cout << "Error: Expected '(' after function name" << std::endl;
        return false;
    }
    itr += 1;
    
    // Parse function arguments
    astNode* argsNode = new astNode("ARGS", "");
    functionNode->addChild(argsNode);
    
    // Handle special case: COUNT(*)
    if (itr.getVal() < tokens.size() && 
        tokens[itr.getVal()].type == TokenType::OPERATOR && 
        tokens[itr.getVal()].value == "*") {
        std::cout << "Function argument: *" << std::endl;
        argsNode->addChild(new astNode("WILDCARD", "*"));
        itr += 1;
    } else {
        // Parse column or expression
        if (!parseValue(tokens, argsNode)) {
            std::cout << "Error: Expected function argument" << std::endl;
            return false;
        }
    }
    
    // Expect closing parenthesis
    if (itr.getVal() >= tokens.size() || 
        tokens[itr.getVal()].type != TokenType::PUNCTUATION || 
        tokens[itr.getVal()].value != ")") {
        std::cout << "Error: Expected ')' after function arguments" << std::endl;
        return false;
    }
    itr += 1;
    
    return true;
}

// Parse FROM clause with JOIN support
bool parser::parseFromClause(const std::vector<Token>& tokens, astNode* parentNode) {
    // Parse first table
    if (!parseTableReference(tokens, parentNode)) {
        return false;
    }
    
    // Parse optional JOINs
    while (itr.getVal() < tokens.size()) {
        const Token& currentToken = tokens[itr.getVal()];
        
        if (currentToken.type == TokenType::KEYWORD && 
            (currentToken.value == "JOIN" || currentToken.value == "INNER" || 
             currentToken.value == "LEFT" || currentToken.value == "RIGHT" || 
             currentToken.value == "FULL")) {
            if (!parseJoinClause(tokens, parentNode)) {
                return false;
            }
        } else {
            break; // No more JOINs
        }
    }
    
    return true;
}

// Parse table reference (table name with optional alias)
bool parser::parseTableReference(const std::vector<Token>& tokens, astNode* parentNode) {
    if (itr.getVal() >= tokens.size() || 
        tokens[itr.getVal()].type != TokenType::IDENTIFIER) {
        std::cout << "Error: Expected table name" << std::endl;
        return false;
    }
    
    const Token& tableToken = tokens[itr.getVal()];
    std::cout << "Table: " << tableToken.value << std::endl;
    
    astNode* tableNode = new astNode("TABLE", tableToken.value);
    parentNode->addChild(tableNode);
    itr += 1;
    
    // Check for table alias
    if (itr.getVal() < tokens.size() && 
        tokens[itr.getVal()].type == TokenType::IDENTIFIER &&
        (itr.getVal() + 1 >= tokens.size() || 
         (tokens[itr.getVal() + 1].type == TokenType::KEYWORD && 
          (tokens[itr.getVal() + 1].value == "WHERE" || 
           tokens[itr.getVal() + 1].value == "JOIN" || 
           tokens[itr.getVal() + 1].value == "INNER" || 
           tokens[itr.getVal() + 1].value == "LEFT" || 
           tokens[itr.getVal() + 1].value == "RIGHT" || 
           tokens[itr.getVal() + 1].value == "GROUP" || 
           tokens[itr.getVal() + 1].value == "ORDER")))) {
        std::cout << "Table alias: " << tokens[itr.getVal()].value << std::endl;
        tableNode->addChild(new astNode("ALIAS", tokens[itr.getVal()].value));
        itr += 1;
    }
    
    return true;
}

// Parse JOIN clause
bool parser::parseJoinClause(const std::vector<Token>& tokens, astNode* parentNode) {
    astNode* joinNode = new astNode("JOIN", "");
    parentNode->addChild(joinNode);
    
    std::string joinType = "";
    
    // Parse JOIN type
    const Token& currentToken = tokens[itr.getVal()];
    if (currentToken.value == "INNER" || currentToken.value == "LEFT" || 
        currentToken.value == "RIGHT" || currentToken.value == "FULL") {
        joinType = currentToken.value;
        itr += 1;
        
        if (itr.getVal() < tokens.size() && 
            tokens[itr.getVal()].type == TokenType::KEYWORD && 
            tokens[itr.getVal()].value == "OUTER") {
            joinType += " OUTER";
            itr += 1;
        }
        
        if (itr.getVal() >= tokens.size() || 
            tokens[itr.getVal()].type != TokenType::KEYWORD || 
            tokens[itr.getVal()].value != "JOIN") {
            std::cout << "Error: Expected JOIN after " << joinType << std::endl;
            return false;
        }
        joinType += " JOIN";
    } else if (currentToken.value == "JOIN") {
        joinType = "INNER JOIN";
    }
    
    std::cout << "JOIN type: " << joinType << std::endl;
    joinNode->addChild(new astNode("JOIN_TYPE", joinType));
    itr += 1; // Skip JOIN keyword
    
    // Parse joined table
    if (!parseTableReference(tokens, joinNode)) {
        return false;
    }
    
    // Parse ON condition
    if (itr.getVal() >= tokens.size() || 
        tokens[itr.getVal()].type != TokenType::KEYWORD || 
        tokens[itr.getVal()].value != "ON") {
        std::cout << "Error: Expected ON after JOIN table" << std::endl;
        return false;
    }
    
    std::cout << "Parsing JOIN ON condition..." << std::endl;
    astNode* onNode = new astNode("ON", "");
    joinNode->addChild(onNode);
    itr += 1;
    
    return parseCondition(tokens, onNode);
}

// Parse GROUP BY clause
bool parser::parseGroupBy(const std::vector<Token>& tokens, astNode* parentNode) {
    std::cout << "Parsing GROUP BY clause..." << std::endl;
    
    // Expect GROUP BY
    if (itr.getVal() + 1 >= tokens.size() || 
        tokens[itr.getVal()].value != "GROUP" || 
        tokens[itr.getVal() + 1].value != "BY") {
        std::cout << "Error: Expected GROUP BY" << std::endl;
        return false;
    }
    
    astNode* groupByNode = new astNode("GROUP_BY", "");
    parentNode->addChild(groupByNode);
    itr += 2; // Skip GROUP BY
    
    // Parse column list
    do {
        if (!parseValue(tokens, groupByNode)) {
            std::cout << "Error: Expected column in GROUP BY" << std::endl;
            return false;
        }
        
        if (itr.getVal() < tokens.size() && 
            tokens[itr.getVal()].type == TokenType::PUNCTUATION && 
            tokens[itr.getVal()].value == ",") {
            itr += 1;
        } else {
            break;
        }
    } while (itr.getVal() < tokens.size());
    
    return true;
}

// Parse HAVING clause
bool parser::parseHaving(const std::vector<Token>& tokens, astNode* parentNode) {
    std::cout << "Parsing HAVING clause..." << std::endl;
    
    astNode* havingNode = new astNode("HAVING", "");
    parentNode->addChild(havingNode);
    itr += 1; // Skip HAVING
    
    return parseCondition(tokens, havingNode);
}

// Parse ORDER BY clause
bool parser::parseOrderBy(const std::vector<Token>& tokens, astNode* parentNode) {
    std::cout << "Parsing ORDER BY clause..." << std::endl;
    
    // Expect ORDER BY
    if (itr.getVal() + 1 >= tokens.size() || 
        tokens[itr.getVal()].value != "ORDER" || 
        tokens[itr.getVal() + 1].value != "BY") {
        std::cout << "Error: Expected ORDER BY" << std::endl;
        return false;
    }
    
    astNode* orderByNode = new astNode("ORDER_BY", "");
    parentNode->addChild(orderByNode);
    itr += 2; // Skip ORDER BY
    
    // Parse order expressions
    do {
        astNode* orderExprNode = new astNode("ORDER_EXPR", "");
        orderByNode->addChild(orderExprNode);
        
        if (!parseValue(tokens, orderExprNode)) {
            std::cout << "Error: Expected column in ORDER BY" << std::endl;
            return false;
        }
        
        // Check for ASC/DESC
        if (itr.getVal() < tokens.size() && 
            tokens[itr.getVal()].type == TokenType::IDENTIFIER &&
            (tokens[itr.getVal()].value == "ASC" || tokens[itr.getVal()].value == "DESC")) {
            std::cout << "Order direction: " << tokens[itr.getVal()].value << std::endl;
            orderExprNode->addChild(new astNode("DIRECTION", tokens[itr.getVal()].value));
            itr += 1;
        }
        
        if (itr.getVal() < tokens.size() && 
            tokens[itr.getVal()].type == TokenType::PUNCTUATION && 
            tokens[itr.getVal()].value == ",") {
            itr += 1;
        } else {
            break;
        }
    } while (itr.getVal() < tokens.size());
    
    return true;
}

// Parse LIMIT clause
bool parser::parseLimit(const std::vector<Token>& tokens, astNode* parentNode) {
    std::cout << "Parsing LIMIT clause..." << std::endl;
    
    astNode* limitNode = new astNode("LIMIT", "");
    parentNode->addChild(limitNode);
    itr += 1; // Skip LIMIT
    
    if (itr.getVal() >= tokens.size() || 
        tokens[itr.getVal()].type != TokenType::NUMBER) {
        std::cout << "Error: Expected number after LIMIT" << std::endl;
        return false;
    }
    
    std::cout << "Limit value: " << tokens[itr.getVal()].value << std::endl;
    limitNode->addChild(new astNode("VALUE", tokens[itr.getVal()].value));
    itr += 1;
    
    return true;
}

// ---------------- INSERT ----------------
bool parser::parseInsert(const std::vector<Token>& tokens, astNode* parentNode) {}

// ---------------- UPDATE ----------------
bool parser::parseUpdate(const std::vector<Token>& tokens, astNode* parentNode) {}

// ---------------- DELETE ----------------
bool parser::parseDelete(const std::vector<Token>& tokens, astNode* parentNode) {}

// ---------------- Condition Parsing ----------------
bool parser::parseCondition(const std::vector<Token>& tokens, astNode* parentNode) {
    std::cout << "Parsing condition..." << std::endl;
    astNode* conditionNode = new astNode("CONDITION", "");
    parentNode->addChild(conditionNode);
    
    return parseLogicalExpression(tokens, conditionNode);
}

// Parse logical expressions (AND, OR)
bool parser::parseLogicalExpression(const std::vector<Token>& tokens, astNode* parentNode) {
    if (!parseLogicalTerm(tokens, parentNode)) {
        return false;
    }
    
    while (itr.getVal() < tokens.size()) {
        const Token& currentToken = tokens[itr.getVal()];
        
        if (currentToken.type == TokenType::KEYWORD && 
            (currentToken.value == "OR" || currentToken.value == "||")) {
            std::cout << "Logical OR operator found" << std::endl;
            astNode* orNode = new astNode("LOGICAL_OP", "OR");
            parentNode->addChild(orNode);
            itr += 1;
            
            if (!parseLogicalTerm(tokens, orNode)) {
                return false;
            }
        } else {
            break;
        }
    }
    
    return true;
}

// Parse logical terms (AND operations)
bool parser::parseLogicalTerm(const std::vector<Token>& tokens, astNode* parentNode) {
    if (!parseLogicalFactor(tokens, parentNode)) {
        return false;
    }
    
    while (itr.getVal() < tokens.size()) {
        const Token& currentToken = tokens[itr.getVal()];
        
        if (currentToken.type == TokenType::KEYWORD && 
            (currentToken.value == "AND" || currentToken.value == "&&")) {
            std::cout << "Logical AND operator found" << std::endl;
            astNode* andNode = new astNode("LOGICAL_OP", "AND");
            parentNode->addChild(andNode);
            itr += 1;
            
            if (!parseLogicalFactor(tokens, andNode)) {
                return false;
            }
        } else {
            break;
        }
    }
    
    return true;
}

// Parse logical factors (NOT, parentheses, comparison expressions)
bool parser::parseLogicalFactor(const std::vector<Token>& tokens, astNode* parentNode) {
    if (itr.getVal() >= tokens.size()) {
        return false;
    }
    
    const Token& currentToken = tokens[itr.getVal()];
    
    // Handle NOT operator
    if (currentToken.type == TokenType::KEYWORD && currentToken.value == "NOT") {
        std::cout << "NOT operator found" << std::endl;
        astNode* notNode = new astNode("LOGICAL_OP", "NOT");
        parentNode->addChild(notNode);
        itr += 1;
        
        return parseLogicalFactor(tokens, notNode);
    }
    
    // Handle parentheses
    if (currentToken.type == TokenType::PUNCTUATION && currentToken.value == "(") {
        std::cout << "Opening parenthesis found in condition" << std::endl;
        itr += 1;
        
        astNode* groupNode = new astNode("GROUP", "");
        parentNode->addChild(groupNode);
        
        if (!parseLogicalExpression(tokens, groupNode)) {
            return false;
        }
        
        if (itr.getVal() >= tokens.size() || 
            tokens[itr.getVal()].type != TokenType::PUNCTUATION || 
            tokens[itr.getVal()].value != ")") {
            std::cout << "Error: Expected closing parenthesis" << std::endl;
            return false;
        }
        
        std::cout << "Closing parenthesis found" << std::endl;
        itr += 1;
        return true;
    }
    
    // Handle comparison expressions
    return parseComparisonExpression(tokens, parentNode);
}

// Parse comparison expressions (=, <, >, <=, >=, <>, !=, LIKE, IN, BETWEEN, IS, EXISTS)
bool parser::parseComparisonExpression(const std::vector<Token>& tokens, astNode* parentNode) {
    astNode* comparisonNode = new astNode("COMPARISON", "");
    parentNode->addChild(comparisonNode);
    
    // Parse left operand
    if (!parseValue(tokens, comparisonNode)) {
        std::cout << "Error: Expected value or column in comparison" << std::endl;
        return false;
    }
    
    if (itr.getVal() >= tokens.size()) {
        return true; // Simple value without comparison
    }
    
    const Token& operatorToken = tokens[itr.getVal()];
    
    // Handle different types of operators
    if (operatorToken.type == TokenType::OPERATOR) {
        return parseSimpleComparison(tokens, comparisonNode);
    } else if (operatorToken.type == TokenType::KEYWORD) {
        if (operatorToken.value == "LIKE") {
            return parseLikeExpression(tokens, comparisonNode);
        } else if (operatorToken.value == "IN") {
            return parseInExpression(tokens, comparisonNode);
        } else if (operatorToken.value == "BETWEEN") {
            return parseBetweenExpression(tokens, comparisonNode);
        } else if (operatorToken.value == "IS") {
            return parseIsExpression(tokens, comparisonNode);
        } else if (operatorToken.value == "EXISTS") {
            return parseExistsExpression(tokens, comparisonNode);
        }
    }
    
    return true; // No operator found, just a simple value
}

// Parse simple comparison operators (=, <, >, <=, >=, <>, !=)
bool parser::parseSimpleComparison(const std::vector<Token>& tokens, astNode* parentNode) {
    const Token& operatorToken = tokens[itr.getVal()];
    std::cout << "Comparison operator: " << operatorToken.value << std::endl;
    
    astNode* operatorNode = new astNode("OPERATOR", operatorToken.value);
    parentNode->addChild(operatorNode);
    itr += 1;
    
    // Parse right operand
    if (!parseValue(tokens, parentNode)) {
        std::cout << "Error: Expected value after comparison operator" << std::endl;
        return false;
    }
    
    return true;
}

// Parse LIKE expression
bool parser::parseLikeExpression(const std::vector<Token>& tokens, astNode* parentNode) {
    std::cout << "LIKE operator found" << std::endl;
    astNode* likeNode = new astNode("LIKE", "");
    parentNode->addChild(likeNode);
    itr += 1;
    
    if (!parseValue(tokens, likeNode)) {
        std::cout << "Error: Expected pattern after LIKE" << std::endl;
        return false;
    }
    
    return true;
}

// Parse IN expression
bool parser::parseInExpression(const std::vector<Token>& tokens, astNode* parentNode) {
    std::cout << "IN operator found" << std::endl;
    astNode* inNode = new astNode("IN", "");
    parentNode->addChild(inNode);
    itr += 1;
    
    if (itr.getVal() >= tokens.size()) {
        std::cout << "Error: Expected value list or subquery after IN" << std::endl;
        return false;
    }
    
    const Token& nextToken = tokens[itr.getVal()];
    
    if (nextToken.type == TokenType::PUNCTUATION && nextToken.value == "(") {
        itr += 1;
        
        // Check if it's a subquery
        if (itr.getVal() < tokens.size() && 
            tokens[itr.getVal()].type == TokenType::KEYWORD && 
            tokens[itr.getVal()].value == "SELECT") {
            std::cout << "Subquery in IN clause detected" << std::endl;
            return handleSubquery(tokens, inNode);
        } else {
            // Parse value list
            std::cout << "Value list in IN clause" << std::endl;
            astNode* valueListNode = new astNode("VALUE_LIST", "");
            inNode->addChild(valueListNode);
            
            do {
                if (!parseValue(tokens, valueListNode)) {
                    std::cout << "Error: Expected value in IN list" << std::endl;
                    return false;
                }
                
                if (itr.getVal() < tokens.size() && 
                    tokens[itr.getVal()].type == TokenType::PUNCTUATION && 
                    tokens[itr.getVal()].value == ",") {
                    itr += 1;
                } else {
                    break;
                }
            } while (itr.getVal() < tokens.size());
            
            if (itr.getVal() >= tokens.size() || 
                tokens[itr.getVal()].type != TokenType::PUNCTUATION || 
                tokens[itr.getVal()].value != ")") {
                std::cout << "Error: Expected closing parenthesis in IN clause" << std::endl;
                return false;
            }
            itr += 1;
        }
    }
    
    return true;
}

// Parse BETWEEN expression
bool parser::parseBetweenExpression(const std::vector<Token>& tokens, astNode* parentNode) {
    std::cout << "BETWEEN operator found" << std::endl;
    astNode* betweenNode = new astNode("BETWEEN", "");
    parentNode->addChild(betweenNode);
    itr += 1;
    
    // Parse first value
    if (!parseValue(tokens, betweenNode)) {
        std::cout << "Error: Expected first value after BETWEEN" << std::endl;
        return false;
    }
    
    // Expect AND keyword
    if (itr.getVal() >= tokens.size() || 
        tokens[itr.getVal()].type != TokenType::KEYWORD || 
        tokens[itr.getVal()].value != "AND") {
        std::cout << "Error: Expected AND after first value in BETWEEN" << std::endl;
        return false;
    }
    itr += 1;
    
    // Parse second value
    if (!parseValue(tokens, betweenNode)) {
        std::cout << "Error: Expected second value after AND in BETWEEN" << std::endl;
        return false;
    }
    
    return true;
}

// Parse IS expression (IS NULL, IS NOT NULL)
bool parser::parseIsExpression(const std::vector<Token>& tokens, astNode* parentNode) {
    std::cout << "IS operator found" << std::endl;
    astNode* isNode = new astNode("IS", "");
    parentNode->addChild(isNode);
    itr += 1;
    
    if (itr.getVal() >= tokens.size()) {
        std::cout << "Error: Expected value after IS" << std::endl;
        return false;
    }
    
    const Token& nextToken = tokens[itr.getVal()];
    
    // Handle IS NOT
    if (nextToken.type == TokenType::KEYWORD && nextToken.value == "NOT") {
        std::cout << "IS NOT found" << std::endl;
        astNode* notNode = new astNode("NOT", "");
        isNode->addChild(notNode);
        itr += 1;
        
        if (!parseValue(tokens, notNode)) {
            std::cout << "Error: Expected value after IS NOT" << std::endl;
            return false;
        }
    } else {
        if (!parseValue(tokens, isNode)) {
            std::cout << "Error: Expected value after IS" << std::endl;
            return false;
        }
    }
    
    return true;
}

// Parse EXISTS expression
bool parser::parseExistsExpression(const std::vector<Token>& tokens, astNode* parentNode) {
    std::cout << "EXISTS operator found" << std::endl;
    astNode* existsNode = new astNode("EXISTS", "");
    parentNode->addChild(existsNode);
    itr += 1;
    
    if (itr.getVal() >= tokens.size() || 
        tokens[itr.getVal()].type != TokenType::PUNCTUATION || 
        tokens[itr.getVal()].value != "(") {
        std::cout << "Error: Expected opening parenthesis after EXISTS" << std::endl;
        return false;
    }
    itr += 1;
    
    // Parse subquery
    if (itr.getVal() >= tokens.size() || 
        tokens[itr.getVal()].type != TokenType::KEYWORD || 
        tokens[itr.getVal()].value != "SELECT") {
        std::cout << "Error: Expected SELECT after EXISTS(" << std::endl;
        return false;
    }
    
    return handleSubquery(tokens, existsNode);
}

// Parse values (identifiers, literals, subqueries, functions)
bool parser::parseValue(const std::vector<Token>& tokens, astNode* parentNode) {
    if (itr.getVal() >= tokens.size()) {
        return false;
    }
    
    const Token& currentToken = tokens[itr.getVal()];
    
    // Handle table.column format
    if (isTableColumn(tokens, itr.getVal())) {
        std::cout << "Table column: " << tokens[itr.getVal()].value << "." << tokens[itr.getVal() + 2].value << std::endl;
        astNode* columnNode = new astNode("COLUMN", tokens[itr.getVal()].value + "." + tokens[itr.getVal() + 2].value);
        parentNode->addChild(columnNode);
        itr += 3;
        return true;
    }
    
    // Handle different token types
    switch (currentToken.type) {
        case TokenType::IDENTIFIER:
            std::cout << "Column: " << currentToken.value << std::endl;
            parentNode->addChild(new astNode("COLUMN", currentToken.value));
            itr += 1;
            return true;
            
        case TokenType::NUMBER:
        case TokenType::DOUBLE:
            std::cout << "Number: " << currentToken.value << std::endl;
            parentNode->addChild(new astNode("NUMBER", currentToken.value));
            itr += 1;
            return true;
            
        case TokenType::STRING:
            std::cout << "String: " << currentToken.value << std::endl;
            parentNode->addChild(new astNode("STRING", currentToken.value));
            itr += 1;
            return true;
            
        case TokenType::DATE:
            std::cout << "Date: " << currentToken.value << std::endl;
            parentNode->addChild(new astNode("DATE", currentToken.value));
            itr += 1;
            return true;
            
        case TokenType::KEYWORD:
            if (currentToken.value == "NULL") {
                std::cout << "NULL value" << std::endl;
                parentNode->addChild(new astNode("NULL", "NULL"));
                itr += 1;
                return true;
            } else if (currentToken.value == "TRUE" || currentToken.value == "FALSE") {
                std::cout << "Boolean: " << currentToken.value << std::endl;
                parentNode->addChild(new astNode("BOOLEAN", currentToken.value));
                itr += 1;
                return true;
            } else if (currentToken.value == "SELECT") {
                std::cout << "Subquery detected" << std::endl;
                return handleSubquery(tokens, parentNode);
            }
            break;
            
        case TokenType::PUNCTUATION:
            if (currentToken.value == "(") {
                // Could be a subquery or function call
                itr += 1;
                if (itr.getVal() < tokens.size() && 
                    tokens[itr.getVal()].type == TokenType::KEYWORD && 
                    tokens[itr.getVal()].value == "SELECT") {
                    std::cout << "Subquery in parentheses" << std::endl;
                    return handleSubquery(tokens, parentNode);
                } else {
                    // Parenthesized expression
                    astNode* groupNode = new astNode("GROUP", "");
                    parentNode->addChild(groupNode);
                    
                    if (!parseLogicalExpression(tokens, groupNode)) {
                        return false;
                    }
                    
                    if (itr.getVal() >= tokens.size() || 
                        tokens[itr.getVal()].type != TokenType::PUNCTUATION || 
                        tokens[itr.getVal()].value != ")") {
                        std::cout << "Error: Expected closing parenthesis" << std::endl;
                        return false;
                    }
                    itr += 1;
                    return true;
                }
            }
            break;
            
        default:
            break;
    }
    
    std::cout << "Error: Unexpected token in value: " << currentToken.value << std::endl;
    return false;
}

bool parser::parse(const std::vector<Token>& tokens, astNode* parentNode) {
    std::cout << "Starting parsing process..." << std::endl;

    // Define the function map
    std::unordered_map<std::string, std::function<bool(const std::vector<Token>& tokens, astNode* parentNode)>> functionMap = {
        {"SELECT", std::bind(&parser::parseSelect, this, std::placeholders::_1, std::placeholders::_2)},
        {"INSERT", std::bind(&parser::parseInsert, this, std::placeholders::_1, std::placeholders::_2)},
        {"UPDATE", std::bind(&parser::parseUpdate, this, std::placeholders::_1, std::placeholders::_2)},
        {"DELETE", std::bind(&parser::parseDelete, this, std::placeholders::_1, std::placeholders::_2)},
        {"CREATE", std::bind(&parser::parseCreate, this, std::placeholders::_1, std::placeholders::_2)}
    };

    if (tokens.empty()) {
        std::cout << "No tokens to parse." << std::endl;
        return false;
    }

    const Token& firstToken = tokens[0];
    std::string firstTokenValueUpper = firstToken.value;
    std::transform(firstTokenValueUpper.begin(), firstTokenValueUpper.end(), firstTokenValueUpper.begin(), ::toupper);
    auto it = functionMap.find(firstTokenValueUpper);
    if (it != functionMap.end()) {
        std::cout << "Identified command: " << firstTokenValueUpper << std::endl;
        return it->second(tokens, parentNode);
    } else {
        std::cout << "Error: Unrecognized command '" << firstToken.value << "'" << std::endl;
        return false;
    }
    std::cout << "Parsing completed." << std::endl;
}