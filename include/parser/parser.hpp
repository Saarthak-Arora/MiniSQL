#pragma once
#include <string>
#include <vector>
#include <utility>
#include "ast.hpp"
#include "../common/token.hpp"
#include "../../utils/Iterator.hpp"

class parser{
    public:
        parser() = default;
        ~parser() = default;
        Iterator itr;
        
        // Basic parsing methods
        bool isToken(const Token& token, TokenType type, const std::string& value = "");
        bool handleSubquery(const std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseSelect(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseInsert(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseUpdate(const  std::vector<Token>& tokens,  astNode* parentNode = nullptr);
        bool parseDelete(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseCreate(const  std::vector<Token>& tokens,  astNode* parentNode = nullptr); 
        bool parse(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        
        // Enhanced SELECT parsing methods
        bool parseSelectList(const std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseSelectExpression(const std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseAggregateFunction(const std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseFromClause(const std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseTableReference(const std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseJoinClause(const std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseGroupBy(const std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseHaving(const std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseOrderBy(const std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseLimit(const std::vector<Token>& tokens, astNode* parentNode = nullptr);
        
        // Enhanced condition parsing methods
        bool parseCondition(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseLogicalExpression(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseLogicalTerm(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseLogicalFactor(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseComparisonExpression(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseSimpleComparison(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseLikeExpression(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseInExpression(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseBetweenExpression(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseIsExpression(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseExistsExpression(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
        bool parseValue(const  std::vector<Token>& tokens, astNode* parentNode = nullptr);
};


