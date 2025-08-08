#include "../include/lexer.h"
#include <iostream>

int main()
{
    Lexer lexer;
    std::string input = "hello123 world 456 test_variable 43abcd234 _hello"
                        " hello 123"
                        " variable_name 456"
                        " _identifier 789"
                        " var123 999"
                        " abc123def 42"
                        "  spaced   tokens  567  "
                        " 123abc"      // should parse 123 and fail on abc
                        " _"           // single underscore identifier
                        " 0"           // single digit number
                        " "            // empty string
                        "   "          // whitespace only
                        " 123 456 789" // multiple numbers
                        " a b c"       // multiple single-char identifiers
                        " mixed123test 999end";

    std::cout
        << "Testing streaming interface:" << std::endl;
    lexer.setInput(input);

    std::cout << "hasNext(): " << (lexer.hasNext() ? "true" : "false") << std::endl;

    int count = 0;
    while (lexer.hasNext())
    {
        auto token = lexer.next();
        if (token.has_value())
        {
            std::cout << "Token " << ++count << ": "
                      << toString(token->getType()) << " = \""
                      << token->getValue() << "\"" << std::endl;
        }
    }

    std::cout << "\nTesting legacy interface:" << std::endl;
    auto tokens = lexer.tokenize(input);
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        std::cout << "Token " << (i + 1) << ": "
                  << toString(tokens[i].getType()) << " = \""
                  << tokens[i].getValue() << "\"" << std::endl;
    }

    std::cout << "\nTesting reset functionality:" << std::endl;
    lexer.setInput("abc 123");
    auto first = lexer.next();
    if (first.has_value())
    {
        std::cout << "First token: " << first->getValue() << std::endl;
    }

    lexer.reset();
    auto first_again = lexer.next();
    if (first_again.has_value())
    {
        std::cout << "After reset, first token: " << first_again->getValue() << std::endl;
    }

    return 0;
}
