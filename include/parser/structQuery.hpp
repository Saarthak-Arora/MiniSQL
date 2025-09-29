#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct queryStructure{
    std::string commandType; // e.g., SELECT, INSERT, UPDATE, DELETE
    std::string tableName;   // Name of the table involved in the query
    std::vector<std::string> columns; // List of columns involved in the query
    std::vector<std::string> values;  // List of values for INSERT or UPDATE
    std::string whereClause; // Conditions for WHERE clause
    std::string additionalInfo; // Any additional information or clauses
    std::vector<queryStructure> subqueries;
    std::vector<std::string> groupBy; // GROUP BY columns
    std::vector<std::string> orderBy; // ORDER BY columns
    std::unordered_map<std::string, std::vector<std::string>> constraints;
// Example: {"id" -> {"PRIMARY KEY"}, "email" -> {"UNIQUE", "NOT NULL"}}
};
