#include "core/symbol_table.h"

#include <stack>
#include <string>

namespace scp::core {

void SymbolTable::AddSymbol(const std::string &name, int value) { symbol_stack_.push(Symbol(name, value)); }

auto SymbolTable::FindSymbol(const std::string &name) const -> int {
  std::stack<Symbol> temp_stack = symbol_stack_;

  while (!temp_stack.empty()) {
    if (temp_stack.top().name_ == name) {
      return temp_stack.top().value_;
    }
    temp_stack.pop();
  }

  return -1;
}

}  // namespace scp::core
