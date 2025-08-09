#include "parser/ll1_parser.h"

#include <iostream>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "core/ast.h"
#include "core/token.h"

namespace scp::parser {

// Help Function
auto IsTerminal(const std::string &symbol, const std::unordered_set<std::string> &terminals) -> bool {
  return terminals.find(symbol) != terminals.end();
}

auto Term(const core::Token &token, const std::string &symbol) -> bool { return ToString(token.GetType()) == symbol; }

// LL1Parser Implementation
void LL1Parser::Init(const char *grammarFile) {}

/**
 * Parses the input using the LL(1) parsing algorithm.
 * Handles epsilon productions, error recovery, and proper stack management.
 */
auto LL1Parser::Parse() -> std::shared_ptr<core::TreeNode> {
  // Initialize parse tree root
  std::shared_ptr<core::TreeNode> parse_root = std::make_shared<core::TreeNode>(0);  // Use 0 as root value

  // Initialize parsing stack with end marker and start symbol
  parse_stack_.push(std::make_pair("$", std::shared_ptr<core::TreeNode>()));
  parse_stack_.push(std::make_pair("S", parse_root));  // Start symbol

  core::Token current_token = core::Token(core::TokenType::IDENTIFIER, "");  // Initialize with empty token
  bool token_consumed = true;  // Flag to track if current token needs to be consumed

  while (!parse_stack_.empty() && parse_stack_.top().first != "$") {
    // Get current token if needed
    if (token_consumed) {
      if (!lexer_.HasNext()) {
        // No more tokens, use EOF marker
        current_token = core::Token(core::TokenType::IDENTIFIER, "$");  // EOF token
      } else {
        std::optional<core::Token> token_opt = lexer_.Next();
        if (!token_opt.has_value()) {
          std::cerr << "Error: Failed to get next token" << std::endl;
          return {};
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
        // Match found, consume token and pop from stack
        parse_stack_.pop();
        token_consumed = true;

        // Update tree node value if it's a meaningful terminal
        if (current_tree_node) {
          // Store token information in the node
          if (current_symbol == "id") {
            current_tree_node->val_ = 1;  // Identifier
          } else if (current_symbol == "int") {
            current_tree_node->val_ = 2;  // Number
          }
        }
      } else {
        std::cerr << "Error: Expected '" << current_symbol << "' but found '" << current_token.GetValue()
                  << "' (type: " << ToString(current_token.GetType()) << ")" << std::endl;
        return {};
      }
    } else {
      // Non-terminal processing
      auto non_terminal_it = parse_table_.find(current_symbol);
      if (non_terminal_it == parse_table_.end()) {
        std::cerr << "Error: No entries in parse table for non-terminal '" << current_symbol << "'" << std::endl;
        return {};
      }

      std::string token_string = ToString(current_token.GetType());
      auto production_it = non_terminal_it->second.find(token_string);

      // If no production found for current token, try EOF case
      if (production_it == non_terminal_it->second.end() && token_string != "$") {
        production_it = non_terminal_it->second.find("$");
      }

      if (production_it == non_terminal_it->second.end()) {
        std::cerr << "Error: No production rule for non-terminal '" << current_symbol << "' with token '"
                  << token_string << "'" << std::endl;
        return {};
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
        std::shared_ptr<core::TreeNode> child_node = std::make_shared<core::TreeNode>(0);
        if (current_tree_node) {
          current_tree_node->AddChild(child_node);
        }

        // Push symbol and its corresponding tree node
        parse_stack_.push(std::make_pair(symbol, child_node));
      }
    }
  }

  // Check if parsing completed successfully
  if (parse_stack_.empty() || parse_stack_.top().first != "$") {
    std::cerr << "Error: Unexpected end of parsing. Stack state: ";
    if (parse_stack_.empty()) {
      std::cerr << "empty";
    } else {
      std::cerr << "top = '" << parse_stack_.top().first << "'";
    }
    std::cerr << std::endl;
    return {};
  }

  // Check if all tokens consumed
  if (lexer_.HasNext()) {
    std::cerr << "Error: Input not fully consumed. Remaining tokens exist." << std::endl;
    return {};
  }

  parse_stack_.pop();  // Remove the end marker "$"
  return parse_root;
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
  return symbols_.find(symbol) != symbols_.end() || terminals_.find(symbol) != terminals_.end() || symbol == "$" ||
         symbol == "ε" || symbol == "epsilon";
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

}  // namespace scp::parser
