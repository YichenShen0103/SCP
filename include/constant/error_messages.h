#pragma once

#include <string>

#include "core/token.h"

namespace scp::constant {

/**
 * Error message constants for DFA operations
 */
class ErrorMessages {
 public:
  // Lexer
  static constexpr const char *INVALID_STATE = "Lexer: Invalid state.";
  static constexpr const char *SYMBOL_NOT_IN_ALPHABET = "Lexer: Symbol not in alphabet.";
  static constexpr const char *DFA_RELEASED_CANNOT_SET_FINAL = "Lexer: DFA is released, cannot set final states.";
  static constexpr const char *DFA_NOT_RELEASED_CANNOT_EVALUATE = "Lexer: DFA is not released, cannot evaluate.";
  static constexpr const char *DFA_RELEASED_CANNOT_ADD_TRANSITION = "Lexer: DFA is released, cannot add transitions.";

  // Parser
  static constexpr const char *FAIL_TO_GET_NEXT_TOKEN = "Parser: Failed to get next token.";
  static constexpr const char *UNEXPECTED_END_OF_PARSING = "Parser: Unexpected end of parsing. Stack state: ";
  static constexpr const char *INPUT_NOT_FULLY_CONSUMED = "Parser: Input not fully consumed. Remaining tokens exist.";

  // Format strings for more complex messages
  static auto SymbolNotInAlphabetWithDetails(char symbol) -> std::string {
    return "Parser: Symbol '" + std::string(1, symbol) + "' (ASCII: " + std::to_string(static_cast<int>(symbol)) +
           ") not in alphabet.";
  }

  static auto NoValidTokenFoundWithDetails(char symbol, size_t position) -> std::string {
    return "Parser: No valid token found at position " + std::to_string(position) + " for character '" +
           std::string(1, symbol) + "'";
  }

  static auto ParsingError(const std::string &current_symbol, const core::Token &current_token) -> std::string {
    return "Parser: Expected '" + current_symbol + "' but found '" + current_token.GetValue() +
           "' (type: " + ToString(current_token.GetType()) + ")";
  }

  static auto NoEntriesInParseTable(const std::string &current_symbol) -> std::string {
    return "Parser: No entries in parse table for non-terminal '" + current_symbol + "'";
  }

  static auto NoProductionRuleForSymbol(const std::string &current_symbol, const std::string &token_string)
      -> std::string {
    return "Parser: No production rule for non-terminal '" + current_symbol + "' with token '" + token_string + "'";
  }
};

}  // namespace scp::constant
