#pragma once

#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "core/ast.h"
#include "lexer/lexer.h"

namespace scp::parser {

/**
 * LL(1) Parser class for parsing source code.
 */
class LL1Parser {
 public:
  /**
   * Constructor for the LL1Parser.
   * @param program_name The name of the program being parsed.
   */
  explicit LL1Parser(std::string program_name) : program_name_(std::move(program_name)) { Init(); }

  /**
   * Destructor for the LL1Parser.
   */
  ~LL1Parser() = default;

  /**
   * Initialize the parser.
   */
  void Init();

  /**
   * Parse the input and generate the AST.
   * @return A shared pointer to the root AST node.
   */
  auto Parse() -> std::shared_ptr<core::AST>;

  /**
   * Print the LL(1) parsing table, this is a const function.
   */
  void PrintParseTable() const;

  /**
   * Set the input for the parser.
   * @param input The input string to parse.
   */
  void SetInput(const std::string &input);

  /**
   * Print the current state of the parsing stack.
   */
  void PrintParsingStack() const;

  /**
   * Check if a symbol is a valid grammar symbol.
   * @param symbol The symbol to check.
   * @return True if the symbol is valid, false otherwise.
   */
  auto IsValidSymbol(const std::string &symbol) const -> bool;

  /**
   * Check if the parse table has an entry for the given non-terminal and terminal.
   * @param nonTerminal The non-terminal symbol.
   * @param terminal The terminal symbol.
   * @return True if the parse table has an entry, false otherwise.
   */
  auto HasParseTableEntry(const std::string &nonTerminal, const std::string &terminal) const -> bool;

  /**
   * Build the AST from the parse tree.
   * @param parse_tree The root of the parse tree.
   * @return A shared pointer to the root AST node.
   */
  auto BuildAST(const std::shared_ptr<core::TreeNode> &parse_tree) -> std::shared_ptr<core::AST>;

 private:
  /* The name of the program being parsed. */
  std::string program_name_;
  /* The root of the parse tree */
  std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> parse_table_;
  /* The set of terminal symbols */
  std::unordered_set<std::string> terminals_;
  /* The set of non-terminal symbols */
  std::unordered_set<std::string> symbols_;
  /* The parsing stack */
  std::stack<std::pair<std::string, std::shared_ptr<core::TreeNode>>> parse_stack_;
  /* The lexer for tokenizing the input */
  lexer::Lexer lexer_;

  /**
   * AST transformation helper methods
   */
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

  /**
   * Collect all statements from the parse tree.
   * @param parse_node The root of the parse tree.
   * @param root The root AST node.
   */
  void CollectStatements(const std::shared_ptr<core::TreeNode> &parse_node,
                         const std::shared_ptr<core::AST::ASTNode> &root);

  /**
   * Print the parse tree for debugging purposes.
   * @param node The current node in the parse tree.
   * @param depth The current depth in the parse tree.
   */
  void PrintParseTreeDebug(const std::shared_ptr<core::TreeNode> &node, int depth);
};

}  // namespace scp::parser
