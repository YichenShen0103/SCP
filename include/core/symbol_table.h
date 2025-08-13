#pragma once

#include <stack>
#include <string>
#include <utility>

namespace scp::core {

class SymbolTable {
 public:
  /**
   * Constructor for the SymbolTable.
   */
  SymbolTable() = default;

  /**
   * Destructor for the SymbolTable.
   */
  ~SymbolTable() = default;

  /**
   * Add a symbol to the table.
   * @param name The name of the symbol.
   */
  void AddSymbol(const std::string &name);

  /**
   * Find a symbol in the table.
   * @param name The name of the symbol to find.
   * @return True if the symbol is found, false otherwise.
   */
  auto FindSymbol(const std::string &name) const -> bool;

 private:
  // Internal storage for symbols
  std::stack<std::string> symbol_stack_;
};

}  // namespace scp::core
