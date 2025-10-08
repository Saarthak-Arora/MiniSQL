#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include "include/parser/lexer.hpp"
#include "include/common/token.hpp"

void printTokens(const std::vector<std::pair<std::string, std::string>>& tokens, const std::string& query) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "QUERY: " << query << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    std::cout << std::left << std::setw(15) << "TOKEN TYPE" << " | " << "TOKEN VALUE" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    for (const auto& token : tokens) {
        std::cout << std::left << std::setw(15) << token.first << " | " << token.second << std::endl;
    }
    std::cout << std::string(60, '=') << std::endl;
}

void testLexer() {
    lexer lex;
    
    // Test queries covering different SQL features
    std::vector<std::string> testQueries = {
        // 1. Basic SELECT queries
        "SELECT * FROM users;",
        "SELECT id, name FROM users WHERE age > 25;",
        "SELECT DISTINCT name FROM users;",
        
        // 2. CREATE TABLE with constraints
        "CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(50) NOT NULL);",
        "CREATE TABLE orders (order_id INT PRIMARY KEY, user_id INT FOREIGN KEY);",
        
        // 3. INSERT statements
        "INSERT INTO users (id, name, age) VALUES (1, 'John Doe', 30);",
        "INSERT INTO users VALUES (2, 'Jane Smith', 25);",
        
        // 4. UPDATE statements
        "UPDATE users SET age = 31 WHERE id = 1;",
        "UPDATE users SET name = 'John Updated', age = 32 WHERE id = 1;",
        
        // 5. DELETE statements
        "DELETE FROM users WHERE age < 18;",
        "DELETE FROM users;",
        
        // 6. Operators and comparisons
        "SELECT * FROM users WHERE age >= 18 AND age <= 65;",
        "SELECT * FROM users WHERE name LIKE 'John%' OR age <> 25;",
        "SELECT * FROM users WHERE id IN (1, 2, 3) AND name IS NOT NULL;",
        
        // 7. Numbers and strings
        "SELECT * FROM products WHERE price = 99.99;",
        "SELECT * FROM orders WHERE order_date = '2023-12-25';",
        "SELECT * FROM users WHERE salary = -1500.50;",
        
        // 8. JOIN operations
        "SELECT u.name, o.amount FROM users u INNER JOIN orders o ON u.id = o.user_id;",
        "SELECT * FROM users LEFT JOIN orders ON users.id = orders.user_id;",
        
        // 9. GROUP BY and ORDER BY
        "SELECT COUNT(*) FROM users GROUP BY age ORDER BY age DESC;",
        "SELECT name, MAX(age) FROM users GROUP BY name HAVING COUNT(*) > 1;",
        
        // 10. Complex datatypes
        "CREATE TABLE products (id INT, name VARCHAR(100), price DOUBLE, created_date DATE);",
        
        // 11. Multiple constraints
        "CREATE TABLE employees (id INT PRIMARY KEY, email VARCHAR(100) UNIQUE NOT NULL);",
        
        // 12. Nested conditions
        "SELECT * FROM users WHERE (age > 20 AND age < 30) OR (name = 'Admin');",
        
        // 13. ALTER TABLE
        "ALTER TABLE users ADD COLUMN email VARCHAR(255);",
        "DROP TABLE old_users;",
        
        // 14. Error cases (unclosed strings, unknown tokens)
        "SELECT * FROM users WHERE name = 'unclosed string;",
        "SELECT * FROM users WHERE @@invalid_token = 1;",
    };
    
    std::cout << "\n🚀 TESTING LEXER WITH " << testQueries.size() << " QUERIES" << std::endl;
    std::cout << "==============================================================\n" << std::endl;
    
    for (size_t i = 0; i < testQueries.size(); i++) {
        try {
            std::cout << "TEST " << (i + 1) << "/" << testQueries.size() << ":";
            auto tokens = lex.getlexer(testQueries[i]);
            printTokens(tokens, testQueries[i]);
            
            // Validate that we got some tokens
            if (tokens.empty()) {
                std::cout << "⚠️  WARNING: No tokens generated!" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cout << "❌ ERROR in test " << (i + 1) << ": " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n✅ LEXER TESTING COMPLETED!" << std::endl;
    std::cout << "Check the output above to verify token classification." << std::endl;
}

int main() {
    std::cout << "=== COMPREHENSIVE LEXER TEST ===" << std::endl;
    testLexer();
    
    std::cout << "\n\n=== INTERACTIVE MODE ===" << std::endl;
    std::cout << "Enter SQL queries to test (type 'exit' to quit):" << std::endl;
    
    lexer lex;
    std::string query;
    
    while (true) {
        std::cout << "\nSQL> ";
        std::getline(std::cin, query);
        
        if (query == "exit" || query == "quit") {
            break;
        }
        
        if (query.empty()) {
            continue;
        }
        
        try {
            auto tokens = lex.getlexer(query);
            printTokens(tokens, query);
        } catch (const std::exception& e) {
            std::cout << "❌ ERROR: " << e.what() << std::endl;
        }
    }
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}