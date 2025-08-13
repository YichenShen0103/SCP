#include "core/symbol_table.h"

#include <stack>
#include <string>

namespace scp::core {

void SymbolTable::AddSymbol(const std::string &name) { symbol_stack_.push(name); }

auto SymbolTable::FindSymbol(const std::string &name) const -> bool {
  std::stack<std::string> temp_stack = symbol_stack_;

  while (!temp_stack.empty()) {
    if (temp_stack.top() == name) {
      return true;
    }
    temp_stack.pop();
  }

  return false;
}

}  // namespace scp::core
