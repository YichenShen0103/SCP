#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "core/token.h"
#include "lexer/dfa.h"

namespace scp::lexer {

class Lexer {
 public:
  Lexer();
  ~Lexer() = default;

  // Set input string for streaming tokenization
  void SetInput(const std::string &input);

  // Get next token from the input stream
  // Returns std::nullopt when no more tokens available
  auto Next() -> std::optional<core::Token>;

  // Check if there are more tokens to read
  auto HasNext() const -> bool;

  // Reset the lexer to start from beginning
  void Reset();

  // Tokenize the input string and return a vector of tokens (legacy method)
  auto Tokenize(const std::string &input) -> std::vector<core::Token>;

 private:
  // Input string and current position for streaming
  std::string input_;
  size_t current_pos_{0};

  // DFA instances for different token types
  std::unique_ptr<DeterministicFiniteAutomata> number_dfa_;
  std::unique_ptr<DeterministicFiniteAutomata> identifier_dfa_;
  std::unique_ptr<DeterministicFiniteAutomata> plus_dfa_;
  std::unique_ptr<DeterministicFiniteAutomata> left_paren_dfa_;
  std::unique_ptr<DeterministicFiniteAutomata> right_paren_dfa_;
  std::unique_ptr<DeterministicFiniteAutomata> times_dfa_;
  std::unique_ptr<DeterministicFiniteAutomata> assign_dfa_;
  std::unique_ptr<DeterministicFiniteAutomata> semicolon_dfa_;

  // Vector to store all DFAs for easier iteration
  std::vector<DeterministicFiniteAutomata *> dfa_list_;

  // Vector to track which DFAs are still active during tokenization
  std::vector<bool> survival_list_;

  // Initialize the DFAs with their respective transitions
  void InitializeDFAs();

  // Setup NUMBER DFA transitions (^[0-9]+$)
  void SetupNumberDFA();

  // Setup IDENTIFIER DFA transitions (^[A-Za-z_][A-Za-z0-9_]*$)
  void SetupIdentifierDFA();

  // Setup PLUS DFA transitions (^\+$)
  void SetupPlusDFA();

  // Setup LEFT_PAREN DFA transitions (^\($)
  void SetupLeftParenDFA();

  // Setup RIGHT_PAREN DFA transitions (^\)$)
  void SetupRightParenDFA();

  // Setup TIMES DFA transitions (^\*$)
  void SetupTimesDFA();

  // Setup ASSIGN DFA transitions (^<-$)
  void SetupAssignDFA();

  // Setup SEMICOLON DFA transitions (^;$)
  void SetupSemicolonDFA();

  // Skip whitespace characters at current position
  void SkipWhitespace();

  // Get next token starting from current position
  auto GetNextToken(core::Token &token) -> bool;
};

}  // namespace scp::lexer
