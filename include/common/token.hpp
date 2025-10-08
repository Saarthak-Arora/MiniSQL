#pragma once
#include <string>

// Enum for token types - covers all possible token classifications
enum class TokenType {
    // Core SQL tokens
    KEYWORD,        // SELECT, FROM, WHERE, etc.
    IDENTIFIER,     // table names, column names, variables
    OPERATOR,       // +, -, *, /, =, <, >, etc.
    DATATYPE,       // INT, VARCHAR, CHAR, etc.
    CONSTRAINT,     // PRIMARY KEY, NOT NULL, etc.
    
    // Literals
    NUMBER,         // Integer numbers: 123, -456
    DOUBLE,         // Floating point: 3.14, -2.5
    STRING,         // String literals: 'hello', 'world'
    DATE,           // Date literals: '2023-12-25'
    
    // Structure
    PUNCTUATION,    // ,;().
    WHITESPACE,     // spaces, tabs, newlines
    
    // Special cases
    ERROR,          // malformed tokens, unclosed strings
    UNKNOWN,        // unrecognized tokens
    EOF_TOKEN       // end of input
};

// Convert TokenType enum to string for debugging/output
inline std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KEYWORD:     return "keyword";
        case TokenType::IDENTIFIER:  return "identifier";
        case TokenType::OPERATOR:    return "operator";
        case TokenType::DATATYPE:    return "datatype";
        case TokenType::CONSTRAINT:  return "constraint";
        case TokenType::NUMBER:      return "number";
        case TokenType::DOUBLE:      return "double";
        case TokenType::STRING:      return "string";
        case TokenType::DATE:        return "date";
        case TokenType::PUNCTUATION: return "punctuation";
        case TokenType::WHITESPACE:  return "whitespace";
        case TokenType::ERROR:       return "error";
        case TokenType::UNKNOWN:     return "unknown";
        case TokenType::EOF_TOKEN:   return "eof";
        default:                     return "invalid";
    }
}

// Convert string to TokenType enum for consistency
inline TokenType stringToTokenType(const std::string& typeStr) {
    if (typeStr == "keyword") return TokenType::KEYWORD;
    if (typeStr == "identifier") return TokenType::IDENTIFIER;
    if (typeStr == "operator") return TokenType::OPERATOR;
    if (typeStr == "datatype") return TokenType::DATATYPE;
    if (typeStr == "constraint") return TokenType::CONSTRAINT;
    if (typeStr == "number") return TokenType::NUMBER;
    if (typeStr == "double") return TokenType::DOUBLE;
    if (typeStr == "string") return TokenType::STRING;
    if (typeStr == "date") return TokenType::DATE;
    if (typeStr == "punctuation") return TokenType::PUNCTUATION;
    if (typeStr == "whitespace") return TokenType::WHITESPACE;
    if (typeStr == "error") return TokenType::ERROR;
    if (typeStr == "eof") return TokenType::EOF_TOKEN;
    return TokenType::UNKNOWN;
}

// Token structure for better organization
struct Token {
    TokenType type;
    std::string value;
    std::string original;  // original case/format
    size_t position;       // position in input string
    size_t line;           // line number (for error reporting)
    size_t column;         // column number (for error reporting)
    
    Token() : type(TokenType::UNKNOWN), value(""), original(""), position(0), line(1), column(1) {}
    
    Token(TokenType t, const std::string& val, size_t pos = 0, size_t ln = 1, size_t col = 1) 
        : type(t), value(val), original(val), position(pos), line(ln), column(col) {}
    
    Token(TokenType t, const std::string& val, const std::string& orig, size_t pos = 0, size_t ln = 1, size_t col = 1) 
        : type(t), value(val), original(orig), position(pos), line(ln), column(col) {}
    
    // Convenience methods
    bool isKeyword() const { return type == TokenType::KEYWORD; }
    bool isIdentifier() const { return type == TokenType::IDENTIFIER; }
    bool isOperator() const { return type == TokenType::OPERATOR; }
    bool isLiteral() const { 
        return type == TokenType::NUMBER || type == TokenType::DOUBLE || 
               type == TokenType::STRING || type == TokenType::DATE; 
    }
    bool isError() const { return type == TokenType::ERROR; }
    bool isEOF() const { return type == TokenType::EOF_TOKEN; }
};