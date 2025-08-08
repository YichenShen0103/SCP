#include <iostream>
#include <vector>
#include "lexer.h"
#include "token.h"

int main()
{
    Lexer lexer;

    // Test inputs including edge cases
    std::vector<std::string> test_inputs = {
        "hello 123",
        "variable_name 456",
        "_identifier 789",
        "var123 999",
        "abc123def 42",
        "  spaced   tokens  567  ",
        "123abc",      // should parse 123 and fail on abc
        "_",           // single underscore identifier
        "0",           // single digit number
        "",            // empty string
        "   ",         // whitespace only
        "123 456 789", // multiple numbers
        "a b c",       // multiple single-char identifiers
        "mixed123test 999end"};

    for (const std::string &input : test_inputs)
    {
        std::cout << "Input: \"" << input << "\"" << std::endl;

        std::vector<Token> tokens = lexer.tokenize(input);

        if (tokens.empty())
        {
            std::cout << "  No tokens found" << std::endl;
        }
        else
        {
            for (const Token &token : tokens)
            {
                std::cout << "  Token: " << toString(token.getType())
                          << " = \"" << token.getValue() << "\"" << std::endl;
            }
        }
        std::cout << std::endl;
    }

    return 0;
}
