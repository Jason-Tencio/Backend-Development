#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "parser.h"
#include "prog2.cpp"
#include "lex.cpp"
#include "lex.h"
#include "GivenParserPart.cpp"
#include <unordered_map>


using namespace std;

enum class Type { INTEGER, REAL, CHARACTER };

// Define struct for variable
struct Variable {
    std::string name;
    Type type;
    int length;
};

// Token structure
struct Token {
    TokenType type;
    string value;
};

// Operator precedence levels
enum class Precedence {
    UNARY,
    EXPONENTIATION,
    MULTIPLICATIVE,
    ADDITIVE,
    RELATIONAL
};

// Operator structure
struct Operator {
    TokenType type;
    Precedence precedence;
    bool rightAssociative;
};
enum class Associativity {
    LEFT_TO_RIGHT,
    RIGHT_TO_LEFT
};

struct OperatorPrecedence {
    int precedence;
    Associativity associativity;
};

std::unordered_map<std::string, OperatorPrecedence> operatorPrecedenceTable = {
    {"+", {4, Associativity::LEFT_TO_RIGHT}},
    {"-", {4, Associativity::LEFT_TO_RIGHT}},
    {"*", {3, Associativity::LEFT_TO_RIGHT}},
    {"/", {3, Associativity::LEFT_TO_RIGHT}},
    {"**", {2, Associativity::RIGHT_TO_LEFT}},
    {"<", {5, Associativity::LEFT_TO_RIGHT}},
    {">", {5, Associativity::LEFT_TO_RIGHT}},
    {"==", {5, Associativity::LEFT_TO_RIGHT}}
};

class Parser {
public:
    Parser(const vector<Token>& tokens) : tokens(tokens), index(0) {}
    void parse() {
        program();
    }

private:
    void program() {
        match(TokenType::PROGRAM);
        match(TokenType::IDENT);
        while (tokens[index].type != TokenType::END) {
            if (tokens[index].type == TokenType::INTEGER ||
                tokens[index].type == TokenType::REAL ||
                tokens[index].type == TokenType::CHARACTER) {
                declaration();
            } else {
                statement();
            }
        }
        match(TokenType::END);
        match(TokenType::PROGRAM);
        match(TokenType::IDENT);
    }

    void declaration() {
        type();
        match(TokenType::IDENT);
        if (tokens[index].type == TokenType::ASSIGN_OP) {
            advance();
            expression();
        }
        while (tokens[index].type == TokenType::COMMA) {
            match(TokenType::COMMA);
            match(TokenType::IDENT);
            if (tokens[index].type == TokenType::ASSIGN_OP) {
                advance();
                expression();
            }
        }
    }

    void type() {
        if (tokens[index].type == TokenType::INTEGER ||
            tokens[index].type == TokenType::REAL ||
            tokens[index].type == TokenType::CHARACTER) {
            advance();
            if (tokens[index].type == TokenType::LBRACKET) {
                match(TokenType::LEN);
                match(TokenType::ASSIGN_OP);
                match(TokenType::CONST_INT);
                match(TokenType::RBRACKET);
            }
        } else {
            throw runtime_error("Invalid type");
        }
    }

    void statement() {
        switch (tokens[index].type) {
            case TokenType::IF:
                ifStatement();
                break;
            case TokenType::PRINT:
                printStatement();
                break;
            default:
                assignmentStatement();
                break;
        }
    }

    void ifStatement() {
        match(TokenType::IF);
        match(TokenType::LPAREN);
        relationalExpression();
        match(TokenType::RPAREN);
        match(TokenType::THEN);
        while (tokens[index].type != TokenType::ELSE &&
               tokens[index].type != TokenType::END) {
            statement();
        }
        if (tokens[index].type == TokenType::ELSE) {
            match(TokenType::ELSE);
            while (tokens[index].type != TokenType::END) {
                statement();
            }
        }
        match(TokenType::END);
        match(TokenType::IF);
    }

    void printStatement() {
        match(TokenType::PRINT);
        if (tokens[index].type == TokenType::PRINT_ALL) {
            advance();
        } else {
            expressionList();
        }
    }

    void assignmentStatement() {
        match(TokenType::IDENT);
        match(TokenType::ASSIGN_OP);
        expression();
    }

