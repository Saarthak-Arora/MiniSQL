#include <string>
#include <algorithm>
#include <unordered_map>
#include <cctype>
#include "../../include/parser/lexer.hpp"
#include "../../include/common/token.hpp"

const std::unordered_map<std::string, TokenType> keyword_map = {
    {"SELECT", TokenType::KEYWORD}, {"FROM", TokenType::KEYWORD}, {"WHERE", TokenType::KEYWORD}, 
    {"INSERT", TokenType::KEYWORD}, {"INTO", TokenType::KEYWORD}, {"VALUES", TokenType::KEYWORD}, 
    {"UPDATE", TokenType::KEYWORD}, {"SET", TokenType::KEYWORD}, {"DELETE", TokenType::KEYWORD}, 
    {"CREATE", TokenType::KEYWORD}, {"TABLE", TokenType::KEYWORD}, {"DROP", TokenType::KEYWORD}, 
    {"ALTER", TokenType::KEYWORD}, {"ADD", TokenType::KEYWORD}, {"NULL", TokenType::KEYWORD}, 
    {"ON", TokenType::KEYWORD}, {"IS", TokenType::KEYWORD}, {"JOIN", TokenType::KEYWORD}, 
    {"INNER", TokenType::KEYWORD}, {"LEFT", TokenType::KEYWORD}, {"RIGHT", TokenType::KEYWORD}, 
    {"FULL", TokenType::KEYWORD}, {"OUTER", TokenType::KEYWORD}, {"AS", TokenType::KEYWORD}, 
    {"BY", TokenType::KEYWORD}, {"GROUP", TokenType::KEYWORD}, {"ORDER", TokenType::KEYWORD}, 
    {"HAVING", TokenType::KEYWORD}, {"DISTINCT", TokenType::KEYWORD}, {"LIMIT", TokenType::KEYWORD}, 
    {"OFFSET", TokenType::KEYWORD}, {"REFERENCES", TokenType::KEYWORD}, {"EXISTS", TokenType::KEYWORD}, 
    {"AND", TokenType::KEYWORD}, {"OR", TokenType::KEYWORD}, {"NOT", TokenType::KEYWORD}, 
    {"IN", TokenType::KEYWORD}, {"LIKE", TokenType::KEYWORD}, {"BETWEEN", TokenType::KEYWORD}, 
    {"ALL", TokenType::KEYWORD}, {"ANY", TokenType::KEYWORD}
};

const std::unordered_map<std::string, TokenType> operator_map = {
    {"+", TokenType::OPERATOR}, {"-", TokenType::OPERATOR}, {"*", TokenType::OPERATOR}, 
    {"/", TokenType::OPERATOR}, {"=", TokenType::OPERATOR}, {"<", TokenType::OPERATOR}, 
    {">", TokenType::OPERATOR}, {"<=", TokenType::OPERATOR}, {">=", TokenType::OPERATOR}, 
    {"<>", TokenType::OPERATOR}, {"!=", TokenType::OPERATOR}, {"&&", TokenType::OPERATOR}, 
    {"||", TokenType::OPERATOR}, {"!", TokenType::OPERATOR}
};

const std::unordered_map<std::string, TokenType> punctuation_map = {
    {",", TokenType::PUNCTUATION}, {";", TokenType::PUNCTUATION}, {"(", TokenType::PUNCTUATION}, 
    {")", TokenType::PUNCTUATION}, {".", TokenType::PUNCTUATION}
};

const std::unordered_map<std::string, TokenType> datatype_map = {
    {"NUMBER", TokenType::DATATYPE}, {"INT", TokenType::DATATYPE}, {"VARCHAR", TokenType::DATATYPE}, 
    {"CHAR", TokenType::DATATYPE}, {"TEXT", TokenType::DATATYPE}, {"FLOAT", TokenType::DATATYPE}, 
    {"DOUBLE", TokenType::DATATYPE}, {"DATE", TokenType::DATATYPE}, {"BOOLEAN", TokenType::DATATYPE}, 
    {"STRING", TokenType::DATATYPE}
};

const std::unordered_map<std::string, TokenType> constraint_map = {
    {"PRIMARY KEY", TokenType::CONSTRAINT}, {"NOT NULL", TokenType::CONSTRAINT}, 
    {"FOREIGN KEY", TokenType::CONSTRAINT}, {"UNIQUE", TokenType::CONSTRAINT}, 
    {"CHECK", TokenType::CONSTRAINT}, {"DEFAULT", TokenType::CONSTRAINT}
};

inline std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool lexer::isValidIdentifier(const std::string& str) {
    if (str.empty()) return false;
    
    if (keyword_map.find(toUpper(str)) != keyword_map.end()) return false;
    
    if (!std::isalpha(str[0]) && str[0] != '_') return false;
    
    return std::all_of(str.begin() + 1, str.end(), 
                      [](char c) { return std::isalnum(c) || c == '_'; });
}

