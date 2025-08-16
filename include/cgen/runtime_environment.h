#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "core/type.h"

namespace scp::cgen {

/**
 * This class represents the runtime environment for code generation.
 * It can be extended to include various runtime configurations and settings.
 */
class RuntimeEnvironment {
 public:
  /**
   * Enum class for different types of registers.
   * This can be extended to include specific registers as needed.
   */
  enum class Register {};

  /**
   * Constructor for the runtime environment.
   */
  explicit RuntimeEnvironment(const std::shared_ptr<core::TypeEnvironment> &environment);

  /**
   * Destructor for the runtime environment.
   */
  ~RuntimeEnvironment() = default;

  /**
   * Get the stack allocation for a given symbol.
   * @param symbol The name of the symbol.
   * @return The stack allocation for the symbol.
   */
  auto GetStackAllocation(const std::string &symbol) -> int;

  /**
   * Get the global string data for a given symbol.
   * @param symbol The name of the symbol.
   * @return The global string data for the symbol.
   */
  auto GetGlobalStringData(const std::string &symbol) -> std::string;

  /**
   * Get the type of a given symbol.
   * @param symbol The name of the symbol.
   * @return The type of the symbol.
   */
  auto GetType(const std::string &symbol) -> core::Type;

  /**
   * Generate the data section for global strings.
   * @return The data section as a string.
   */
  auto GenerateDataSection() const -> std::string;

  /**
   * Add a string constant to the global string table.
   * @param str_literal The string literal (with quotes).
   * @return The label for the string.
   */
  auto AddStringConstant(const std::string &str_literal) -> std::string;

  /**
   * Get the total stack size needed for all variables.
   * @return The number of stack slots needed.
   */
  auto GetStackSize() const -> int;

  /**
   * Get a unique label suffix for input operations.
   * @return A unique number for generating labels.
   */
  auto GetUniqueInputId() -> int;

 private:
  /* Symbol table mapping variable names to their stack allocations and types */
  std::unordered_map<std::string, std::pair<int, core::Type>> symbol_table_;
  /* Global string data table mapping string literals to their labels */
  std::unordered_map<std::string, std::string> global_string_data_table_;
  /* Counter for unique input IDs */
  int input_counter_{0};
};

}  // namespace scp::cgen
