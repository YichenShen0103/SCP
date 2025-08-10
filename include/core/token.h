#pragma once

#include <string>
#include <utility>

namespace scp::core {

/**
 * Enum class for token types
 */
enum class TokenType { IDENTIFIER, NUMBER, PLUS, TIMES, LEFT_PAREN, RIGHT_PAREN, ASSIGN, SEMICOLON, END_OF_FILE };

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
  Token(TokenType type, std::string value) : type_(type), value_(std::move(value)) {}

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

 private:
  /* The type of the token */
  TokenType type_;
  /* The value of the token */
  std::string value_;
};

}  // namespace scp::core
