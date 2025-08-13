#pragma once

#include <string>
#include <utility>

namespace scp::core {

/**
 * Enum class for token types
 */
enum class TokenType {
  IDENTIFIER,
  NUMBER,
  PLUS,
  TIMES,
  LEFT_PAREN,
  RIGHT_PAREN,
  ASSIGN,
  SEMICOLON,
  END_OF_FILE,
  STRING
};

/**
 * Convert a token type to a string representation.
 * @param type The token type.
 * @return The string representation of the token type.
 */
auto ToString(TokenType type) -> const char *;

/**
 * Class representing a token in the source code.
 */
class Token {
 public:
  /**
   * Constructor for a token.
   * @param type The type of the token.
   * @param value The value of the token.
   */
  Token(TokenType type, std::string value, int line_pos, int column_pos)
      : type_(type), value_(std::move(value)), position_({line_pos, column_pos}) {}

  /**
   * Destructor for a token.
   */
  ~Token() = default;

  /**
   * Get the type of the token.
   * @return The type of the token.
   */
  auto GetType() const -> TokenType { return type_; }

  /**
   * Get the value of the token.
   * @return The value of the token.
   */
  auto GetValue() const -> const std::string & { return value_; }

  /**
   * Get the position of the token in the source code.
   * @return A pair containing (line, column) position.
   */
  auto GetPosition() const -> const std::pair<int, int> & { return position_; }

  /**
   * Get the line position of the token.
   * @return The line number (1-based).
   */
  auto GetLine() const -> int { return position_.first; }

  /**
   * Get the column position of the token.
   * @return The column number (1-based).
   */
  auto GetColumn() const -> int { return position_.second; }

 private:
  /* The type of the token */
  TokenType type_;
  /* The value of the token */
  std::string value_;
  /* Position in the source code */
  std::pair<int, int> position_;
};

}  // namespace scp::core