bool lexer::isNumeric(const std::string& str) {
    if (str.empty()) return false;
    size_t start = (str[0] == '-' || str[0] == '+') ? 1 : 0;
    if (start >= str.size()) return false;
    return std::all_of(str.begin() + start, str.end(), ::isdigit);
}

bool lexer::isFloatingPoint(const std::string& str) {
    if (str.empty()) return false;
    bool hasDecimal = false;
    size_t start = (str[0] == '-' || str[0] == '+') ? 1 : 0;
    if (start >= str.size()) return false;
    
    for (size_t i = start; i < str.size(); ++i) {
        if (str[i] == '.') {
            if (hasDecimal) return false; 
            hasDecimal = true;
        } else if (!std::isdigit(str[i])) {
            return false;
        }
    }
    return hasDecimal; 
}

bool lexer::isDateFormat(const std::string& str) {
    if (str.size() != 10 || str[4] != '-' || str[7] != '-') return false;

    std::string year = str.substr(0, 4);
    std::string month = str.substr(5, 2);
    std::string day = str.substr(8, 2);

    if (!std::all_of(year.begin(), year.end(), ::isdigit) ||
        !std::all_of(month.begin(), month.end(), ::isdigit) ||
        !std::all_of(day.begin(), day.end(), ::isdigit)) {
        return false;
    }

    int monthInt = std::stoi(month);
    int dayInt = std::stoi(day);

    if (monthInt < 1 || monthInt > 12) return false;
    if (dayInt < 1 || dayInt > 31) return false; 

    return true;
}

bool lexer::isStringLiteral(const std::string& str) {
    if (str.size() < 2) return false;
    if (str.front() != '\'' || str.back() != '\'') return false;
    
    std::string content = str.substr(1, str.size() - 2);
    return !isDateFormat(content); 
}


std::pair<bool, std::string> lexer::checkMultiWordConstraint(const std::string& currentToken, const std::string& input, size_t currentPos) {
    std::string upperCurrent = toUpper(currentToken);
    
    if (upperCurrent == "PRIMARY" || upperCurrent == "NOT" || upperCurrent == "FOREIGN") {
        
        size_t i = currentPos;
        while (i < input.size() && std::isspace(input[i])) i++;
        
        std::string nextToken;
        while (i < input.size() && !std::isspace(input[i]) && 
               punctuation_map.find(std::string(1, input[i])) == punctuation_map.end()) {
            nextToken += input[i];
            i++;
        }
        
        if (!nextToken.empty()) {
            std::string multiWord = upperCurrent + " " + toUpper(nextToken);
            if (constraint_map.find(multiWord) != constraint_map.end()) {
                return {true, multiWord};
            }
        }
    }
    
    return {false, ""};
}

Token lexer::classifyToken(const std::string& tokenStr, size_t position, size_t line, size_t column) {
    if (tokenStr.empty()) {
        return Token(TokenType::UNKNOWN, "", position, line, column);
    }
    
    std::string upperToken = toUpper(tokenStr);
    
    
    auto operatorIt = operator_map.find(tokenStr);
    if (operatorIt != operator_map.end()) {
        return Token(TokenType::OPERATOR, tokenStr, position, line, column);
    }
     
    auto punctIt = punctuation_map.find(tokenStr);
    if (punctIt != punctuation_map.end()) {
        return Token(TokenType::PUNCTUATION, tokenStr, position, line, column);
    }
    
    auto constraintIt = constraint_map.find(upperToken);
    if (constraintIt != constraint_map.end()) {
        return Token(TokenType::CONSTRAINT, upperToken, tokenStr, position, line, column);
    }
    
    auto keywordIt = keyword_map.find(upperToken);
    if (keywordIt != keyword_map.end()) {
        return Token(TokenType::KEYWORD, upperToken, tokenStr, position, line, column);
    }
    
    auto datatypeIt = datatype_map.find(upperToken);
    if (datatypeIt != datatype_map.end()) {
        return Token(TokenType::DATATYPE, upperToken, tokenStr, position, line, column);
    }
    
    if (isFloatingPoint(tokenStr)) {
        return Token(TokenType::DOUBLE, tokenStr, position, line, column);
    }
    
    if (isNumeric(tokenStr)) {
        return Token(TokenType::NUMBER, tokenStr, position, line, column);
    }
    
    if (isStringLiteral(tokenStr)) {
        return Token(TokenType::STRING, tokenStr, position, line, column);
    }
    
    if (tokenStr.size() >= 2 && tokenStr.front() == '\'' && tokenStr.back() == '\'') {
        std::string content = tokenStr.substr(1, tokenStr.size() - 2);
        if (isDateFormat(content)) {
            return Token(TokenType::DATE, tokenStr, position, line, column);
        }
    }
    
    if (isValidIdentifier(tokenStr)) {
        return Token(TokenType::IDENTIFIER, tokenStr, position, line, column);
    }
    
    return Token(TokenType::UNKNOWN, tokenStr, position, line, column);
}
    
