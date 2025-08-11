#include "parser/slr_parser.h"

#include <cctype>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "constant/ast_constant.h"
#include "constant/error_messages.h"
#include "core/ast.h"
#include "core/token.h"

namespace scp::parser {

void SLRParser::SetInput(const std::string &input) { lexer_.SetInput(input); }

void SLRParser::Init() {
  symbols_ = {"Program", "StatementList", "Statement", "Expression", "Term", "Factor"};
  terminals_ = {"identifier", "number", "left_paren", "right_paren", "plus", "times", "assign", "semicolon", "$"};
  symbols_.insert(terminals_.begin(), terminals_.end());
  slr_stack_ = {};
  slr_stack_.push({constant::ASTConstant::ROOT_NODE_VALUE, nullptr, 0});
  /*
   * Terminals:
   *    identifier, number, plus (+), times (*), assign (<-), left_paren ((), right_paren ()), semicolon (;)
   *
   * Non-terminals:
   *    Program, StatementList, Statement, Expression, Term, Factor
   *
   * Start symbol:
   *    Program
   *
   * Productions:
   *    Program -> StatementList
   *    StatementList -> Statement StatementList
   *                | ε
   *    Statement -> identifier assign Expression semicolon
   *    Expression -> Expression plus Term
   *                | Term
   *    Term -> Term times Factor
   *        | Factor
   *    Factor -> identifier
   *        | number
   *        | left_paren Expression right_paren
   */
  // Build goto table - state transitions for non-terminals
  goto_table_[0]["Program"] = 1;
  goto_table_[0]["StatementList"] = 2;
  goto_table_[0]["Statement"] = 3;
  goto_table_[2]["Statement"] = 4;
  goto_table_[2]["StatementList"] = 5;
  goto_table_[3]["Statement"] = 4;  // Missing goto for Statement from state 3
  goto_table_[3]["StatementList"] = 5;
  goto_table_[4]["Statement"] = 4;  // Missing goto for Statement from state 4
  goto_table_[4]["StatementList"] = 5;
  goto_table_[6]["Expression"] = 8;
  goto_table_[6]["Term"] = 9;
  goto_table_[6]["Factor"] = 10;
  goto_table_[8]["Term"] = 11;
  goto_table_[9]["Factor"] = 12;
  goto_table_[11]["Factor"] = 12;
  goto_table_[13]["Expression"] = 14;
  goto_table_[13]["Term"] = 9;
  goto_table_[13]["Factor"] = 10;
  goto_table_[18]["Term"] = 11;
  goto_table_[18]["Factor"] = 10;
  goto_table_[19]["Factor"] = 12;

  // Build action table - actions for each state and terminal
  // State 0: Start state
  action_table_[0]["identifier"] = Action(Action::ActionType::SHIFT, 7);
  action_table_[0]["$"] = Action(Action::ActionType::REDUCE, 0, {}, "StatementList");  // reduce by ε production

  // State 1: S' -> Program.
  action_table_[1]["$"] = Action(Action::ActionType::ACCEPT);

  // State 2: Program -> StatementList.
  action_table_[2]["$"] = Action(Action::ActionType::REDUCE, 0, {"StatementList"}, "Program");

  // State 3: StatementList -> Statement .StatementList
  action_table_[3]["identifier"] = Action(Action::ActionType::SHIFT, 7);
  action_table_[3]["$"] = Action(Action::ActionType::REDUCE, 0, {}, "StatementList");  // reduce by ε production

  // State 4: StatementList -> Statement .StatementList
  action_table_[4]["identifier"] = Action(Action::ActionType::SHIFT, 7);
  action_table_[4]["$"] = Action(Action::ActionType::REDUCE, 0, {}, "StatementList");  // reduce by ε production

  // State 5: StatementList -> Statement StatementList.
  action_table_[5]["identifier"] =
      Action(Action::ActionType::REDUCE, 0, {"Statement", "StatementList"}, "StatementList");
  action_table_[5]["$"] = Action(Action::ActionType::REDUCE, 0, {"Statement", "StatementList"}, "StatementList");

  // State 6: Statement -> identifier assign .Expression semicolon
  action_table_[6]["identifier"] = Action(Action::ActionType::SHIFT, 15);
  action_table_[6]["number"] = Action(Action::ActionType::SHIFT, 16);
  action_table_[6]["left_paren"] = Action(Action::ActionType::SHIFT, 13);

  // State 7: Statement -> identifier .assign Expression semicolon
  action_table_[7]["assign"] = Action(Action::ActionType::SHIFT, 6);

  // State 8: Statement -> identifier assign Expression .semicolon
  action_table_[8]["semicolon"] = Action(Action::ActionType::SHIFT, 17);
  action_table_[8]["plus"] = Action(Action::ActionType::SHIFT, 18);

  // State 9: Expression -> Term., Term -> Term .times Factor
  action_table_[9]["semicolon"] = Action(Action::ActionType::REDUCE, 0, {"Term"}, "Expression");
  action_table_[9]["right_paren"] = Action(Action::ActionType::REDUCE, 0, {"Term"}, "Expression");
  action_table_[9]["plus"] = Action(Action::ActionType::REDUCE, 0, {"Term"}, "Expression");
  action_table_[9]["times"] = Action(Action::ActionType::SHIFT, 19);

  // State 10: Term -> Factor.
  action_table_[10]["semicolon"] = Action(Action::ActionType::REDUCE, 0, {"Factor"}, "Term");
  action_table_[10]["right_paren"] = Action(Action::ActionType::REDUCE, 0, {"Factor"}, "Term");
  action_table_[10]["plus"] = Action(Action::ActionType::REDUCE, 0, {"Factor"}, "Term");
  action_table_[10]["times"] = Action(Action::ActionType::REDUCE, 0, {"Factor"}, "Term");

  // State 11: Expression -> Expression plus Term., Term -> Term .times Factor
  action_table_[11]["semicolon"] = Action(Action::ActionType::REDUCE, 0, {"Expression", "plus", "Term"}, "Expression");
  action_table_[11]["right_paren"] =
      Action(Action::ActionType::REDUCE, 0, {"Expression", "plus", "Term"}, "Expression");
  action_table_[11]["plus"] = Action(Action::ActionType::REDUCE, 0, {"Expression", "plus", "Term"}, "Expression");
  action_table_[11]["times"] = Action(Action::ActionType::SHIFT, 19);

  // State 12: Term -> Term times Factor.
  action_table_[12]["semicolon"] = Action(Action::ActionType::REDUCE, 0, {"Term", "times", "Factor"}, "Term");
  action_table_[12]["right_paren"] = Action(Action::ActionType::REDUCE, 0, {"Term", "times", "Factor"}, "Term");
  action_table_[12]["plus"] = Action(Action::ActionType::REDUCE, 0, {"Term", "times", "Factor"}, "Term");
  action_table_[12]["times"] = Action(Action::ActionType::REDUCE, 0, {"Term", "times", "Factor"}, "Term");

  // State 13: Factor -> left_paren .Expression right_paren
  action_table_[13]["identifier"] = Action(Action::ActionType::SHIFT, 15);
  action_table_[13]["number"] = Action(Action::ActionType::SHIFT, 16);
  action_table_[13]["left_paren"] = Action(Action::ActionType::SHIFT, 13);

  // State 14: Factor -> left_paren Expression .right_paren, Expression -> Expression .plus Term
  action_table_[14]["right_paren"] = Action(Action::ActionType::SHIFT, 20);
  action_table_[14]["plus"] = Action(Action::ActionType::SHIFT, 18);

  // State 15: Factor -> identifier.
  action_table_[15]["semicolon"] = Action(Action::ActionType::REDUCE, 0, {"identifier"}, "Factor");
  action_table_[15]["right_paren"] = Action(Action::ActionType::REDUCE, 0, {"identifier"}, "Factor");
  action_table_[15]["plus"] = Action(Action::ActionType::REDUCE, 0, {"identifier"}, "Factor");
  action_table_[15]["times"] = Action(Action::ActionType::REDUCE, 0, {"identifier"}, "Factor");

  // State 16: Factor -> number.
  action_table_[16]["semicolon"] = Action(Action::ActionType::REDUCE, 0, {"number"}, "Factor");
  action_table_[16]["right_paren"] = Action(Action::ActionType::REDUCE, 0, {"number"}, "Factor");
  action_table_[16]["plus"] = Action(Action::ActionType::REDUCE, 0, {"number"}, "Factor");
  action_table_[16]["times"] = Action(Action::ActionType::REDUCE, 0, {"number"}, "Factor");

  // State 17: Statement -> identifier assign Expression semicolon.
  action_table_[17]["identifier"] =
      Action(Action::ActionType::REDUCE, 0, {"identifier", "assign", "Expression", "semicolon"}, "Statement");
  action_table_[17]["$"] =
      Action(Action::ActionType::REDUCE, 0, {"identifier", "assign", "Expression", "semicolon"}, "Statement");

  // State 18: Expression -> Expression plus .Term
  action_table_[18]["identifier"] = Action(Action::ActionType::SHIFT, 15);
  action_table_[18]["number"] = Action(Action::ActionType::SHIFT, 16);
  action_table_[18]["left_paren"] = Action(Action::ActionType::SHIFT, 13);

  // State 19: Term -> Term times .Factor
  action_table_[19]["identifier"] = Action(Action::ActionType::SHIFT, 15);
  action_table_[19]["number"] = Action(Action::ActionType::SHIFT, 16);
  action_table_[19]["left_paren"] = Action(Action::ActionType::SHIFT, 13);

  // State 20: Factor -> left_paren Expression right_paren.
  action_table_[20]["semicolon"] =
      Action(Action::ActionType::REDUCE, 0, {"left_paren", "Expression", "right_paren"}, "Factor");
  action_table_[20]["right_paren"] =
      Action(Action::ActionType::REDUCE, 0, {"left_paren", "Expression", "right_paren"}, "Factor");
  action_table_[20]["plus"] =
      Action(Action::ActionType::REDUCE, 0, {"left_paren", "Expression", "right_paren"}, "Factor");
  action_table_[20]["times"] =
      Action(Action::ActionType::REDUCE, 0, {"left_paren", "Expression", "right_paren"}, "Factor");
}

auto SLRParser::Parse() -> std::shared_ptr<core::AST> {
  lexer_.Reset();
  auto root_node = std::make_shared<core::TreeNode>(constant::ASTConstant::ROOT_NODE_VALUE);
  while (true) {
    if (lexer_.HasNext()) {
      auto token = lexer_.Next();
      auto terminal_node = std::make_shared<core::TreeNode>(token->GetValue());

      std::string token_value = TokenTypeToString(token->GetType());

      bool to_next = false;
      while (!to_next) {
        int current_state = std::get<2>(slr_stack_.top());

        auto state_action_it = action_table_.find(current_state);
        if (state_action_it == action_table_.end()) {
          std::cerr << constant::ErrorMessages::NoActionFoundForState(current_state) << std::endl;
          return nullptr;
        }
        auto action_it = state_action_it->second.find(token_value);
        if (action_it == state_action_it->second.end()) {
          std::cerr << constant::ErrorMessages::NoActionFoundForToken(token_value) << std::endl;
          return nullptr;
        }
        Action action_to_take = action_it->second;

        // Process the token
        switch (action_to_take.type_) {
          case Action::ActionType::SHIFT:
            slr_stack_.push({token_value, terminal_node, action_to_take.state_});
            to_next = true;
            break;
          case Action::ActionType::REDUCE: {
            auto reduce_node = std::make_shared<core::TreeNode>(action_to_take.lhs_);
            std::vector<std::shared_ptr<core::TreeNode>> child_nodes;
            for (size_t i = 0; i < action_to_take.rhs_.size(); ++i) {
              child_nodes.push_back(std::get<1>(slr_stack_.top()));
              slr_stack_.pop();
            }
            for (auto it = child_nodes.rbegin(); it != child_nodes.rend(); ++it) {
              reduce_node->AddChild(*it);
            }
            int top_state = std::get<2>(slr_stack_.top());

            auto goto_it = goto_table_.find(top_state);
            if (goto_it == goto_table_.end() || goto_it->second.find(action_to_take.lhs_) == goto_it->second.end()) {
              std::cerr << "Error: No goto entry for state " << top_state << " and symbol " << action_to_take.lhs_
                        << std::endl;
              return nullptr;
            }

            int new_state = goto_table_[top_state][action_to_take.lhs_];
            slr_stack_.push({action_to_take.lhs_, reduce_node, new_state});
            to_next = false;
            break;
          }
          case Action::ActionType::ACCEPT:
            return BuildAST(root_node);
          case Action::ActionType::REJECT:
            std::cerr << constant::ErrorMessages::ParsingError(token_value, *token) << std::endl;
            break;
        }
      }
    } else {
      int current_state = std::get<2>(slr_stack_.top());

      auto state_action_it = action_table_.find(current_state);
      if (state_action_it == action_table_.end()) {
        std::cerr << constant::ErrorMessages::ParsingError("$", core::Token(core::TokenType::END_OF_FILE, "$"))
                  << std::endl;
        return nullptr;
      }
      auto action_it = state_action_it->second.find("$");
      if (action_it == state_action_it->second.end()) {
        std::cerr << constant::ErrorMessages::NoActionFoundForToken("$") << std::endl;
        return nullptr;
      }
      Action action_to_take = action_it->second;

      // Process EOF token
      if (action_to_take.type_ == Action::ActionType::ACCEPT) {
        // The Program node should be on top of the stack
        auto program_node = std::get<1>(slr_stack_.top());
        return BuildAST(program_node);
      }
      if (action_to_take.type_ == Action::ActionType::REDUCE) {
        auto reduce_node = std::make_shared<core::TreeNode>(action_to_take.lhs_);
        std::vector<std::shared_ptr<core::TreeNode>> child_nodes;
        for (size_t i = 0; i < action_to_take.rhs_.size(); ++i) {
          child_nodes.push_back(std::get<1>(slr_stack_.top()));
          slr_stack_.pop();
        }
        for (auto it = child_nodes.rbegin(); it != child_nodes.rend(); ++it) {
          reduce_node->AddChild(*it);
        }
        int top_state = std::get<2>(slr_stack_.top());

        auto goto_it = goto_table_.find(top_state);
        if (goto_it == goto_table_.end() || goto_it->second.find(action_to_take.lhs_) == goto_it->second.end()) {
          std::cerr << "Error: No goto entry for state " << top_state << " and symbol " << action_to_take.lhs_
                    << std::endl;
          return nullptr;
        }

        int new_state = goto_table_[top_state][action_to_take.lhs_];
        slr_stack_.push({action_to_take.lhs_, reduce_node, new_state});
        // Continue processing EOF with the new state
      } else {
        std::cerr << constant::ErrorMessages::ParsingError("$", core::Token(core::TokenType::END_OF_FILE, "$"))
                  << std::endl;
        return nullptr;
      }
    }
  }
}

auto SLRParser::BuildAST(const std::shared_ptr<core::TreeNode> &parse_tree) -> std::shared_ptr<core::AST> {
  if (!parse_tree) {
    return nullptr;
  }

  auto ast = std::make_shared<core::AST>(program_name_);
  auto root_node = TransformToASTNode(parse_tree);
  ast->SetRoot(root_node);

  return ast;
}

auto SLRParser::IsValidSymbol(const std::string &symbol) const -> bool {
  return symbols_.find(symbol) != symbols_.end() || terminals_.find(symbol) != terminals_.end() ||
         symbol == constant::ASTConstant::END_NODE_VALUE;
}

auto SLRParser::TokenTypeToString(core::TokenType type) -> std::string {
  switch (type) {
    case core::TokenType::IDENTIFIER:
      return "identifier";
    case core::TokenType::NUMBER:
      return "number";
    case core::TokenType::PLUS:
      return "plus";
    case core::TokenType::TIMES:
      return "times";
    case core::TokenType::LEFT_PAREN:
      return "left_paren";
    case core::TokenType::RIGHT_PAREN:
      return "right_paren";
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

// AST transformation helper methods
auto SLRParser::IsTerminal(const std::string &symbol, const std::unordered_set<std::string> &terminals) -> bool {
  return terminals.find(symbol) != terminals.end();
}

auto SLRParser::TransformToASTNode(const std::shared_ptr<core::TreeNode> &parse_node)
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

auto SLRParser::CreateTerminalASTNode(const std::string &symbol) -> std::shared_ptr<core::AST::ASTNode> {
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

auto SLRParser::TransformProgram(const std::shared_ptr<core::TreeNode> &parse_node)
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

void SLRParser::CollectStatements(const std::shared_ptr<core::TreeNode> &parse_node,
                                  const std::shared_ptr<core::AST::ASTNode> &root) {
  // StatementList -> Statement StatementList | ε
  // Due to reverse order, children are: StatementList, Statement
  // Process in reverse order to maintain correct statement sequence
  std::vector<std::shared_ptr<core::TreeNode>> children_vec(parse_node->children_.begin(), parse_node->children_.end());

  for (auto it = children_vec.rbegin(); it != children_vec.rend(); ++it) {
    const auto &child = *it;
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

auto SLRParser::TransformStatement(const std::shared_ptr<core::TreeNode> &parse_node)
    -> std::shared_ptr<core::AST::ASTNode> {
  // Statement -> identifier assign Expression semicolon
  auto assign_node = std::make_shared<core::AST::ASTNode>(core::ASTNodeType::ASSIGN, "<-");

  std::shared_ptr<core::AST::ASTNode> identifier_node = nullptr;
  std::shared_ptr<core::AST::ASTNode> expression_node = nullptr;

  // In SLR parser, children are typically added in the order they appear in production
  // So for "identifier assign Expression semicolon", we expect children in that order
  // But due to how we pop from stack, they are in reverse order:
  // Child 0: semicolon, Child 1: Expression, Child 2: assign, Child 3: identifier
  int child_count = 0;
  for (const auto &child : parse_node->children_) {
    if (child_count == 3) {
      // This should be identifier (at index 3 due to reverse order)
      if (child->children_.empty()) {
        identifier_node = std::make_shared<core::AST::ASTNode>(core::ASTNodeType::IDENTIFIER, child->val_);
      }
    } else if (child_count == 1) {
      // This should be Expression (at index 1 due to reverse order)
      if (child->val_ == "Expression") {
        expression_node = TransformToASTNode(child);
      }
    }
    // Skip assign token (index 2) and semicolon (index 0)
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

auto SLRParser::TransformExpression(const std::shared_ptr<core::TreeNode> &parse_node)
    -> std::shared_ptr<core::AST::ASTNode> {
  // Expression -> Expression plus Term | Term

  if (parse_node->children_.size() == 1) {
    // Expression -> Term
    auto term_node = parse_node->children_.front();
    return TransformToASTNode(term_node);
  }
  if (parse_node->children_.size() == 3) {
    // Expression -> Expression plus Term
    // But children are in reverse order: Term, plus, Expression
    auto it = parse_node->children_.begin();
    auto right_term = TransformToASTNode(*it);  // This is Term (first child due to reverse order)
    ++it;                                       // skip plus operator
    ++it;                                       // get to Expression
    auto left_expr = TransformToASTNode(*it);   // This is Expression (last child due to reverse order)

    auto plus_node = std::make_shared<core::AST::ASTNode>(core::ASTNodeType::PLUS, "+");
    if (left_expr) {
      plus_node->AddChild(left_expr);
    }
    if (right_term) {
      plus_node->AddChild(right_term);
    }
    return plus_node;
  }

  return nullptr;
}

auto SLRParser::TransformTerm(const std::shared_ptr<core::TreeNode> &parse_node)
    -> std::shared_ptr<core::AST::ASTNode> {
  // Term -> Term times Factor | Factor

  if (parse_node->children_.size() == 1) {
    // Term -> Factor
    auto factor_node = parse_node->children_.front();
    return TransformToASTNode(factor_node);
  }
  if (parse_node->children_.size() == 3) {
    // Term -> Term times Factor
    // But children are in reverse order: Factor, times, Term
    auto it = parse_node->children_.begin();
    auto right_factor = TransformToASTNode(*it);  // This is Factor (first child due to reverse order)
    ++it;                                         // skip times operator
    ++it;                                         // get to Term
    auto left_term = TransformToASTNode(*it);     // This is Term (last child due to reverse order)

    auto times_node = std::make_shared<core::AST::ASTNode>(core::ASTNodeType::TIMES, "*");
    if (left_term) {
      times_node->AddChild(left_term);
    }
    if (right_factor) {
      times_node->AddChild(right_factor);
    }
    return times_node;
  }

  return nullptr;
}

auto SLRParser::TransformFactor(const std::shared_ptr<core::TreeNode> &parse_node)
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
