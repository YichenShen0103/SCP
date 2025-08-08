#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include "dfa.h"
#include "token.h"

class Lexer
{
public:
    Lexer();
    ~Lexer() = default;

    // Set input string for streaming tokenization
    void setInput(const std::string &input);

    // Get next token from the input stream
    // Returns std::nullopt when no more tokens available
    std::optional<Token> next();

    // Check if there are more tokens to read
    bool hasNext() const;

    // Reset the lexer to start from beginning
    void reset();

    // Tokenize the input string and return a vector of tokens (legacy method)
    std::vector<Token> tokenize(const std::string &input);

private:
    // Input string and current position for streaming
    std::string input_;
    size_t current_pos_;

    // DFA instances for different token types
    std::unique_ptr<DeterministicFiniteAutomata> number_dfa_;
    std::unique_ptr<DeterministicFiniteAutomata> identifier_dfa_;

    // Vector to store all DFAs for easier iteration
    std::vector<DeterministicFiniteAutomata *> dfa_list_;

    // Vector to track which DFAs are still active during tokenization
    std::vector<bool> survival_list_;

    // Initialize the DFAs with their respective transitions
    void initializeDFAs();

    // Setup NUMBER DFA transitions (^[0-9]+$)
    void setupNumberDFA();

    // Setup IDENTIFIER DFA transitions (^[A-Za-z_][A-Za-z0-9_]*$)
    void setupIdentifierDFA();

    // Convert DFA token class name to TokenType enum
    TokenType getTokenType(const std::string &tokenClass);

    // Skip whitespace characters at current position
    void skipWhitespace();

    // Get next token starting from current position
    bool getNextToken(Token &token);
};
