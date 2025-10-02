#include <iostream>
#include <string>
#include <sstream>  // for std::ostringstream
#include "include/parser/lexer.hpp"
#include "include/parser/parser.hpp"
#include "include/parser/structQuery.hpp"

void print(astNode* node, int depth = 0) {
    if (!node) return;
    std::string indent(depth * 2, ' ');
    std::cout << indent << node->nodeType << ": " << node->value << std::endl;
    for (auto child : node->children) {
        print(child, depth + 1);
    }
}

int main() {
    std::cout << "welcome to your database center" << std::endl;
    std::cout << "This is a simple database program." << std::endl;
    std::cout << "You can use this program to manage your data." << std::endl;

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

        print(root);
        delete root; // Clean up the AST to prevent memory leaks

        std::cout << "-----------------------------------" << std::endl;
    }

    return 0;
}
