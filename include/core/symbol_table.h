#pragma once

#include <stack>
#include <string>
#include <utility>

namespace scp::core {

class SymbolTable {
 public:
  /**
   * Symbol structure representing a symbol in the symbol table.
   */
  struct Symbol {
    std::string name_;  // Name of the symbol
    int value_;         // Value of the symbol (if applicable)

    explicit Symbol(std::string name, int value = 0) : name_(std::move(name)), value_(value) {}
  };

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
  void AddSymbol(const std::string &name, int value = 0);

  /**
   * Find a symbol in the table.
   * @param name The name of the symbol to find.
   * @return True if the symbol is found, false otherwise.
   */
  auto FindSymbol(const std::string &name) const -> int;

 private:
  // Internal storage for symbols
  std::stack<Symbol> symbol_stack_;
};

}  // namespace scp::core
