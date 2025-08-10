#pragma once

#include <string>
#include <utility>

namespace scp::core {

enum class TokenType { IDENTIFIER, NUMBER, PLUS, TIMES, LEFT_PAREN, RIGHT_PAREN, ASSIGN, SEMICOLON, END_OF_FILE };

auto ToString(TokenType type) -> const char *;

class Token {
 public:
  Token(TokenType type, std::string value) : type_(type), value_(std::move(value)) {}

  auto GetType() const -> TokenType { return type_; }
  auto GetValue() const -> const std::string & { return value_; }

 private:
  TokenType type_;
  std::string value_;
};

}  // namespace scp::core
