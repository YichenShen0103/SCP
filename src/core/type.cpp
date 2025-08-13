#include "core/type.h"

#include <stack>
#include <string>

namespace scp::core {

void TypeEnvironment::SymbolTable::AddSymbol(const std::string &name, Type type) { symbol_stack_.push({name, type}); }

auto TypeEnvironment::SymbolTable::FindSymbolByName(const std::string &name, Symbol &symbol) const -> bool {
  std::stack<Symbol> temp_stack = symbol_stack_;

  while (!temp_stack.empty()) {
    if (temp_stack.top().name_ == name) {
      symbol = temp_stack.top();
      return true;
    }
    temp_stack.pop();
  }

  return false;
}

auto TypeToString(Type type) -> std::string {
  switch (type) {
    case Type::STRING:
      return "string";
    case Type::NUMBER:
      return "number";
    case Type::UNDEFINED:
      return "undefined";
    default:
      return "unknown";
  }
}

void TypeEnvironment::AddSymbol(const std::string &name, Type type) { symbol_table_.AddSymbol(name, type); }

auto TypeEnvironment::GetType(const std::string &name) const -> Type {
  TypeEnvironment::SymbolTable::Symbol symbol{"", Type::UNDEFINED};
  if (symbol_table_.FindSymbolByName(name, symbol)) {
    return symbol.type_;
  }
  return Type::UNDEFINED;  // Return UNDEFINED if the symbol is not found
}

}  // namespace scp::core
