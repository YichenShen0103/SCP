#include "core/ast.h"

#include <iostream>
#include <memory>

#include "constant/error_messages.h"
#include "core/type.h"

namespace scp::core {

auto AST::ASTNode::TypeCheck(const std::shared_ptr<TypeEnvironment> &environment, bool &has_bug) const -> Type {
  switch (type_) {
    case ASTNodeType::ROOT:
      for (auto &child : children_) {
        child->TypeCheck(environment, has_bug);
      }
      return Type::UNDEFINED;  // Root node does not have a type
    case ASTNodeType::ASSIGN: {
      if (children_.size() != 2) {
        throw std::runtime_error(constant::ErrorMessages::Panic("Invalid number of children for AST node"));
      }
      auto child1 = children_.front();
      auto child2 = children_.back();
      Type type1 = environment->GetType(child1->value_);
      Type type2 = child2->TypeCheck(environment, has_bug);
      if (type1 == Type::UNDEFINED) {
        environment->AddSymbol(child1->value_, type2);
        return type2;
      }
      if (type1 == type2) {
        return type1;
      }
      has_bug = true;
      std::cerr << constant::ErrorMessages::TypeCannotAssign(child1->GetValue(), TypeToString(type1),
                                                             TypeToString(type2))
                << std::endl;
      return type1;  // Return the type of the left-hand side
    }
    case ASTNodeType::IDENTIFIER: {
      Type type = environment->GetType(value_);
      if (type == Type::UNDEFINED) {
        has_bug = true;
        std::cerr << constant::ErrorMessages::UseVariableBeforeDeclaration(value_) << std::endl;
        return Type::UNDEFINED;  // Return UNDEFINED if the variable is not found
      }
      return type;
    }
    case ASTNodeType::TIMES: {
      if (children_.size() != 2) {
        throw std::runtime_error(constant::ErrorMessages::Panic("Invalid number of children for AST node"));
      }
      auto child1 = children_.front();
      auto child2 = children_.back();
      Type type1 = child1->TypeCheck(environment, has_bug);
      Type type2 = child2->TypeCheck(environment, has_bug);
      if (type1 == Type::NUMBER && type2 == Type::NUMBER) {
        return Type::NUMBER;
      }
      if (type1 == Type::NUMBER && type2 == Type::STRING) {
        return Type::STRING;
      }
      if (type1 == Type::STRING && type2 == Type::NUMBER) {
        return Type::STRING;
      }
      has_bug = true;
      std::cerr << constant::ErrorMessages::TypeCannotTime(TypeToString(type1), TypeToString(type2)) << std::endl;
      return Type::UNDEFINED;  // Return UNDEFINED if the types do not match
    }
    case ASTNodeType::PLUS: {
      if (children_.size() != 2) {
        throw std::runtime_error(constant::ErrorMessages::Panic("Invalid number of children for AST node"));
      }
      auto child1 = children_.front();
      auto child2 = children_.back();
      Type type1 = child1->TypeCheck(environment, has_bug);
      Type type2 = child2->TypeCheck(environment, has_bug);
      if (type1 == Type::NUMBER && type2 == Type::NUMBER) {
        return Type::NUMBER;
      }
      if (type1 == Type::STRING && type2 == Type::STRING) {
        return Type::STRING;
      }
      has_bug = true;
      std::cerr << constant::ErrorMessages::TypeCannotAdd(TypeToString(type1), TypeToString(type2)) << std::endl;
      return Type::UNDEFINED;  // Return UNDEFINED if the types do not match
    }
    case ASTNodeType::NUMBER:
      return Type::NUMBER;
    case ASTNodeType::STRING:
      return Type::STRING;
  }
}

}  // namespace scp::core
