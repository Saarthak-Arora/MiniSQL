#include <iostream>
#include <string>
#include <algorithm>
#include "../../include/parser/lexer.hpp"

std::vector<std::string> types = {"string", "char", "number", "identifier", "operator", "keyword", "punctuation"};
std::string keywords[] = {"SELECT", "FROM", "WHERE", "INSERT", "INTO", "VALUES", "UPDATE", "SET", "DELETE", "CREATE", "TABLE", "DROP", "ALTER", "ADD", "AND", "OR", "NOT", "NULL", "IN","PRIMARY", "FOREIGN", "NOT_NULL", "DEFAULT", "UNIQUE" };
std::vector<std::string> operators = {"+", "-", "*", "/", "=", "<", ">", "<=", ">=", "<>", "!=", "&&", "||", "!"};
std::vector<std::string> punctuation = {",", ";", "(", ")", "."};
std::vector<std::string> whitespace = {" ", "\t", "\n"};
std::vector<std::string> datatypes = {"INT", "VARCHAR", "CHAR", "TEXT", "FLOAT", "DOUBLE", "DATE", "BOOLEAN"};

bool is_keyword(const std::string& str) {
    std::string upper_str = str;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(), ::toupper);
    for (const auto& kw : keywords) {
        if (upper_str== kw) {
            return true;
        }
    }
    return false;
}
bool is_operator(const std::string& str) {
    for (const auto& op : operators) {
        if (str == op) {
            return true;
        }
    }
    return false;
}
bool is_punctuation(const std::string& str) {
    for (const auto& punc : punctuation) {
        if (str == punc) {
            return true;
        }
    }
    return false;
}
bool is_whitespace(const std::string& str) {
    for (const auto& ws : whitespace) {
        if (str == ws) {
            return true;
        }
    }
    return false;
}
bool is_number(const std::string& str) {
    for (const auto& ch : str) {
        if (!isdigit(ch)) {
            return false;
        }
    }
    return true;
}
bool is_identifier(const std::string& str) {
    if (str.empty() || !(isalpha(str[0]) || str[0] == '_')) {
        return false;
    }
    for (const auto& ch : str) {
        if (!(isalnum(ch) || ch == '_')) {
            return false;
        }
    }
    return true;
}
bool is_string(const std::string& str) {
    return str.size() >= 2 && str.front() == '\"' && str.back() == '\"';
}
bool is_char(const std::string& str) {
    return str.size() == 3 && str.front() == '\'' && str.back() == '\'';
}
bool is_double(const std::string& str) {
    bool decimal_point_found = false;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '.') {
            if (decimal_point_found) {
                return false; // More than one decimal point
            }
            decimal_point_found = true;
        } else if (!isdigit(str[i])) {
            return false; // Non-digit character found
        }
    }
    return decimal_point_found; // Must have at least one decimal point to be a double
}
bool is_datatype(const std::string& str) {
    std::string upper_str = str;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(), ::toupper);
    for (const auto& dt : datatypes) {
        if (upper_str == dt) {
            return true;
        }
    }
    return false;
}
std::pair<std::string, std::string> classify_token(const std::string& token) {
    if (is_keyword(token)) {
        std::string upper_str = token;
        std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(), ::toupper);
        return {"keyword", upper_str};
    } else if (is_operator(token)) {
        return {"operator", token};
    } else if(is_datatype(token)){
        return {"datatype", token};
    }else if (is_punctuation(token)) {
        return {"punctuation", token};
    } else if (is_number(token)) {
        return {"number", token};
    } else if (is_string(token)) {
        return {"string", token};
    } else if (is_char(token)) {
        return {"char", token};
    } else if (is_identifier(token)) {
        return {"identifier", token};
    } else if(is_double(token)){
        return {"double", token};
    }
    else {
        return {"unknown", token};
    }
}

std::vector<std::pair<std::string, std::string>> lexer::getlexer(const std::string& cmd) {
    std::vector<std::pair<std::string, std::string>> tokens;
    std::string current_token;
    bool in_string = false;
    bool in_char = false;

    for (size_t i = 0; i < cmd.size(); ++i) {
        char ch = cmd[i];

        if(ch == '\r' || ch == '\t' || ch == '\n') continue; 

        // Handle string literals
        if (ch == '"' || ch == '\'') {
            if (in_string || in_char) {
                current_token += ch;
                tokens.push_back({"string", current_token});
                current_token.clear();
                in_string = in_char = false;
            } else {
                if (!current_token.empty()) {
                    tokens.push_back(classify_token(current_token));
                    current_token.clear();
                }
                current_token += ch;
                in_string = (ch == '"');
                in_char = (ch == '\'');
            }
            continue;
        }

        // Handle multi-character operators
        if (ch == '=' || ch == '<' || ch == '>' || ch == '!') {
            if (!current_token.empty()) {
                tokens.push_back(classify_token(current_token));
                current_token.clear();
            }

            // Check for multi-character operators
            if (i + 1 < cmd.size() && (cmd[i + 1] == '=')) {
                tokens.push_back({"operator", std::string(1, ch) + cmd[i + 1]});
                ++i; // Skip the next character
            } else {
                tokens.push_back({"operator", std::string(1, ch)});
            }
            continue;
        }

        // Handle whitespace
        if (std::isspace(ch)) {
            if (!current_token.empty()) {
                tokens.push_back(classify_token(current_token));
                current_token.clear();
            }
            continue;
        }

        // Handle punctuation
        if (ch == ',' || ch == ';' || ch == '(' || ch == ')') {
            if (!current_token.empty()) {
                tokens.push_back(classify_token(current_token));
                current_token.clear();
            }
            tokens.push_back({"punctuation", std::string(1, ch)});
            continue;
        }

        if(is_datatype(current_token) && !(ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z')){
            if (!current_token.empty()) {
                tokens.push_back(classify_token(current_token));
                current_token.clear();
            }
            tokens.push_back({"punctuation", std::string(1, ch)});    
            continue;
        }

        // Accumulate characters for identifiers, numbers, etc.
        current_token += ch;
    }

    
    // Add the last token if any
    if (!current_token.empty()) {
        tokens.push_back(classify_token(current_token));
    }
    return tokens;
}
