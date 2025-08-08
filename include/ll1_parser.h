#pragma once

#include "token.h"
#include <queue>
#include <stack>
#include <string>
#include <unordered_set>
#include <vector>

class LL1Parser {
public:
  LL1Parser(const char *grammarFile) { Init(grammarFile); }
  ~LL1Parser() = default;

  void Init(const char *grammarFile);
  void parse(Token token);
  void printParseTable() const;
  bool accept() { return accepted_; };

private:
  std::vector<std::vector<std::vector<std::string>>> parseTable_;
  std::unordered_set<std::string> terminals_;
  std::unordered_set<std::string> symbols_;

  std::stack<std::string> parseStack_;
  bool accepted_;
  std::queue<Token> token_stream_;
};
