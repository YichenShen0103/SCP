#include "semant/type_checker.h"

#include "constant/error_messages.h"

namespace scp::semant {

void TypeChecker::CheckType() {
  bool has_bug = false;
  ast_->GetRoot()->TypeCheck(type_environment_, has_bug);
  if (has_bug) {
    throw std::runtime_error(constant::ErrorMessages::TYPE_CHECK_FAILED);
  }
}

}  // namespace scp::semant
