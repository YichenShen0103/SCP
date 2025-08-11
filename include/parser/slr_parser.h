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

class SLRParser {
 public:
  /**
   * Represents a parsing action in the SLR parser.
   */
  struct Action {
    /**
     * Represents the type of action to be taken.
     */
    enum class ActionType { SHIFT, REDUCE, ACCEPT, REJECT };

    /**
     * Default constructor for Action.
     */
    Action() : type_(ActionType::REJECT), state_(0) {}

    /**
     * Constructs a new Action.
     * @param type The type of action to be taken.
     * @param state The state to transition to, reserve for SHIFT.
     * @param rhs The right-hand side symbols to reduce, reserve for REDUCE.
     * @param lhs The left-hand side symbol to reduce to, reserve for REDUCE.
     */
    explicit Action(ActionType type, int state = 0, std::vector<std::string> rhs = {}, std::string lhs = {})
        : type_(type), state_(state), rhs_(std::move(rhs)), lhs_(std::move(lhs)) {}

    /* The type of action to be taken. */
    ActionType type_;
    /* The state to transition to, reserve for SHIFT. */
    int state_;
    /* The right-hand side symbols to reduce, reserve for REDUCE. */
    std::vector<std::string> rhs_;
    /* The left-hand side symbol to reduce to, reserve for REDUCE. */
    std::string lhs_;
  };

  /**
   * Constructor for the SLRParser.
   * @param program_name The name of the program being parsed.
   */
  explicit SLRParser(std::string program_name) : program_name_(std::move(program_name)) { Init(); }

  /**
   * Destructor for the SLRParser.
   */
  ~SLRParser() = default;

  /**
   * Initialize the SLR parser. Main effort is to build the action and goto tables.
   */
  void Init();

  /**
   * Parse the input and produce an AST.
   * @return A shared pointer to the root AST node.
   */
  auto Parse() -> std::shared_ptr<core::AST>;

  /**
   * Set the input for the parser.
   * @param input The input string to parse.
   */
  void SetInput(const std::string &input);

  /**
   * Build the AST from the parse tree.
   * @param parse_tree The root of the parse tree.
   * @return A shared pointer to the root AST node.
   */
  auto BuildAST(const std::shared_ptr<core::TreeNode> &parse_tree) -> std::shared_ptr<core::AST>;

  /**
   * Check if a symbol is valid.
   * @param symbol The symbol to check.
   * @return True if the symbol is valid, false otherwise.
   */
  auto IsValidSymbol(const std::string &symbol) const -> bool;

 private:
  /* The name of the program being parsed. */
  std::string program_name_;
  /* terminal symbols */
  std::unordered_set<std::string> terminals_;
  /* all symbols */
  std::unordered_set<std::string> symbols_;
  /* SLR parsing stack */
  std::stack<std::tuple<std::string, std::shared_ptr<core::TreeNode>, int>> slr_stack_;
  /* lexer for tokenization */
  lexer::Lexer lexer_;
  /* The action table for the SLR parser */
  std::unordered_map<int, std::unordered_map<std::string, Action>> action_table_;
  /* Goto Table of dfa */
  std::unordered_map<int, std::unordered_map<std::string, int>> goto_table_;

  /**
   * Convert token type to parser terminal string.
   * @param type The token type.
   * @return The corresponding terminal string for the parser.
   */
  auto TokenTypeToString(core::TokenType type) -> std::string;

  /**
   * AST transformation helper methods
   */
  auto TransformToASTNode(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
  auto CreateTerminalASTNode(const std::string &symbol) -> std::shared_ptr<core::AST::ASTNode>;
  auto TransformProgram(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
  auto TransformStatement(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
  auto TransformExpression(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
  auto TransformTerm(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;
  auto TransformFactor(const std::shared_ptr<core::TreeNode> &parse_node) -> std::shared_ptr<core::AST::ASTNode>;

  /**
   * Collect all statements from the parse tree.
   * @param parse_node The root of the parse tree.
   * @param root The root AST node.
   */
  void CollectStatements(const std::shared_ptr<core::TreeNode> &parse_node,
                         const std::shared_ptr<core::AST::ASTNode> &root);

  /**
   * Check if a symbol is a terminal symbol.
   * @param symbol The symbol to check.
   * @param terminals The set of terminal symbols.
   * @return True if the symbol is a terminal, false otherwise.
   */
  auto IsTerminal(const std::string &symbol, const std::unordered_set<std::string> &terminals) -> bool;
};

}  // namespace scp::parser
