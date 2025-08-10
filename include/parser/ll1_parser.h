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
  explicit LL1Parser() { Init(); }
  ~LL1Parser() = default;

  void Init();
  auto Parse() -> std::shared_ptr<core::AST>;
  void PrintParseTable() const;

  // Additional utility functions
  void SetInput(const std::string &input);
  void PrintParsingStack() const;
  auto IsValidSymbol(const std::string &symbol) const -> bool;
  auto HasParseTableEntry(const std::string &nonTerminal, const std::string &terminal) const -> bool;
  auto BuildAST(const std::shared_ptr<core::TreeNode> &parse_tree) -> std::shared_ptr<core::AST>;

 private:
  std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> parse_table_;
  std::unordered_set<std::string> terminals_;
  std::unordered_set<std::string> symbols_;

  std::stack<std::pair<std::string, std::shared_ptr<core::TreeNode>>> parse_stack_;
  lexer::Lexer lexer_;

  // AST transformation helper methods
  auto TransformToASTNode(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
  auto CreateTerminalASTNode(const std::string &symbol) -> std::shared_ptr<core::AST::ASTNode>;
  auto TransformProgram(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
  auto TransformStatement(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
  auto TransformExpression(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
  auto TransformExpressionPrime(const std::shared_ptr<core::TreeNode> &parse_node,
                                std::shared_ptr<core::AST::ASTNode> left_operand)
      -> std::shared_ptr<core::AST::ASTNode>;
  auto TransformTerm(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
  auto TransformTermPrime(const std::shared_ptr<core::TreeNode> &parse_node,
                          std::shared_ptr<core::AST::ASTNode> left_operand) -> std::shared_ptr<core::AST::ASTNode>;
  auto TransformFactor(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;

  // Helper method to collect statements directly into root
  void CollectStatements(const std::shared_ptr<core::TreeNode> &parse_node,
                         const std::shared_ptr<core::AST::ASTNode> &root);

  // Debug helper
  void PrintParseTreeDebug(const std::shared_ptr<core::TreeNode> &node, int depth);
};

}  // namespace scp::parser
