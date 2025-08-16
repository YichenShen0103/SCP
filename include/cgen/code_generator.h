#pragma once

#include <memory>
#include <string>

#include "cgen/runtime_environment.h"
#include "core/ast.h"

namespace scp::cgen {

/**
 * This class is responsible for generating code.
 */
class CodeGenerator {
 public:
  /**
   * Constructor for the CodeGenerator.
   * @param ast The abstract syntax tree to generate code from.
   */
  CodeGenerator(std::shared_ptr<core::AST> ast, const std::shared_ptr<core::TypeEnvironment> &type_environment);

  /**
   * Destructor for the CodeGenerator.
   */
  ~CodeGenerator() = default;

  /**
   * Generate code from the AST.
   * @return The generated code as a string.
   */
  auto GenerateCode() const -> std::string;

 private:
  std::shared_ptr<core::AST> ast_;
  std::shared_ptr<RuntimeEnvironment> runtime_environment_;

  /**
   * Generate string utility functions.
   * @return The string utility functions as assembly code.
   */
  auto GenerateStringUtilities() const -> std::string;
};

}  // namespace scp::cgen
