#include "../include/lexer.h"
#include <iostream>

// Alphabet constants
const std::string ID_ALPHABET = "0123456789abcdefghijklmnopqrstuvwxyz"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ_";
const std::string NUMBER_ALPHABET = "0123456789";

Lexer::Lexer() : current_pos_(0)
{
    initializeDFAs();
}

void Lexer::initializeDFAs()
{
    // Create DFA instances
    number_dfa_.reset(new DeterministicFiniteAutomata(
        2, NUMBER_ALPHABET, "NUMBER"));
    identifier_dfa_.reset(new DeterministicFiniteAutomata(
        2, ID_ALPHABET, "IDENTIFIER"));

    // Setup transitions for each DFA
    setupNumberDFA();
    setupIdentifierDFA();

    // Release the DFAs (make them ready for evaluation)
    number_dfa_->Release();
    identifier_dfa_->Release();

    // Setup the DFA list for iteration
    dfa_list_.push_back(number_dfa_.get());
    dfa_list_.push_back(identifier_dfa_.get());

    // Initialize survival list
    survival_list_.resize(dfa_list_.size());
}

void Lexer::setupNumberDFA()
{
    // Set up NUMBER DFA transitions: ^[0-9]+$
    // Transition from state 0 to state 1 on any digit
    for (char c = '0'; c <= '9'; ++c)
    {
        number_dfa_->AddTransition(0, c, 1);
        number_dfa_->AddTransition(1, c, 1); // Stay in accepting state
    }
    number_dfa_->SetFinalState(1); // State 1 is accepting
}

void Lexer::setupIdentifierDFA()
{
    // Set up IDENTIFIER DFA transitions: ^[A-Za-z_][A-Za-z0-9_]*$
    // First character must be letter or underscore
    for (char c = 'a'; c <= 'z'; ++c)
    {
        identifier_dfa_->AddTransition(0, c, 1);
    }
    for (char c = 'A'; c <= 'Z'; ++c)
    {
        identifier_dfa_->AddTransition(0, c, 1);
    }
    identifier_dfa_->AddTransition(0, '_', 1);

    // Subsequent characters can be letters, digits, or underscore
    for (size_t i = 0; i < ID_ALPHABET.size(); ++i)
    {
        identifier_dfa_->AddTransition(1, ID_ALPHABET[i], 1);
    }
    identifier_dfa_->SetFinalState(1); // State 1 is accepting
}

TokenType Lexer::getTokenType(const std::string &tokenClass)
{
    if (tokenClass == "NUMBER")
    {
        return TokenType::NUMBER;
    }
    else if (tokenClass == "IDENTIFIER")
    {
        return TokenType::IDENTIFIER;
    }
    // Should not reach here with current implementation
    return TokenType::IDENTIFIER;
}

void Lexer::setInput(const std::string &input)
{
    input_ = input;
    current_pos_ = 0;
}

std::optional<Token> Lexer::next()
{
    Token token(TokenType::IDENTIFIER, ""); // dummy initialization
    if (getNextToken(token))
    {
        return token;
    }
    return std::nullopt;
}

bool Lexer::hasNext() const
{
    // Skip whitespace to check if there's more content
    size_t pos = current_pos_;
    while (pos < input_.size() && (input_[pos] == ' ' || input_[pos] == '\t' ||
                                   input_[pos] == '\n' || input_[pos] == '\r'))
    {
        ++pos;
    }
    return pos < input_.size();
}

void Lexer::reset()
{
    current_pos_ = 0;
}

void Lexer::skipWhitespace()
{
    while (current_pos_ < input_.size() &&
           (input_[current_pos_] == ' ' || input_[current_pos_] == '\t' ||
            input_[current_pos_] == '\n' || input_[current_pos_] == '\r'))
    {
        ++current_pos_;
    }
}

bool Lexer::getNextToken(Token &token)
{
    // Skip whitespace
    skipWhitespace();

    if (current_pos_ >= input_.size())
    {
        return false;
    }

    size_t token_start = current_pos_;

    // Reset DFAs for next token
    for (size_t i = 0; i < dfa_list_.size(); ++i)
    {
        dfa_list_[i]->Init();
        survival_list_[i] = true;
    }

    size_t last_accepted_pos = token_start;
    int last_accepted_dfa = -1;
    bool have_survival = true;

    // Try to consume characters as long as possible
    while (current_pos_ < input_.size() && have_survival)
    {
        // Stop at whitespace
        if (input_[current_pos_] == ' ' || input_[current_pos_] == '\t' ||
            input_[current_pos_] == '\n' || input_[current_pos_] == '\r')
        {
            break;
        }

        have_survival = false;

        for (size_t i = 0; i < dfa_list_.size(); ++i)
        {
            if (survival_list_[i])
            {
                if (!dfa_list_[i]->Evaluate(input_[current_pos_]))
                {
                    survival_list_[i] = false;
                }
                else
                {
                    have_survival = true;
                    // Check if this DFA is in accepting state
                    if (dfa_list_[i]->isAccepted())
                    {
                        last_accepted_pos = current_pos_ + 1;
                        last_accepted_dfa = i;
                    }
                }
            }
        }

        ++current_pos_;
    }

    // If we found an accepted token, create and return it
    if (last_accepted_dfa != -1)
    {
        std::string tokenValue = input_.substr(token_start,
                                               last_accepted_pos - token_start);
        std::string tokenClass = dfa_list_[last_accepted_dfa]->getTokenClass();
        TokenType tokenType = getTokenType(tokenClass);

        token = Token(tokenType, tokenValue);

        // Set current_pos_ to the position after the accepted token
        current_pos_ = last_accepted_pos;
        return true;
    }
    else
    {
        // Skip the problematic character
        std::cerr << "Warning: No valid token found at position " << token_start
                  << " for character '" << input_[token_start] << "'" << std::endl;
        ++current_pos_;
        return false;
    }
}

std::vector<Token> Lexer::tokenize(const std::string &input)
{
    std::vector<Token> tokens;

    // Set input and reset position
    setInput(input);

    // Use the streaming interface to get all tokens
    auto token_opt = next();
    while (token_opt.has_value())
    {
        tokens.push_back(token_opt.value());
        token_opt = next();
    }

    return tokens;
}
