#pragma once

#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/ast.h"
#include "lexer/lexer.h"

namespace scp::parser {

class LL1Parser {
 public:
  explicit LL1Parser(const char *grammarFile) { Init(grammarFile); }
  ~LL1Parser() = default;

  void Init(const char *grammarFile);
  auto Parse() -> std::shared_ptr<core::TreeNode>;
  void PrintParseTable() const;

  // Additional utility functions
  void SetInput(const std::string &input);
  void PrintParsingStack() const;
  auto IsValidSymbol(const std::string &symbol) const -> bool;
  auto HasParseTableEntry(const std::string &nonTerminal, const std::string &terminal) const -> bool;

 private:
  std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> parse_table_;
  std::unordered_set<std::string> terminals_;
  std::unordered_set<std::string> symbols_;

  std::stack<std::pair<std::string, std::shared_ptr<core::TreeNode>>> parse_stack_;
  lexer::Lexer lexer_;
};

}  // namespace scp::parser
