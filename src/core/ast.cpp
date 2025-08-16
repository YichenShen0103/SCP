#include "core/ast.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "cgen/runtime_environment.h"
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
      if (type2 == Type::IN_STREAM) {
        type2 = Type::STRING;
      }
      if (type1 == Type::IN_STREAM) {
        has_bug = true;
        std::cerr << constant::ErrorMessages::CANNOT_ASSIGN_TO_INPUT_STREAM << std::endl;
        return Type::UNDEFINED;  // Return UNDEFINED if trying to assign to an input stream
      }
      if (type1 == Type::UNDEFINED) {
        environment->AddSymbol(child1->value_, type2);
        return type2;
      }
      if (type2 == Type::OUT_STREAM) {
        has_bug = true;
        std::cerr << constant::ErrorMessages::OUTPUT_STREAM_AS_RIGHT_VALUE << std::endl;
        return Type::UNDEFINED;  // Return UNDEFINED if trying to assign to an output stream
      }
      if (type1 == type2) {
        return type1;
      }
      if (type1 == Type::OUT_STREAM) {
        return Type::OUT_STREAM;  // If the left-hand side is an output stream, return it
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

auto AST::ASTNode::GenerateCode(const std::shared_ptr<cgen::RuntimeEnvironment> &runtime) const -> std::string {
  std::stringstream code;
  switch (type_) {
    case ASTNodeType::ROOT: {
      // First generate all code to collect string constants
      std::stringstream temp_code;
      for (const auto &child : children_) {
        temp_code << child->GenerateCode(runtime);
      }

      // Generate data section
      std::string data_section = runtime->GenerateDataSection();
      if (!data_section.empty()) {
        code << data_section << std::endl;
      }

      code << ".text" << std::endl << ".globl main" << std::endl << "main:" << std::endl;

      // Initialize stack and frame pointer
      int stack_size = runtime->GetStackSize();
      if (stack_size > 0) {
        code << "    addiu $sp, $sp, -" << (stack_size * 4) << std::endl;  // Allocate stack space for variables
        code << "    move $fp, $sp" << std::endl;                          // Set frame pointer
      }

      code << temp_code.str();

      // Restore stack and exit
      if (stack_size > 0) {
        code << "    addiu $sp, $sp, " << (stack_size * 4) << std::endl;  // Restore stack pointer
      }
      // Add program exit code
      code << "    li $v0, 10" << std::endl << "    syscall" << std::endl;
      break;
    }
    case ASTNodeType::ASSIGN: {
      auto child1 = children_.front();
      auto child2 = children_.back();

      // Special handling for stdout output
      if (child1->GetValue() == "stdout") {
        code << child2->GenerateCode(runtime);
        // Check if it's a string type
        Type expr_type = child2->GetRuntimeType(runtime);
        if (expr_type == Type::STRING) {
          // Print string system call
          code << "    li $v0, 4" << std::endl;
          code << "    syscall" << std::endl;
        } else {
          // Print integer system call
          code << "    li $v0, 1" << std::endl;
          code << "    syscall" << std::endl;
        }
      } else if (child2->GetValue() == "stdin") {
        // Read from stdin and assign to variable
        Type var_type = runtime->GetType(child1->GetValue());
        if (var_type == Type::STRING) {
          // Generate unique labels for this input operation
          int input_id = runtime->GetUniqueInputId();

          // Read string into temporary buffer
          code << "    li $v0, 8" << std::endl;             // read string syscall
          code << "    la $a0, input_buffer" << std::endl;  // temporary buffer address
          code << "    li $a1, 256" << std::endl;           // max length
          code << "    syscall" << std::endl;

          // Calculate length of input string
          code << "    la $t0, input_buffer" << std::endl;
          code << "    move $t1, $t0" << std::endl;
          code << "len_scan_assign" << input_id << ":" << std::endl;
          code << "    lb $t2, 0($t1)" << std::endl;
          code << "    beq $t2, $zero, len_done_assign" << input_id << std::endl;
          code << "    addiu $t1, $t1, 1" << std::endl;
          code << "    j len_scan_assign" << input_id << std::endl;
          code << "len_done_assign" << input_id << ":" << std::endl;
          code << "    subu $t3, $t1, $t0" << std::endl;  // length in $t3

          // Allocate heap memory for string (length + 1 for null terminator)
          code << "    addiu $a0, $t3, 1" << std::endl;  // length + 1
          code << "    li $v0, 9" << std::endl;          // sbrk syscall to allocate memory
          code << "    syscall" << std::endl;
          code << "    move $t4, $v0" << std::endl;  // heap address in $t4

          // Copy string from input_buffer to heap
          code << "    move $t5, $t0" << std::endl;  // source pointer
          code << "copy_loop_assign" << input_id << ":" << std::endl;
          code << "    lb $t6, 0($t5)" << std::endl;
          code << "    sb $t6, 0($t4)" << std::endl;
          code << "    beq $t6, $zero, copy_done_assign" << input_id << std::endl;
          code << "    addiu $t5, $t5, 1" << std::endl;
          code << "    addiu $t4, $t4, 1" << std::endl;
          code << "    j copy_loop_assign" << input_id << std::endl;
          code << "copy_done_assign" << input_id << ":" << std::endl;

          // Trim newline from heap-allocated string
          code << "    subu $a0, $t4, $t3" << std::endl;       // reset to start of heap string
          code << "    jal string_trim_newline" << std::endl;  // Call trim newline function
          code << "    subu $a0, $t4, $t3" << std::endl;       // reload heap string address into $a0
        } else {
          // Read integer
          code << "    li $v0, 5" << std::endl;  // read integer syscall
          code << "    syscall" << std::endl;
          code << "    move $a0, $v0" << std::endl;
        }
        code << "    sw $a0, " << runtime->GetStackAllocation(child1->GetValue()) << "($fp)" << std::endl;
      } else {
        // Normal assignment
        code << child2->GenerateCode(runtime);
        code << "    sw $a0, " << runtime->GetStackAllocation(child1->GetValue()) << "($fp)" << std::endl;
      }
      break;
    }
    case ASTNodeType::NUMBER:
      code << "    li $a0, " << value_ << std::endl;
      break;
    case ASTNodeType::STRING: {
      std::string label = runtime->AddStringConstant(value_);
      code << "    la $a0, " << label << std::endl;
      break;
    }
    case ASTNodeType::PLUS: {
      auto left_child = children_.front();
      auto right_child = children_.back();

      code << left_child->GenerateCode(runtime);
      // Push left operand onto stack (make space first, then store)
      code << "    addiu $sp, $sp, -4" << std::endl;
      code << "    sw $a0, 0($sp)" << std::endl;
      code << right_child->GenerateCode(runtime);

      // Check if it's string concatenation
      Type left_type = left_child->GetRuntimeType(runtime);
      Type right_type = right_child->GetRuntimeType(runtime);

      if (left_type == Type::STRING || right_type == Type::STRING) {
        // String concatenation - call string concatenation function
        code << "    lw $a1, 0($sp)" << std::endl;     // First string address
        code << "    jal string_concat" << std::endl;  // Call concatenation function
      } else {
        // Numeric addition
        code << "    lw $t1, 0($sp)" << std::endl;
        code << "    add $a0, $t1, $a0" << std::endl;
      }
      code << "    addiu $sp, $sp, 4" << std::endl;
      break;
    }
    case ASTNodeType::TIMES: {
      auto left_child = children_.front();
      auto right_child = children_.back();

      code << left_child->GenerateCode(runtime);
      // Push left operand onto stack (make space first, then store)
      code << "    addiu $sp, $sp, -4" << std::endl;
      code << "    sw $a0, 0($sp)" << std::endl;
      code << right_child->GenerateCode(runtime);

      // Check if it's string repetition
      Type left_type = left_child->GetRuntimeType(runtime);

      if (left_type == Type::STRING) {
        // String repetition - call string repetition function
        code << "    lw $a1, 0($sp)" << std::endl;     // String address
        code << "    move $a2, $a0" << std::endl;      // Repetition count
        code << "    jal string_repeat" << std::endl;  // Call repetition function
      } else {
        // Numeric multiplication
        code << "    lw $t1, 0($sp)" << std::endl;
        code << "    mul $a0, $t1, $a0" << std::endl;
      }
      code << "    addiu $sp, $sp, 4" << std::endl;
      break;
    }
    case ASTNodeType::IDENTIFIER: {
      if (value_ == "stdin") {
        // Read from stdin
        if (runtime->GetType(value_) == core::Type::STRING) {
          // Generate unique labels for this input operation
          int input_id = runtime->GetUniqueInputId();

          // Read string into temporary buffer
          code << "    li $v0, 8" << std::endl;  // read string
          code << "    la $a0, input_buffer" << std::endl;
          code << "    li $a1, 256" << std::endl;
          code << "    syscall" << std::endl;

          // Calculate length of input string
          code << "    la $t0, input_buffer" << std::endl;
          code << "    move $t1, $t0" << std::endl;
          code << "len_scan_ident" << input_id << ":" << std::endl;
          code << "    lb $t2, 0($t1)" << std::endl;
          code << "    beq $t2, $zero, len_done_ident" << input_id << std::endl;
          code << "    addiu $t1, $t1, 1" << std::endl;
          code << "    j len_scan_ident" << input_id << std::endl;
          code << "len_done_ident" << input_id << ":" << std::endl;
          code << "    subu $t3, $t1, $t0" << std::endl;  // length in $t3

          // Allocate heap memory for string (length + 1 for null terminator)
          code << "    addiu $a0, $t3, 1" << std::endl;  // length + 1
          code << "    li $v0, 9" << std::endl;          // sbrk syscall to allocate memory
          code << "    syscall" << std::endl;
          code << "    move $t4, $v0" << std::endl;  // heap address in $t4

          // Copy string from input_buffer to heap
          code << "    move $t5, $t0" << std::endl;  // source pointer
          code << "copy_loop_ident" << input_id << ":" << std::endl;
          code << "    lb $t6, 0($t5)" << std::endl;
          code << "    sb $t6, 0($t4)" << std::endl;
          code << "    beq $t6, $zero, copy_done_ident" << input_id << std::endl;
          code << "    addiu $t5, $t5, 1" << std::endl;
          code << "    addiu $t4, $t4, 1" << std::endl;
          code << "    j copy_loop_ident" << input_id << std::endl;
          code << "copy_done_ident" << input_id << ":" << std::endl;

          // Trim newline from heap-allocated string
          code << "    subu $a0, $t4, $t3" << std::endl;       // reset to start of heap string
          code << "    jal string_trim_newline" << std::endl;  // Call trim newline function
          code << "    subu $a0, $t4, $t3" << std::endl;       // reload heap string address into $a0
        } else {
          code << "    li $v0, 5" << std::endl;  // read integer
          code << "    syscall" << std::endl;
          code << "    move $a0, $v0" << std::endl;
        }
      } else {
        code << "    lw $a0, " << runtime->GetStackAllocation(value_) << "($fp)" << std::endl;
      }
      break;
    }
  }
  return code.str();
}

auto AST::ASTNode::GetRuntimeType(const std::shared_ptr<cgen::RuntimeEnvironment> &runtime) const -> Type {
  switch (type_) {
    case ASTNodeType::NUMBER:
      return Type::NUMBER;
    case ASTNodeType::STRING:
      return Type::STRING;
    case ASTNodeType::IDENTIFIER:
      try {
        return runtime->GetType(value_);
      } catch (const std::runtime_error &) {
        return Type::UNDEFINED;
      }
    case ASTNodeType::PLUS:
    case ASTNodeType::TIMES: {
      if (!children_.empty()) {
        auto first_child_type = children_.front()->GetRuntimeType(runtime);
        // If the first operand is a string, the entire expression is string type
        if (first_child_type == Type::STRING) {
          return Type::STRING;
        }
        // Otherwise check the second operand
        if (children_.size() > 1) {
          auto second_child_type = children_.back()->GetRuntimeType(runtime);
          if (second_child_type == Type::STRING) {
            return Type::STRING;
          }
        }
        return Type::NUMBER;
      }
      return Type::UNDEFINED;
    }
    default:
      return Type::UNDEFINED;
  }
}

}  // namespace scp::core
