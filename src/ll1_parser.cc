#include "../include/ll1_parser.h"
#include "../include/token.h"

#include <iostream>
#include <string_view>

// Help Function
bool isTerminal(const std::string &symbol,
                const std::unordered_set<std::string> &terminals)
{
  return terminals.find(symbol) != terminals.end();
}

bool term(Token token, const std::string &symbol)
{
  return toString(token.getType()) == std::string_view(symbol);
}

// LL1Parser Implementation
void LL1Parser::Init(const char *grammarFile) {}

void LL1Parser::parse(Token token) {}

void LL1Parser::printParseTable() const
{
  // Print the parse table for debugging purposes
  for (const auto &row : parseTable_)
  {
    for (const auto &cell : row)
    {
      for (const auto &symbol : cell)
      {
        std::cout << symbol << " ";
      }
      std::cout << "| ";
    }
    std::cout << std::endl;
  }
}
