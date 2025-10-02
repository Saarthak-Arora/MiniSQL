#pragma once
#include <string>
#include <vector>
#include <utility>
#include "structQuery.hpp"
#include "../ast/ast.hpp"

class parser{
    public:
        std::vector<std::string> commandTypes = {"SELECT", "INSERT", "UPDATE", "DELETE", "CREATE", "DROP", "ALTER"};
        bool isToken(const std::pair<std::string, std::string>& token, const std::string& type, const std::string& value);
        bool handleSubquery(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure &parentQuery, astNode* parentNode = nullptr);
        void parseSelect(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure &storedQuery, astNode* parentNode = nullptr);
        void parseInsert(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure &storedQuery, astNode* parentNode = nullptr);
        void parseUpdate(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i,  queryStructure &storedQuery, astNode* parentNode = nullptr);
        void parseDelete(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure &storedQuery, astNode* parentNode = nullptr);
        void parseCreate(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i,  queryStructure &storedQuery, astNode* parentNode = nullptr); 
        void parseCondition(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure &storedQuery, astNode* parentNode = nullptr);
        void parse(const std::vector<std::pair<std::string, std::string>>& tokens, size_t& i, queryStructure &storedQuery, astNode* parentNode = nullptr);
};


