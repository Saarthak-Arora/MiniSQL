#include <iostream>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include "../../include/parser/lexer.hpp"

// Define keywords, operators, punctuation, and datatypes
const std::unordered_set<std::string> keywords = {
    "SELECT", "FROM", "WHERE", "INSERT", "INTO", "VALUES", "UPDATE", "SET", "DELETE",
    "CREATE", "TABLE", "DROP", "ALTER", "ADD", "NULL", "PRIMARY", "FOREIGN", "NOT_NULL",
    "DEFAULT", "UNIQUE", "ON", "IS", "JOIN", "INNER", "LEFT", "RIGHT", "FULL", "OUTER"
};

const std::unordered_set<std::string> operators = {
    "+", "-", "*", "/", "=", "<", ">", "<=", ">=", "<>", "!=", "&&", "||", "!","EXISTS", "AND", "OR", "NOT", "IN", "LIKE", "BETWEEN", "ALL", "ANY"
};

const std::unordered_set<std::string> punctuation = {",", ";", "(", ")", "."};

const std::unordered_set<std::string> datatypes = {
    "INT", "VARCHAR", "CHAR", "TEXT", "FLOAT", "DOUBLE", "DATE", "BOOLEAN"
};

// Define multi-word constraints
const std::unordered_map<std::string, std::string> multi_word_constraints = {
    {"PRIMARY KEY", "PRIMARY_KEY"},
    {"NOT NULL", "NOT_NULL"},
    {"FOREIGN KEY", "FOREIGN_KEY"}
};

// Helper function: Convert a string to uppercase
std::string to_upper(const std::string& str) {
    std::string upper_str = str;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(), ::toupper);
    return upper_str;
}

// Check if a string is a keyword
bool is_keyword(const std::string& str) {
    return keywords.find(to_upper(str)) != keywords.end();
}

// Check if a string is an operator
bool is_operator(const std::string& str) {
    return operators.find(str) != operators.end();
}

// Check if a string is punctuation
bool is_punctuation(const std::string& str) {
    return punctuation.find(str) != punctuation.end();
}

// Check if a string is a datatype
bool is_datatype(const std::string& str) {
    return datatypes.find(to_upper(str)) != datatypes.end();
}

// Check if a string is a valid identifier
bool is_identifier(const std::string& str) {
    if (str.empty() || is_keyword(str)) return false;
    if (!isalpha(str[0]) && str[0] != '_') return false;
    for (char ch : str) {
        if (!isalnum(ch) && ch != '_') return false;
    }
    return true;
}

// Check if a string is a number
bool is_number(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

// Check if a string is a string literal
bool is_string_literal(const std::string& str) {
    return str.size() >= 2 && str.front() == '\'' && str.back() == '\'';
}

// Classify a token
std::pair<std::string, std::string> classify_token(const std::string& token) {
    if (is_keyword(token)) {
        return {"keyword", to_upper(token)};
    } else if (is_operator(token)) {
        return {"operator", token};
    }
     else if (is_punctuation(token)) {
        return {"punctuation", token};
    } else if (is_number(token)) {
        return {"number", token};
    } else if (is_string_literal(token)) {
        return {"string", token};
    } else if (is_datatype(token)) {
        return {"datatype", to_upper(token)};
    } 
    else if (is_identifier(token)) {
        return {"identifier", token};
    } else {
        return {"unknown", token};
    }
}

// Tokenize the input query
std::vector<std::pair<std::string, std::string>> lexer::getlexer(const std::string& input) {
    std::vector<std::pair<std::string, std::string>> tokens;
    std::string current_token;

    for (size_t i = 0; i < input.size(); ++i) {
        char ch = input[i];

        // Handle whitespace
        if (isspace(ch)) {
            if (!current_token.empty()) {
                tokens.push_back(classify_token(current_token));
                current_token.clear();
            }
            continue;
        }

        // Handle punctuation
        if (is_punctuation(std::string(1, ch))) {
            if (!current_token.empty()) {
                tokens.push_back(classify_token(current_token));
                current_token.clear();
            }
            tokens.push_back({"punctuation", std::string(1, ch)});
            continue;
        }

        // Handle operators
        if (is_operator(std::string(1, ch))) {
            if (!current_token.empty()) {
                tokens.push_back(classify_token(current_token));
                current_token.clear();
            }
            tokens.push_back({"operator", std::string(1, ch)});
            continue;
        }

        // Accumulate characters for identifiers, numbers, etc.
        current_token += ch;
    }

    // Add the last token if any
    if (!current_token.empty()) {
        tokens.push_back(classify_token(current_token));
    }

    // Combine multi-word constraints
    std::vector<std::pair<std::string, std::string>> combined_tokens;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i + 1 < tokens.size() && tokens[i].first == "keyword" && tokens[i + 1].first == "keyword") {
            std::string combined = tokens[i].second + " " + tokens[i + 1].second;
            if (multi_word_constraints.find(combined) != multi_word_constraints.end()) {
                combined_tokens.push_back({"keyword", multi_word_constraints.at(combined)});
                i++; // Skip the next token
                continue;
            }
        }
        combined_tokens.push_back(tokens[i]);
    }

    return combined_tokens;
}
