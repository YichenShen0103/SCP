#pragma once

#include <string>

enum class TokenType
{
  IDENTIFIER,
  NUMBER
};

constexpr const char *toString(TokenType type)
{
  switch (type)
  {
  case TokenType::IDENTIFIER:
    return "id";
  case TokenType::NUMBER:
    return "int";
  default:
    return "";
  }
}

class Token
{
public:
  Token(TokenType type, const std::string &value)
      : type_(type), value_(value) {}

  TokenType getType() const { return type_; }
  const std::string &getValue() const { return value_; }

private:
  TokenType type_;
  std::string value_;
};
