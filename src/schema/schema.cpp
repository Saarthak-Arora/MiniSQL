#include "../../include/schema/schema.hpp"
#include <iostream>
#include <algorithm>

// Add a table to the schema
void DatabaseSchema::addTable(const Table& table) {
    if (tables.find(table.name) != tables.end()) {
        std::cerr << "Error: Table '" << table.name << "' already exists." << std::endl;
        return;
    }
    tables[table.name] = table;
    std::cout << "Table '" << table.name << "' added to the schema." << std::endl;
}

// Check if a table exists
bool DatabaseSchema::tableExists(const std::string& tableName) const {
    return tables.find(tableName) != tables.end();
}

// Get table metadata
const Table* DatabaseSchema::getTable(const std::string& tableName) const {
    auto it = tables.find(tableName);
    if (it != tables.end()) {
        return &it->second;
    }
    return nullptr;
}

// Validate a column's constraints
bool DatabaseSchema::validateColumnConstraints(const std::string& tableName, const std::string& columnName, const std::string& value) const {
    const Table* table = getTable(tableName);
    if (!table) {
        std::cerr << "Error: Table '" << tableName << "' does not exist." << std::endl;
        return false;
    }

    auto colIt = table->columns.find(columnName);
    if (colIt == table->columns.end()) {
        std::cerr << "Error: Column '" << columnName << "' does not exist in table '" << tableName << "'." << std::endl;
        return false;
    }

    const Column& column = colIt->second;

    // Check NOT NULL constraint
    if (column.isNotNull && value.empty()) {
        std::cerr << "Error: Column '" << columnName << "' cannot be NULL." << std::endl;
        return false;
    }

    // Check UNIQUE constraint (this would require additional logic to check existing data)
    if (column.isUnique) {
        std::cerr << "Error: UNIQUE constraint validation not implemented yet." << std::endl;
    }

    // Check datatype (basic validation)
    if (column.datatype == "INT" && !std::all_of(value.begin(), value.end(), ::isdigit)) {
        std::cerr << "Error: Invalid value '" << value << "' for column '" << columnName << "' of type INT." << std::endl;
        return false;
    }


    return true;
}

// Print the schema (for debugging)
void DatabaseSchema::printSchema() const {
    for (const auto& [tableName, table] : tables) {
        std::cout << "Table: " << tableName << std::endl;
        for (const auto& [columnName, column] : table.columns) {
            std::cout << "  Column: " << columnName
                      << " (Type: " << column.datatype
                      << ", PrimaryKey: " << column.isPrimaryKey
                      << ", Unique: " << column.isUnique
                      << ", NotNull: " << column.isNotNull << ")" << std::endl;
        }
        for (const auto& [colName, ref] : table.foreignKeys) {
            std::cout << "  ForeignKey: " << colName << " -> " << ref << std::endl;
        }
    }
}