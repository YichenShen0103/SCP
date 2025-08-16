#include "cgen/runtime_environment.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "core/type.h"

namespace scp::cgen {

RuntimeEnvironment::RuntimeEnvironment(const std::shared_ptr<core::TypeEnvironment> &environment) {
  auto symbol_table = environment->GetSymbolTable();
  int allocated_offset = 0;
  while (auto symbol = symbol_table->PopSymbol()) {
    // Stack offset should be byte offset relative to frame pointer (4 bytes per variable)
    symbol_table_.insert(std::make_pair(symbol->name_, std::make_pair(allocated_offset * 4, symbol->type_)));
    allocated_offset++;
  }
}

auto RuntimeEnvironment::GetStackAllocation(const std::string &symbol) -> int {
  auto it = symbol_table_.find(symbol);
  if (it != symbol_table_.end()) {
    return it->second.first;
  }
  throw std::runtime_error("Symbol not found: " + symbol);
}

auto RuntimeEnvironment::GetGlobalStringData(const std::string &symbol) -> std::string {
  auto it = global_string_data_table_.find(symbol);
  if (it != global_string_data_table_.end()) {
    return it->second;
  }
  // Generate new label for string constant
  std::string label = "str_" + std::to_string(global_string_data_table_.size());
  global_string_data_table_[symbol] = label;
  return label;
}

auto RuntimeEnvironment::GetType(const std::string &symbol) -> core::Type {
  auto it = symbol_table_.find(symbol);
  if (it != symbol_table_.end()) {
    return it->second.second;
  }
  throw std::runtime_error("Symbol not found: " + symbol);
}

auto RuntimeEnvironment::GenerateDataSection() const -> std::string {
  std::stringstream code;

  code << ".data" << std::endl;

  // Add string constants
  for (const auto &pair : global_string_data_table_) {
    code << pair.second << ": .asciiz " << pair.first << std::endl;
  }

  // Always add buffers needed for string processing (even if current program doesn't use them)
  code << std::endl << "# Buffers for string operations" << std::endl;
  code << "input_buffer: .space 256" << std::endl;
  code << "concat_buffer: .space 512" << std::endl;
  code << "repeat_buffer: .space 1024" << std::endl;

  return code.str();
}

auto RuntimeEnvironment::AddStringConstant(const std::string &str_literal) -> std::string {
  auto it = global_string_data_table_.find(str_literal);
  if (it != global_string_data_table_.end()) {
    return it->second;
  }

  std::string label = "str_" + std::to_string(global_string_data_table_.size());
  global_string_data_table_[str_literal] = label;
  return label;
}

auto RuntimeEnvironment::GetStackSize() const -> int { return symbol_table_.size(); }

auto RuntimeEnvironment::GetUniqueInputId() -> int { return ++input_counter_; }

}  // namespace scp::cgen
