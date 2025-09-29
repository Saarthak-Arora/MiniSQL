#pragma once
#include <string>
#include <vector>
#include <utility>
#include "structQuery.hpp"

class parser{
    public:
        size_t i = 0;
        std::vector<std::string> commandTypes = {"SELECT", "INSERT", "UPDATE", "DELETE", "CREATE", "DROP", "ALTER"};
        bool isToken(const std::pair<std::string, std::string>& token, const std::string& type, const std::string& value);
        bool handleSubquery(const std::vector<std::pair<std::string, std::string>>& tokens, queryStructure &parentQuery);
        void parseSelect(const std::vector<std::pair<std::string, std::string>>& tokens, queryStructure &storedQuery);
        void parseInsert(const std::vector<std::pair<std::string, std::string>>& tokens, queryStructure &storedQuery);
        void parseUpdate(const std::vector<std::pair<std::string, std::string>>& tokens, queryStructure &storedQuery);
        void parseDelete(const std::vector<std::pair<std::string, std::string>>& tokens, queryStructure &storedQuery);
        void parseCreate(const std::vector<std::pair<std::string, std::string>>& tokens, queryStructure &storedQuery); 
        void parse(const std::vector<std::pair<std::string, std::string>>& tokens, queryStructure &storedQuery);
};