    void expressionList() {
        expression();
        while (tokens[index].type == TokenType::COMMA) {
            advance();
            expression();
        }
    }

    void relationalExpression() {
        expression();
        if (tokens[index].type == TokenType::OP) {
            advance();
            expression();
        }
    }

    void expression() {
        parseExpression(Precedence::ADDITIVE);
    }

    void parseExpression(Precedence precedenceLevel) {
        if (precedenceLevel == Precedence::UNARY) {
            if (tokens[index].type == TokenType::OP &&
                (tokens[index].value == "+" || tokens[index].value == "-")) {
                advance();
            }
        }

        parseTermExpression();

        while (tokens[index].type == TokenType::OP &&
               operatorPrecedence(tokens[index].value) >= precedenceLevel) {
            advance();
            parseTermExpression();
        }
    }

    void parseTermExpression() {
        parseSFactor();

        while (tokens[index].type == TokenType::OP &&
               (tokens[index].value == "*" || tokens[index].value == "/")) {
            advance();
            parseSFactor();
        }
    }

    void parseSFactor() {
        if (tokens[index].type == TokenType::OP &&
            (tokens[index].value == "+" || tokens[index].value == "-")) {
            advance();
        }
        parseFactor();
    }

    void parseFactor() {
        switch (tokens[index].type) {
            case TokenType::IDENT:
            case TokenType::CONST_INT:
            case TokenType::CONST_REAL:
            case TokenType::CONST_STRING:
                advance();
                break;
            case TokenType::LPAREN:
                advance();
                parseExpression(Precedence::ADDITIVE);
                match(TokenType::RPAREN);
                break;
            default:
                throw runtime_error("Invalid factor");
        }
    }

    void match(TokenType expected) {
        if (tokens[index].type == expected) {
            advance();
        } else {
            throw runtime_error("Syntax error: unexpected token");
        }
    }

    void advance() {
        index++;
    }

    const vector<Token>& tokens;
    size_t index;

    // Function to determine precedence level of operators
    Precedence operatorPrecedence(const string& op) {
        if (op == "+" || op == "-") {
            return Precedence::ADDITIVE;
        } else if (op == "*" || op == "/") {
            return Precedence::MULTIPLICATIVE;
        } else if (op == "**") {
            return Precedence::EXPONENTIATION;
        } else {
            return Precedence::RELATIONAL;
        }
    }
};

vector<Token> tokenize(const string& code) {
    vector<Token> tokens;
    size_t pos = 0;

    while (pos < code.size()) {
        // Skip whitespace
        while (isspace(code[pos])) {
            pos++;
        }
        // Tokenize based on regular expressions
        // Example: Identifiers
        if (isalpha(code[pos])) {
            size_t start = pos;
            while (isalnum(code[pos]) || code[pos] == '_') {
                pos++;
            }
            string value = code.substr(start, pos - start);
            TokenType type = getTokenType(value);
            tokens.push_back({type, value});
        }
        // Example: Numeric constants
        else if (isdigit(code[pos])) {
            size_t start = pos;
            while (isdigit(code[pos])) {
                pos++;
            }
            string value = code.substr(start, pos - start);
            TokenType type = TokenType::CONST_INT; // Assuming integers for simplicity
            tokens.push_back({type, value});
        }
        // Example: Operators
        else if (isOperator(code[pos])) {
            // Handle operators
            size_t start = pos;
            while (isOperator(code[pos])) {
                pos++;
            }
            string value = code.substr(start, pos - start);
            TokenType type = getTokenType(value);
            tokens.push_back({type, value});
        }
        // Example: Keywords
        else if (isKeyword(code.substr(pos, 6))) { // Check for the longest keyword length
            // Handle keywords
            size_t start = pos;
            while (isKeyword(code.substr(pos, pos - start))) {
                pos++;
            }
            string value = code.substr(start, pos - start);
            TokenType type = getTokenType(value);
            tokens.push_back({type, value});
        }
        // Handle other token types similarly...

        // Handle unrecognized characters or syntax errors
        else {
            throw runtime_error("Unexpected character at position " + to_string(pos));
        }
    }
    return tokens;

}
