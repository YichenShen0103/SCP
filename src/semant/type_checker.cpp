#include "semant/type_checker.h"

#include <memory>
#include <utility>

#include "constant/error_messages.h"

namespace scp::semant {

TypeChecker::TypeChecker(std::shared_ptr<core::AST> ast)
    : type_environment_(std::make_shared<core::TypeEnvironment>()), ast_(std::move(ast)) {
  type_environment_->AddSymbol("stdin", core::Type::IN_STREAM);
  type_environment_->AddSymbol("stdout", core::Type::OUT_STREAM);
}

auto TypeChecker::CheckType() -> std::shared_ptr<core::TypeEnvironment> {
  bool has_bug = false;
  ast_->GetRoot()->TypeCheck(type_environment_, has_bug);
  if (has_bug) {
    throw std::runtime_error(constant::ErrorMessages::TYPE_CHECK_FAILED);
  }
  return type_environment_;
}

}  // namespace scp::semant
