#include "parser/ll1_parser.h"

#include <cctype>
#include <iostream>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "constant/AST_constant.h"
#include "constant/error_messages.h"
#include "core/ast.h"
#include "core/token.h"

namespace scp::parser {

// Help Function
/**
 * Check if a symbol is a terminal symbol.
 * @param symbol The symbol to check.
 * @param terminals The set of terminal symbols.
 * @return True if the symbol is a terminal, false otherwise.
 */
auto IsTerminal(const std::string &symbol, const std::unordered_set<std::string> &terminals) -> bool {
  return terminals.find(symbol) != terminals.end();
}

/**
 * Convert a token type to a string representation for the parser table.
 * @param type The token type to convert.
 * @return A string representation of the token type.
 */
auto TokenTypeToParserString(core::TokenType type) -> std::string {
  switch (type) {
    case core::TokenType::IDENTIFIER:
      return "identifier";
    case core::TokenType::NUMBER:
      return "number";
    case core::TokenType::LEFT_PAREN:
      return "left_paren";
    case core::TokenType::RIGHT_PAREN:
      return "right_paren";
    case core::TokenType::PLUS:
      return "plus";
    case core::TokenType::TIMES:
      return "times";
    case core::TokenType::ASSIGN:
      return "assign";
    case core::TokenType::SEMICOLON:
      return "semicolon";
    case core::TokenType::END_OF_FILE:
      return "$";
    default:
      return "";
  }
}

/**
 * Check if a token matches a specific terminal symbol.
 * @param token The token to check.
 * @param symbol The terminal symbol to match against.
 * @return True if the token matches the symbol, false otherwise.
 */
auto Term(const core::Token &token, const std::string &symbol) -> bool {
  return TokenTypeToParserString(token.GetType()) == symbol;
}

// LL1Parser Implementation
void LL1Parser::Init() {
  symbols_ = {"Program", "StatementList", "Statement", "Expression", "Term", "Factor"};
  terminals_ = {"identifier", "number", "left_paren", "right_paren", "plus", "times", "assign", "semicolon", "$"};
  symbols_.insert(terminals_.begin(), terminals_.end());
  parse_table_ = {
      // Program -> StatementList
      {"Program", {{"identifier", {"StatementList"}}, {"$", {"StatementList"}}}},

      // StatementList -> Statement StatementList
      //               -> ε
      {"StatementList", {{"identifier", {"Statement", "StatementList"}}, {"$", {"ε"}}}},

      // Statement -> identifier assign Expression semicolon
      {"Statement", {{"identifier", {"identifier", "assign", "Expression", "semicolon"}}}},

      // Expression -> Term Expression'
      {"Expression",
       {{"identifier", {"Term", "Expression'"}},
        {"number", {"Term", "Expression'"}},
        {"left_paren", {"Term", "Expression'"}}}},

      // Expression' -> plus Term Expression'
      // Expression' -> ε
      {"Expression'", {{"plus", {"plus", "Term", "Expression'"}}, {"semicolon", {"ε"}}, {"right_paren", {"ε"}}}},

      // Term -> Factor Term'
      {"Term",
       {{"identifier", {"Factor", "Term'"}}, {"number", {"Factor", "Term'"}}, {"left_paren", {"Factor", "Term'"}}}},

      // Term' -> times Factor Term'
      // Term' -> ε
      {"Term'",
       {{"times", {"times", "Factor", "Term'"}}, {"plus", {"ε"}}, {"semicolon", {"ε"}}, {"right_paren", {"ε"}}}},

      // Factor -> identifier | number | left_paren Expression right_paren
      {"Factor",
       {{"identifier", {"identifier"}},
        {"number", {"number"}},
        {"left_paren", {"left_paren", "Expression", "right_paren"}}}}};
}

auto LL1Parser::Parse() -> std::shared_ptr<core::AST> {
  // Initialize parse tree root
  std::shared_ptr<core::TreeNode> parse_root =
      std::make_shared<core::TreeNode>(constant::ASTConstant::ROOT_NODE_VALUE);  // Use "-" as root value

  // Initialize parsing stack with end marker and start symbol
  parse_stack_.push(std::make_pair("$", std::make_shared<core::TreeNode>(constant::ASTConstant::END_NODE_VALUE)));
  parse_stack_.push(std::make_pair("Program", parse_root));  // Start symbol

  core::Token current_token = core::Token(core::TokenType::IDENTIFIER, "");  // Initialize with empty token
  bool token_consumed = true;  // Flag to track if current token needs to be consumed

  while (!parse_stack_.empty() && parse_stack_.top().first != constant::ASTConstant::END_NODE_VALUE) {
    // Get current token if needed
    if (token_consumed) {
      if (!lexer_.HasNext()) {
        // No more tokens, use EOF marker
        current_token = core::Token(core::TokenType::END_OF_FILE, constant::ASTConstant::END_NODE_VALUE);  // EOF token
      } else {
        std::optional<core::Token> token_opt = lexer_.Next();
        if (!token_opt.has_value()) {
          std::cerr << constant::ErrorMessages::FAIL_TO_GET_NEXT_TOKEN << std::endl;
          return nullptr;
        }
        current_token = token_opt.value();
      }
      token_consumed = false;
    }

    std::string current_symbol = parse_stack_.top().first;
    std::shared_ptr<core::TreeNode> current_tree_node = parse_stack_.top().second;

    if (IsTerminal(current_symbol, terminals_)) {
      // Terminal symbol processing
      if (Term(current_token, current_symbol)) {
        // Match found, update tree node with actual token value and consume token
        current_tree_node->val_ = current_token.GetValue();
        parse_stack_.pop();
        token_consumed = true;
      } else {
        std::cerr << constant::ErrorMessages::ParsingError(current_symbol, current_token) << std::endl;
        return nullptr;
      }
    } else {
      // Non-terminal processing
      auto non_terminal_it = parse_table_.find(current_symbol);
      if (non_terminal_it == parse_table_.end()) {
        std::cerr << constant::ErrorMessages::NoEntriesInParseTable(current_symbol) << std::endl;
        return nullptr;
      }

      std::string token_string = TokenTypeToParserString(current_token.GetType());
      auto production_it = non_terminal_it->second.find(token_string);

      // If no production found for current token, try EOF case
      if (production_it == non_terminal_it->second.end() && token_string != constant::ASTConstant::END_NODE_VALUE) {
        production_it = non_terminal_it->second.find(constant::ASTConstant::END_NODE_VALUE);
      }

      if (production_it == non_terminal_it->second.end()) {
        std::cerr << constant::ErrorMessages::NoProductionRuleForSymbol(current_symbol, token_string) << std::endl;
        return nullptr;
      }

      const std::vector<std::string> &production = production_it->second;
      parse_stack_.pop();  // Remove current non-terminal from stack

      // Handle epsilon production
      if (production.size() == 1 && (production[0] == "ε" || production[0] == "epsilon")) {
        // Epsilon production - don't push anything to stack, don't consume token
        // The tree node remains as is (representing the epsilon production)
        continue;
      }

      // Push production symbols in reverse order (rightmost first)
      for (auto it = production.rbegin(); it != production.rend(); ++it) {
        const std::string &symbol = *it;

        // Create child node for this symbol
        std::shared_ptr<core::TreeNode> child_node = std::make_shared<core::TreeNode>(symbol);
        if (current_tree_node) {
          current_tree_node->AddChild(child_node);
        }
        // Push symbol and its corresponding tree node
        parse_stack_.push(std::make_pair(symbol, child_node));
      }
    }
  }

  // Check if parsing completed successfully
  if (parse_stack_.empty() || parse_stack_.top().first != constant::ASTConstant::END_NODE_VALUE) {
    std::cerr << constant::ErrorMessages::UNEXPECTED_END_OF_PARSING;
    if (parse_stack_.empty()) {
      std::cerr << "empty";
    } else {
      std::cerr << "top = '" << parse_stack_.top().first << "'";
    }
    std::cerr << std::endl;
    return nullptr;
  }

  // Check if all tokens consumed
  if (lexer_.HasNext()) {
    std::cerr << constant::ErrorMessages::INPUT_NOT_FULLY_CONSUMED << std::endl;
    return nullptr;
  }

  parse_stack_.pop();  // Remove the end marker "$"
  return BuildAST(parse_root);
}

void LL1Parser::PrintParseTable() const {
  for (const auto &row : parse_table_) {
    std::cout << "Non-terminal: " << row.first << std::endl;

    for (const auto &cell : row.second) {
      std::cout << "  " << cell.first << " -> ";

      if (cell.second.empty()) {
        std::cout << "ε";
      } else {
        for (const auto &symbol : cell.second) {
          if (&symbol != &cell.second.front()) {
            std::cout << " ";
          }
          std::cout << symbol;
        }
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
}

void LL1Parser::PrintParsingStack() const {
  std::cout << "Parse Stack (top to bottom): ";
  std::stack<std::pair<std::string, std::shared_ptr<core::TreeNode>>> temp_stack = parse_stack_;
  std::vector<std::string> stack_contents;

  while (!temp_stack.empty()) {
    stack_contents.push_back(temp_stack.top().first);
    temp_stack.pop();
  }

  for (auto it = stack_contents.rbegin(); it != stack_contents.rend(); ++it) {
    std::cout << *it << " ";
  }
  std::cout << std::endl;
}

auto LL1Parser::IsValidSymbol(const std::string &symbol) const -> bool {
  return symbols_.find(symbol) != symbols_.end() || terminals_.find(symbol) != terminals_.end() ||
         symbol == constant::ASTConstant::END_NODE_VALUE || symbol == "ε" || symbol == "epsilon";
}

void LL1Parser::SetInput(const std::string &input) { lexer_.SetInput(input); }

auto LL1Parser::HasParseTableEntry(const std::string &nonTerminal, const std::string &terminal) const -> bool {
  auto non_terminal_it = parse_table_.find(nonTerminal);
  if (non_terminal_it == parse_table_.end()) {
    return false;
  }

  auto terminal_it = non_terminal_it->second.find(terminal);
  return terminal_it != non_terminal_it->second.end();
}

auto LL1Parser::BuildAST(const std::shared_ptr<core::TreeNode> &parse_tree) -> std::shared_ptr<core::AST> {
  if (!parse_tree) {
    return nullptr;
  }

  auto ast = std::make_shared<core::AST>(program_name_);
  auto root_node = TransformToASTNode(parse_tree);
  ast->SetRoot(root_node);

  return ast;
}

void LL1Parser::PrintParseTreeDebug(const std::shared_ptr<core::TreeNode> &node, int depth) {
  if (!node) {
    for (int i = 0; i < depth; ++i) {
      std::cout << "  ";
    }
    std::cout << "NULL" << std::endl;
    return;
  }

  for (int i = 0; i < depth; ++i) {
    std::cout << "  ";
  }

  std::cout << "Node: '" << node->val_ << "' (children: " << node->children_.size() << ")" << std::endl;

  for (const auto &child : node->children_) {
    PrintParseTreeDebug(child, depth + 1);
  }
}

auto LL1Parser::TransformToASTNode(const std::shared_ptr<core::TreeNode> &parse_node)
    -> std::shared_ptr<core::AST::ASTNode> {
  if (!parse_node) {
    return nullptr;
  }

  const std::string &symbol = parse_node->val_;

  // Handle terminal nodes
  if (IsTerminal(symbol, terminals_)) {
    return CreateTerminalASTNode(symbol);
  }

  // Handle special case: root node with value "-" but representing Program
  if (symbol == "-" && !parse_node->children_.empty()) {
    // This is the parse tree root, treat it as Program
    return TransformProgram(parse_node);
  }

  // Handle non-terminal nodes - transform based on grammar rule
  if (symbol == "Program") {
    return TransformProgram(parse_node);
  }
  if (symbol == "StatementList") {
    return nullptr;
  }
  if (symbol == "Statement") {
    return TransformStatement(parse_node);
  }
  if (symbol == "Expression") {
    return TransformExpression(parse_node);
  }
  if (symbol == "Term") {
    return TransformTerm(parse_node);
  }
  if (symbol == "Factor") {
    return TransformFactor(parse_node);
  }

  // Default case - create a generic node
  auto ast_node = std::make_shared<core::AST::ASTNode>(core::ASTNodeType::ROOT, symbol);
  for (const auto &child : parse_node->children_) {
    auto child_ast = TransformToASTNode(child);
    if (child_ast) {
      ast_node->AddChild(child_ast);
    }
  }
  return ast_node;
}

auto LL1Parser::CreateTerminalASTNode(const std::string &symbol) -> std::shared_ptr<core::AST::ASTNode> {
  if (symbol == "identifier") {
    return std::make_shared<core::AST::ASTNode>(core::ASTNodeType::IDENTIFIER, symbol);
  }
  if (symbol == "number") {
    return std::make_shared<core::AST::ASTNode>(core::ASTNodeType::NUMBER, symbol);
  }
  if (symbol == "plus") {
    return std::make_shared<core::AST::ASTNode>(core::ASTNodeType::PLUS, "+");
  }
  if (symbol == "times") {
    return std::make_shared<core::AST::ASTNode>(core::ASTNodeType::TIMES, "*");
  }
  if (symbol == "assign") {
    return std::make_shared<core::AST::ASTNode>(core::ASTNodeType::ASSIGN, "<-");
  }

  // For other terminals like semicolon, parentheses, etc., return nullptr
  // as they are typically not needed in the AST
  return nullptr;
}

auto LL1Parser::TransformProgram(const std::shared_ptr<core::TreeNode> &parse_node)
    -> std::shared_ptr<core::AST::ASTNode> {
  // Program -> StatementList
  // Create root node and collect all statements directly
  auto root = std::make_shared<core::AST::ASTNode>(core::ASTNodeType::ROOT, constant::ASTConstant::ROOT_NODE_VALUE);

  if (!parse_node->children_.empty()) {
    // Get the StatementList and extract all statements from it
    auto statement_list_node = parse_node->children_.front();
    CollectStatements(statement_list_node, root);
  }

  return root;
}

void LL1Parser::CollectStatements(const std::shared_ptr<core::TreeNode> &parse_node,
                                  const std::shared_ptr<core::AST::ASTNode> &root) {
  // StatementList -> Statement StatementList | ε
  for (const auto &child : parse_node->children_) {
    if (child->val_ == "Statement") {
      auto stmt_node = TransformToASTNode(child);
      if (stmt_node) {
        root->AddChild(stmt_node);
      }
    } else if (child->val_ == "StatementList") {
      // Recursively process nested StatementList
      CollectStatements(child, root);
    }
  }
}

auto LL1Parser::TransformStatement(const std::shared_ptr<core::TreeNode> &parse_node)
    -> std::shared_ptr<core::AST::ASTNode> {
  // Statement -> identifier assign Expression semicolon
  auto assign_node = std::make_shared<core::AST::ASTNode>(core::ASTNodeType::ASSIGN, "<-");

  std::shared_ptr<core::AST::ASTNode> identifier_node = nullptr;
  std::shared_ptr<core::AST::ASTNode> expression_node = nullptr;

  // Since children are added with push_front, they are in reverse order
  // From debug output: index 0='a', index 1='<-', index 2='Expression', index 3=';'

  int child_count = 0;
  for (const auto &child : parse_node->children_) {
    if (child_count == 0) {
      // This should be identifier (first element due to reverse order)
      if (child->children_.empty()) {
        identifier_node = std::make_shared<core::AST::ASTNode>(core::ASTNodeType::IDENTIFIER, child->val_);
      }
    } else if (child_count == 2) {
      // This should be Expression
      if (child->val_ == "Expression") {
        expression_node = TransformToASTNode(child);
      }
    }
    // Skip assign token (index 1) and semicolon (index 3)
    child_count++;
  }

  if (identifier_node) {
    assign_node->AddChild(identifier_node);
  }
  if (expression_node) {
    assign_node->AddChild(expression_node);
  }

  return assign_node;
}

auto LL1Parser::TransformExpression(const std::shared_ptr<core::TreeNode> &parse_node)
    -> std::shared_ptr<core::AST::ASTNode> {
  // Expression -> Term Expression'
  // We need to handle left-associative operators properly

  std::shared_ptr<core::AST::ASTNode> left_operand = nullptr;
  std::shared_ptr<core::AST::ASTNode> expression_prime = nullptr;

  for (const auto &child : parse_node->children_) {
    if (child->val_ == "Term") {
      left_operand = TransformToASTNode(child);
    } else if (child->val_ == "Expression'") {
      expression_prime = TransformExpressionPrime(child, left_operand);
    }
  }

  return expression_prime ? expression_prime : left_operand;
}

auto LL1Parser::TransformExpressionPrime(const std::shared_ptr<core::TreeNode> &parse_node,
                                         std::shared_ptr<core::AST::ASTNode> left_operand)
    -> std::shared_ptr<core::AST::ASTNode> {
  // Expression' -> plus Term Expression' | ε

  if (parse_node->children_.empty()) {
    // ε production - return the left operand as is
    return left_operand;
  }

  std::shared_ptr<core::AST::ASTNode> operator_node = nullptr;
  std::shared_ptr<core::AST::ASTNode> right_operand = nullptr;
  std::shared_ptr<core::AST::ASTNode> rest_expression = nullptr;

  // Children order: index 0='+', index 1='Term', index 2='Expression''
  int child_count = 0;
  for (const auto &child : parse_node->children_) {
    if (child_count == 0 && child->val_ == "+") {
      operator_node = std::make_shared<core::AST::ASTNode>(core::ASTNodeType::PLUS, "+");
    } else if (child_count == 1 && child->val_ == "Term") {
      right_operand = TransformToASTNode(child);
    } else if (child_count == 2 && child->val_ == "Expression'") {
      // Create binary operation node first
      if (operator_node && left_operand && right_operand) {
        operator_node->AddChild(left_operand);
        operator_node->AddChild(right_operand);
        rest_expression = TransformExpressionPrime(child, operator_node);
      }
    }
    child_count++;
  }

  return rest_expression ? rest_expression : (operator_node ? operator_node : left_operand);
}

auto LL1Parser::TransformTerm(const std::shared_ptr<core::TreeNode> &parse_node)
    -> std::shared_ptr<core::AST::ASTNode> {
  // Term -> Factor Term'

  std::shared_ptr<core::AST::ASTNode> left_operand = nullptr;
  std::shared_ptr<core::AST::ASTNode> term_prime = nullptr;

  for (const auto &child : parse_node->children_) {
    if (child->val_ == "Factor") {
      left_operand = TransformToASTNode(child);
    } else if (child->val_ == "Term'") {
      term_prime = TransformTermPrime(child, left_operand);
    }
  }

  return term_prime ? term_prime : left_operand;
}

auto LL1Parser::TransformTermPrime(const std::shared_ptr<core::TreeNode> &parse_node,
                                   std::shared_ptr<core::AST::ASTNode> left_operand)
    -> std::shared_ptr<core::AST::ASTNode> {
  // Term' -> times Factor Term' | ε

  if (parse_node->children_.empty()) {
    // ε production - return the left operand as is
    return left_operand;
  }

  std::shared_ptr<core::AST::ASTNode> operator_node = nullptr;
  std::shared_ptr<core::AST::ASTNode> right_operand = nullptr;
  std::shared_ptr<core::AST::ASTNode> rest_term = nullptr;

  // Children order: index 0='*', index 1='Factor', index 2='Term''
  int child_count = 0;
  for (const auto &child : parse_node->children_) {
    if (child_count == 0 && child->val_ == "*") {
      operator_node = std::make_shared<core::AST::ASTNode>(core::ASTNodeType::TIMES, "*");
    } else if (child_count == 1 && child->val_ == "Factor") {
      right_operand = TransformToASTNode(child);
    } else if (child_count == 2 && child->val_ == "Term'") {
      // Create binary operation node first
      if (operator_node && left_operand && right_operand) {
        operator_node->AddChild(left_operand);
        operator_node->AddChild(right_operand);
        rest_term = TransformTermPrime(child, operator_node);
      }
    }
    child_count++;
  }

  return rest_term ? rest_term : (operator_node ? operator_node : left_operand);
}

auto LL1Parser::TransformFactor(const std::shared_ptr<core::TreeNode> &parse_node)
    -> std::shared_ptr<core::AST::ASTNode> {
  // Factor -> identifier | number | left_paren Expression right_paren

  // Check each child to find the meaningful content
  for (const auto &child : parse_node->children_) {
    if (child->val_ == "Expression") {
      // For parenthesized expressions, just return the expression node
      return TransformToASTNode(child);
    }
    if (child->children_.empty() && !child->val_.empty()) {
      // This is a leaf node with actual value
      // Determine type based on content
      if (std::isdigit(child->val_[0]) != 0) {
        return std::make_shared<core::AST::ASTNode>(core::ASTNodeType::NUMBER, child->val_);
      }
      if (std::isalpha(child->val_[0]) != 0) {
        return std::make_shared<core::AST::ASTNode>(core::ASTNodeType::IDENTIFIER, child->val_);
      }
    }
  }

  return nullptr;
}

}  // namespace scp::parser
