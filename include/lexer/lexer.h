#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "core/token.h"
#include "lexer/dfa.h"

namespace scp::lexer {

/**
 * Lexical analyzer for tokenizing input strings.
 */
class Lexer {
 public:
  /**
   * Constructor for the Lexer.
   */
  Lexer();

  /**
   * Destructor for the Lexer.
   */
  ~Lexer() = default;

  /**
   * Set the input string for the lexer.
   * @param input The input string to tokenize.
   */
  void SetInput(const std::string &input);

  /**
   * Get the next token from the input stream.
   * @return The next token, or std::nullopt if no more tokens are available.
   */
  auto Next() -> std::optional<core::Token>;

  /**
   * Check if there are more tokens to read.
   * @return True if there are more tokens, false otherwise.
   */
  auto HasNext() const -> bool;

  /**
   * Reset the lexer to start from the beginning.
   */
  void Reset();

  /**
   * Tokenize the input string and return a vector of tokens.
   * @param input The input string to tokenize.
   * @return A vector of tokens extracted from the input string.
   */
  auto Tokenize(const std::string &input) -> std::vector<core::Token>;

 private:
  // Input string and current position for streaming
  /* The input string to tokenize */
  std::string input_;
  /* The current position in the input string */
  size_t current_pos_{0};

  /* DFA for all tokens */
  std::unique_ptr<DeterministicFiniteAutomata> number_dfa_;
  std::unique_ptr<DeterministicFiniteAutomata> identifier_dfa_;
  std::unique_ptr<DeterministicFiniteAutomata> plus_dfa_;
  std::unique_ptr<DeterministicFiniteAutomata> left_paren_dfa_;
  std::unique_ptr<DeterministicFiniteAutomata> right_paren_dfa_;
  std::unique_ptr<DeterministicFiniteAutomata> times_dfa_;
  std::unique_ptr<DeterministicFiniteAutomata> assign_dfa_;
  std::unique_ptr<DeterministicFiniteAutomata> semicolon_dfa_;

  /* List of all DFAs used in the lexer */
  std::vector<DeterministicFiniteAutomata *> dfa_list_;

  /* Vector to track which DFAs are still active during tokenization */
  std::vector<bool> survival_list_;

  /**
   * Initialize the DFAs with their respective transitions.
   */
  void InitializeDFAs();

  /**
   * Setup the DFA for all tokens.
   */
  void SetupNumberDFA();      // (^[0-9]+$)
  void SetupIdentifierDFA();  // (^[A-Za-z_][A-Za-z0-9_]*$)
  void SetupPlusDFA();        // (^\+$)
  void SetupLeftParenDFA();   // (^\($)
  void SetupRightParenDFA();  // (^\)$)
  void SetupTimesDFA();       // (^\*$)
  void SetupAssignDFA();      // (^\=$)
  void SetupSemicolonDFA();   // (^\;$)

  /**
   * Skip whitespace characters at the current position.
   */
  void SkipWhitespace();

  /**
   * Get the next token from the input stream.
   * @param token The token to populate with the next token information.
   * @return True if a token was successfully retrieved, false otherwise.
   */
  auto GetNextToken(core::Token &token) -> bool;
};

}  // namespace scp::lexer
