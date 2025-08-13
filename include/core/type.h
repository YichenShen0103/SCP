#pragma once

#include <stack>
#include <string>
#include <utility>

namespace scp::core {

/**
 * Enumeration representing the different types in the SCP language.
 */
enum class Type { STRING, NUMBER, UNDEFINED };

/**
 * Converts a Type enum to its string representation.
 * @param type The Type enum to convert.
 * @return A string representation of the Type.
 */
auto TypeToString(Type type) -> std::string;

/**
 * The type environment for type checking
 */
class TypeEnvironment {
 public:
  /**
   * The SymbolTable class is used to manage symbols in the SCP language.
   */
  class SymbolTable {
   public:
    /**
     * The structure representing a symbol in the SymbolTable.
     */
    struct Symbol {
      /* The name of the Symbol */
      std::string name_;
      /* The type of the Symbol */
      Type type_;

      /**
       * Constructor of the symbol
       */
      Symbol(std::string name, Type type) : name_(std::move(name)), type_(type) {}
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
     * @param type The type of the symbol.
     */
    void AddSymbol(const std::string &name, Type type);

    /**
     * Find a symbol in the table.
     * @param name The name of the symbol to find.
     * @param symbol The Symbol object to fill if found.
     * @return True if the symbol is found, false otherwise.
     */
    auto FindSymbolByName(const std::string &name, Symbol &symbol) const -> bool;

   private:
    // Internal storage for symbols
    std::stack<Symbol> symbol_stack_;
  };

  /**
   * Default constructor for TypeEnvironment.
   */
  TypeEnvironment() = default;

  /**
   * Add a symbol to the symbol table.
   * @param name The name of the symbol to add.
   */
  void AddSymbol(const std::string &name, Type type);

  /**
   * Get the type of a symbol by its name.
   * @param name The name of the symbol to look up.
   * @return The Type of the symbol.
   */
  auto GetType(const std::string &name) const -> Type;

 private:
  /* The current symbol table */
  SymbolTable symbol_table_;
};

}  // namespace scp::core
