#include <iostream>
#include <string>
#include <sstream>  // for std::ostringstream
#include "include/parser/lexer.hpp"
#include "include/parser/parser.hpp"
#include "include/parser/structQuery.hpp"
#include "include/schema/schema.hpp" // Include the schema header

int main() {
    std::cout << "welcome to your database center" << std::endl;
    std::cout << "This is a simple database program." << std::endl;
    std::cout << "You can use this program to manage your data." << std::endl;

    // Create the database schema
    DatabaseSchema schema;

    // Add tables and columns to the schema
    Table users = {
        "users",
        {
            {"id", {"id", "INT", true, true, true}}, // Primary key, unique, not null
            {"name", {"name", "VARCHAR", false, false, false}},
            {"age", {"age", "INT", false, false, false}}
        },
        {} // No foreign keys
    };
    schema.addTable(users);

    Table orders = {
        "orders",
        {
            {"order_id", {"order_id", "INT", true, true, true}}, // Primary key, unique, not null
            {"user_id", {"user_id", "INT", false, false, true}}, // Foreign key
            {"amount", {"amount", "DOUBLE", false, false, false}}
        },
        {{"user_id", "users.id"}} // Foreign key: user_id -> users.id
    };
    schema.addTable(orders);

    // Print the schema for debugging
    schema.printSchema();

    while (true) {
        std::cout << "Please enter a command (type 'exit' to quit): " << std::endl;

        std::ostringstream commandStream;
        std::string line;

        // Keep reading lines until we encounter a semicolon
        while (std::getline(std::cin, line)) {
            if(line == "exit" || line == "exit;") {
                commandStream.str(""); // Clear the stream
                commandStream << line;
                break;
            }
            if(line.empty()) continue; // skip empty lines
            commandStream << line << " ";
            if (line.find(';') != std::string::npos) {
                break; // end of query
            }
        }

        std::string command = commandStream.str();

        if (command == "exit" || command == "exit;") {
            std::cout << "Exiting the program. Goodbye!" << std::endl;
            break;
        }

        std::cout << "You entered: " << command << std::endl;

        lexer lex;
        std::vector<std::pair<std::string, std::string>> tokens = lex.getlexer(command);

        std::cout << "Tokens:" << std::endl;
        for (const auto& token : tokens) {
            std::cout << token.first << " : " << token.second << std::endl;
        }

        std::cout << "Command tokenized." << std::endl;
        std::cout << "-----------------------------------" << std::endl;

        parser pars;
        queryStructure storedQuery;
        size_t i = 0;
        astNode* root = new astNode("ROOT", "ROOT");
        pars.parse(tokens, i, storedQuery, root);

        std::cout << "AST Structure:" << std::endl;
        root->print(root);
        std::cout << "Validating AST..." << std::endl;
        root->validateAST(root, schema); // Validate the AST against the schema
        std::cout << "AST validation completed." << std::endl;
        delete root; // Clean up the AST to prevent memory leaks

        std::cout << "-----------------------------------" << std::endl;
    }

    return 0;
}
