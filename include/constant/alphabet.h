#pragma once

namespace scp::constant {

/**
 * Alphabet constant class for the lexer
 */
class Alphabet {
 public:
  // Alphabet definitions
  static constexpr const char *DIGIT_ALPHABET = "0123456789";
  static constexpr const char *LETTER_ALPHABET = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static constexpr const char *IDENTIFIER_ALPHABET = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
  static constexpr const char *PLUS_ALPHABET = "+";
  static constexpr const char *TIMES_ALPHABET = "*";
  static constexpr const char *LEFT_PAREN_ALPHABET = "(";
  static constexpr const char *RIGHT_PAREN_ALPHABET = ")";
  static constexpr const char *ASSIGN_ALPHABET = "<-";
  static constexpr const char *SEMICOLON_ALPHABET = ";";
};

}  // namespace scp::constant
