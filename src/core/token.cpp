#include "core/token.h"

namespace scp::core {

auto ToString(TokenType type) -> const char * {
  switch (type) {
    case TokenType::IDENTIFIER:
      return "IDENTIFIER";
    case TokenType::NUMBER:
      return "NUMBER";
    case TokenType::PLUS:
      return "PLUS";
    case TokenType::TIMES:
      return "TIMES";
    case TokenType::LEFT_PAREN:
      return "LP";
    case TokenType::RIGHT_PAREN:
      return "RP";
    case TokenType::ASSIGN:
      return "ASSIGN";
    case TokenType::SEMICOLON:
      return "SEMICOLON";
    case TokenType::END_OF_FILE:
      return "EOF";
    case TokenType::STRING:
      return "STRING";
    default:
      return "";
  }
}

}  // namespace scp::core
