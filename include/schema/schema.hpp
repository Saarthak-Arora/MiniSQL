#pragma once
#ifndef SCHEMA_HPP  
#define SCHEMA_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>

// Column metadata
struct Column {
    std::string name;
    std::string datatype;
    bool isPrimaryKey = false;
    bool isUnique = false;
    bool isNotNull = false;
};

// Table metadata
struct Table {
    std::string name;
    std::unordered_map<std::string, Column> columns; // Column name -> Column metadata
    std::unordered_map<std::string, std::string> foreignKeys; // Column name -> Referenced table.column
};

// Database schema
class DatabaseSchema {
private:
    std::unordered_map<std::string, Table> tables; // Table name -> Table metadata

public:
    // Add a table to the schema
    void addTable(const Table& table);

    // Check if a table exists
    bool tableExists(const std::string& tableName) const;

    // Get table metadata
    const Table* getTable(const std::string& tableName) const;

    // Validate a column's constraints
    bool validateColumnConstraints(const std::string& tableName, const std::string& columnName, const std::string& value) const;

    // Print the schema (for debugging)
    void printSchema() const;
};

#endif // SCHEMA_HPP