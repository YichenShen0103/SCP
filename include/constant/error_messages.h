#pragma once

#include <string>

namespace scp::constant {

/**
 * Error message constants for DFA operations
 */
class ErrorMessages {
 public:
  static constexpr const char *INVALID_STATE = "Invalid state.";
  static constexpr const char *SYMBOL_NOT_IN_ALPHABET = "Symbol not in alphabet.";
  static constexpr const char *DFA_RELEASED_CANNOT_SET_FINAL = "DFA is released, cannot set final states.";
  static constexpr const char *DFA_NOT_RELEASED_CANNOT_EVALUATE = "DFA is not released, cannot evaluate.";
  static constexpr const char *DFA_RELEASED_CANNOT_ADD_TRANSITION = "DFA is released, cannot add transitions.";

  // Format strings for more complex messages
  static auto SymbolNotInAlphabetWithDetails(char symbol) -> std::string {
    return "Symbol '" + std::string(1, symbol) + "' (ASCII: " + std::to_string(static_cast<int>(symbol)) +
           ") not in alphabet.";
  }

  static auto NoValidTokenFoundWithDetails(char symbol, size_t position) -> std::string {
    return "Warning: No valid token found at position " + std::to_string(position) + " for character '" +
           std::string(1, symbol) + "'";
  }
};

}  // namespace scp::constant
