#pragma once
#include <string>
#include <vector>
#include "../common/token.hpp"

class lexer {
public:
    // Modern interface using Token struct
    std::vector<Token> tokenize(const std::string& input);
    
    // Legacy interface for backward compatibility
    std::vector<std::pair<std::string, std::string>> getlexer(const std::string& cmd);

private:
    // Helper methods for better organization
    Token classifyToken(const std::string& tokenStr, size_t position = 0, size_t line = 1, size_t column = 1);
    bool isValidIdentifier(const std::string& str);
    bool isNumeric(const std::string& str);
    bool isFloatingPoint(const std::string& str);
    bool isDateFormat(const std::string& str);
    bool isStringLiteral(const std::string& str);
    
    // Multi-word constraint handling
    std::pair<bool, std::string> checkMultiWordConstraint(const std::string& currentToken, const std::string& input, size_t currentPos);
};