#pragma once

#include <string>

#include "core/token.h"

namespace scp::constant {

/**
 * Error message constants for DFA operations
 */
class ErrorMessages {
 public:
  // Lexer error messages
  static constexpr const char *INVALID_STATE = "Lexer: Invalid state.";
  static constexpr const char *SYMBOL_NOT_IN_ALPHABET = "Lexer: Symbol not in alphabet.";
  static constexpr const char *DFA_RELEASED_CANNOT_SET_FINAL = "Lexer: DFA is released, cannot set final states.";
  static constexpr const char *DFA_NOT_RELEASED_CANNOT_EVALUATE = "Lexer: DFA is not released, cannot evaluate.";
  static constexpr const char *DFA_RELEASED_CANNOT_ADD_TRANSITION = "Lexer: DFA is released, cannot add transitions.";

  // Parser error messages
  static constexpr const char *FAIL_TO_GET_NEXT_TOKEN = "Parser: Failed to get next token.";
  static constexpr const char *UNEXPECTED_END_OF_PARSING = "Parser: Unexpected end of parsing. Stack state: ";
  static constexpr const char *INPUT_NOT_FULLY_CONSUMED = "Parser: Input not fully consumed. Remaining tokens exist.";

  /**
   * Generate an error message for a symbol not in the alphabet with its ASCII value.
   * @param symbol The symbol that is not in the alphabet.
   * @return A formatted error message.
   */
  static auto SymbolNotInAlphabetWithDetails(char symbol) -> std::string {
    return "Lexer: Symbol '" + std::string(1, symbol) + "' (ASCII: " + std::to_string(static_cast<int>(symbol)) +
           ") not in alphabet.";
  }

  /**
   * Generate an error message for a token that is not valid.
   * @param symbol The symbol that is not valid.
   * @param position The position of the symbol in the input.
   * @return A formatted error message.
   */
  static auto NoValidTokenFoundWithDetails(char symbol, size_t position) -> std::string {
    return "Parser: No valid token found at position " + std::to_string(position) + " for character '" +
           std::string(1, symbol) + "'";
  }

  /**
   * Generate an error message for a parsing error.
   * @param current_symbol The current symbol being parsed.
   * @param current_token The current token being processed.
   * @return A formatted error message.
   */
  static auto ParsingError(const std::string &current_symbol, const core::Token &current_token) -> std::string {
    return "Parser: Expected '" + current_symbol + "' but found '" + current_token.GetValue() +
           "' (type: " + ToString(current_token.GetType()) + ")";
  }

  /**
   * Generate an error message suggesting a fix for a missing entry in the parse table.
   * @param current_symbol The current symbol being parsed.
   * @return A formatted error message.
   */
  static auto NoEntriesInParseTable(const std::string &current_symbol) -> std::string {
    return "Parser: No entries in parse table for non-terminal '" + current_symbol + "'";
  }

  /**
   * Generate an error message for a missing production rule.
   * @param current_symbol The current symbol being parsed.
   * @param token_string The token string that was not recognized.
   * @return A formatted error message.
   */
  static auto NoProductionRuleForSymbol(const std::string &current_symbol, const std::string &token_string)
      -> std::string {
    return "Parser: No production rule for non-terminal '" + current_symbol + "' with token '" + token_string + "'";
  }

  /**
   * Generate an error message for a missing action for a specific token.
   * @param token The token for which no action was found.
   * @return A formatted error message.
   */
  static auto NoActionFoundForToken(const std::string &token) -> std::string {
    return "Parser: No action found for token '" + token + "'";
  }

  /**
   * Generate an error message for a missing action in the action table.
   * @param state The state for which no action was found.
   * @return A formatted error message.
   */
  static auto NoActionFoundForState(int state) -> std::string {
    return "Parser: No action found for state " + std::to_string(state);
  }
};

}  // namespace scp::constant
