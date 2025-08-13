#pragma once

#include <memory>
#include <utility>

#include "core/ast.h"
#include "core/type.h"

namespace scp::semant {

/**
 * The TypeChecker class is responsible for checking the types of expressions in the SCP language.
 */
class TypeChecker {
 public:
  /**
   * Constructor for the TypeChecker.
   */
  explicit TypeChecker(std::shared_ptr<core::AST> ast)
      : type_environment_(std::make_shared<core::TypeEnvironment>()), ast_(std::move(ast)) {}

  /**
   * Default destructor for the TypeChecker.
   */
  ~TypeChecker() = default;

  /**
   * Checks the types of the expressions in the AST.
   */
  void CheckType();

 private:
  /* The type environment for the type checker. */
  std::shared_ptr<core::TypeEnvironment> type_environment_;
  /* The abstract syntax tree being checked. */
  std::shared_ptr<core::AST> ast_;
};

}  // namespace scp::semant