std::vector<Token> lexer::tokenize(const std::string& input) {
    std::vector<Token> tokens;
    tokens.reserve(input.size() / 4);
    
    std::string currentToken;
    size_t position = 0;
    size_t line = 1;
    size_t column = 1;
    
    for (size_t i = 0; i < input.size(); ++i) {
        char ch = input[i];
        
        if (ch == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        
        if (std::isspace(ch)) {
            if (!currentToken.empty()) {
                // Check for multi-word constraints before classifying
                auto [isMultiWord, constraintStr] = checkMultiWordConstraint(currentToken, input, i);
                if (isMultiWord) {
                    size_t nextTokenStart = i;
                    while (nextTokenStart < input.size() && std::isspace(input[nextTokenStart])) nextTokenStart++;
                    
                    std::string nextToken;
                    size_t nextTokenEnd = nextTokenStart;
                    while (nextTokenEnd < input.size() && !std::isspace(input[nextTokenEnd]) && 
                           punctuation_map.find(std::string(1, input[nextTokenEnd])) == punctuation_map.end()) {
                        nextToken += input[nextTokenEnd];
                        nextTokenEnd++;
                    }
                    
                    if (!nextToken.empty()) {
                        tokens.push_back(Token(TokenType::CONSTRAINT, constraintStr, position, line, column - currentToken.size()));
                        currentToken.clear();
                        i = nextTokenEnd - 1;
                        continue;
                    }
                }
                
                tokens.push_back(classifyToken(currentToken, position, line, column - currentToken.size()));
                currentToken.clear();
            }
            continue;
        }
        
        // Handle multi-character operators
        if (i + 1 < input.size()) {
            std::string twoChar = std::string(1, ch) + input[i + 1];
            if (operator_map.find(twoChar) != operator_map.end()) {
                if (!currentToken.empty()) {
                    tokens.push_back(classifyToken(currentToken, position, line, column - currentToken.size()));
                    currentToken.clear();
                }
                tokens.push_back(Token(TokenType::OPERATOR, twoChar, position, line, column));
                i++; 
                column++;
                continue;
            }
        }
        
        // Handle string literals
        if (ch == '\'') {
            if (!currentToken.empty()) {
                tokens.push_back(classifyToken(currentToken, position, line, column - currentToken.size()));
                currentToken.clear();
            }
            
            std::string stringLiteral(1, ch);
            i++;
            column++;
            
            while (i < input.size() && input[i] != '\'') {
                stringLiteral += input[i];
                if (input[i] == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                i++;
            }
            
            if (i < input.size()) {
                stringLiteral += input[i]; 
                column++;
            } else {
                tokens.push_back(Token(TokenType::ERROR, "unclosed_string_literal", position, line, column));
                continue;
            }
            
            tokens.push_back(classifyToken(stringLiteral, position, line, column - stringLiteral.size()));
            continue;
        }
        
        // Handle numbers (including decimals and negative numbers)
        if (std::isdigit(ch) || 
            (ch == '-' && i + 1 < input.size() && std::isdigit(input[i + 1]) && currentToken.empty()) ||
            (ch == '+' && i + 1 < input.size() && std::isdigit(input[i + 1]) && currentToken.empty()) ||
            (ch == '.' && !currentToken.empty() && std::all_of(currentToken.begin(), currentToken.end(), ::isdigit))) {
            currentToken += ch;
            continue;
        }
        
        // Handle punctuation
        std::string singleChar(1, ch);
        if (punctuation_map.find(singleChar) != punctuation_map.end()) {
            if (!currentToken.empty()) {
                tokens.push_back(classifyToken(currentToken, position, line, column - currentToken.size()));
                currentToken.clear();
            }
            tokens.push_back(Token(TokenType::PUNCTUATION, singleChar, position, line, column));
            continue;
        }
        
        // Handle single-character operators
        if (operator_map.find(singleChar) != operator_map.end()) {
            if (!currentToken.empty()) {
                tokens.push_back(classifyToken(currentToken, position, line, column - currentToken.size()));
                currentToken.clear();
            }
            tokens.push_back(Token(TokenType::OPERATOR, singleChar, position, line, column));
            continue;
        }
        
        // Default: accumulate character
        currentToken += ch;
    }
    
    if (!currentToken.empty()) {
        tokens.push_back(classifyToken(currentToken, position, line, column - currentToken.size()));
    }
    
    return tokens;
}

std::vector<std::pair<std::string, std::string>> lexer::getlexer(const std::string& input) {
    std::vector<Token> modernTokens = tokenize(input);
    std::vector<std::pair<std::string, std::string>> legacyTokens;
    legacyTokens.reserve(modernTokens.size());
    
    for (const auto& token : modernTokens) {
        legacyTokens.emplace_back(tokenTypeToString(token.type), token.value);
    }
    
    return legacyTokens;
}
