#pragma once
#include <string>
#include <vector>

class lexer{
    public:
      std::vector<std::pair<std::string, std::string>> getlexer(const std::string& cmd);
};