#include "cgen/code_generator.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "core/type.h"

namespace scp::cgen {

CodeGenerator::CodeGenerator(std::shared_ptr<core::AST> ast,
                             const std::shared_ptr<core::TypeEnvironment> &type_environment)
    : ast_(std::move(ast)) {
  runtime_environment_ = std::make_shared<RuntimeEnvironment>(type_environment);
}

auto CodeGenerator::GenerateCode() const -> std::string {
  std::stringstream code;

  // Generate main program code
  code << ast_->GetRoot()->GenerateCode(runtime_environment_);

  // Add string processing utility functions
  code << std::endl << "# String utility functions" << std::endl;
  code << GenerateStringUtilities();

  return code.str();
}

auto CodeGenerator::GenerateStringUtilities() const -> std::string {
  std::stringstream code;

  // String processing utility functions (defined only in .text section)
  code << std::endl << ".text" << std::endl;

  // String concatenation function
  code << "string_concat:" << std::endl;
  code << "    # $a0 = second string address, $a1 = first string address" << std::endl;
  code << "    # result in $a0" << std::endl;
  code << "    move $t0, $a1        # first string address" << std::endl;
  code << "    move $t1, $a0        # second string address" << std::endl;
  code << "    la $a0, concat_buffer # result buffer" << std::endl;
  code << "    move $t2, $a0        # current position in result" << std::endl;
  code << std::endl;
  code << "concat_loop1:" << std::endl;
  code << "    lb $t3, 0($t0)       # load byte from first string" << std::endl;
  code << "    beq $t3, $zero, concat_second # if null terminator, copy second string" << std::endl;
  code << "    sb $t3, 0($t2)       # store byte to result" << std::endl;
  code << "    addiu $t0, $t0, 1    # next char in first string" << std::endl;
  code << "    addiu $t2, $t2, 1    # next position in result" << std::endl;
  code << "    j concat_loop1" << std::endl;
  code << std::endl;
  code << "concat_second:" << std::endl;
  code << "concat_loop2:" << std::endl;
  code << "    lb $t3, 0($t1)       # load byte from second string" << std::endl;
  code << "    sb $t3, 0($t2)       # store byte to result" << std::endl;
  code << "    beq $t3, $zero, concat_done # if null terminator, done" << std::endl;
  code << "    addiu $t1, $t1, 1    # next char in second string" << std::endl;
  code << "    addiu $t2, $t2, 1    # next position in result" << std::endl;
  code << "    j concat_loop2" << std::endl;
  code << std::endl;
  code << "concat_done:" << std::endl;
  code << "    jr $ra               # return" << std::endl;
  code << std::endl;

  // String repeat function
  code << "string_repeat:" << std::endl;
  code << "    # $a1 = string address, $a2 = repeat count" << std::endl;
  code << "    # result in $a0" << std::endl;
  code << "    la $a0, repeat_buffer # result buffer" << std::endl;
  code << "    move $t0, $a0        # current position in result" << std::endl;
  code << "    move $t1, $a2        # repeat counter" << std::endl;
  code << std::endl;
  code << "repeat_outer_loop:" << std::endl;
  code << "    beq $t1, $zero, repeat_done # if counter is 0, done" << std::endl;
  code << "    move $t2, $a1        # reset string pointer" << std::endl;
  code << std::endl;
  code << "repeat_inner_loop:" << std::endl;
  code << "    lb $t3, 0($t2)       # load byte from string" << std::endl;
  code << "    beq $t3, $zero, repeat_next # if null terminator, next iteration" << std::endl;
  code << "    sb $t3, 0($t0)       # store byte to result" << std::endl;
  code << "    addiu $t2, $t2, 1    # next char in string" << std::endl;
  code << "    addiu $t0, $t0, 1    # next position in result" << std::endl;
  code << "    j repeat_inner_loop" << std::endl;
  code << std::endl;
  code << "repeat_next:" << std::endl;
  code << "    addiu $t1, $t1, -1   # decrement counter" << std::endl;
  code << "    j repeat_outer_loop" << std::endl;
  code << std::endl;
  code << "repeat_done:" << std::endl;
  code << "    sb $zero, 0($t0)     # null terminate result" << std::endl;
  code << "    jr $ra               # return" << std::endl;
  code << std::endl;

  // String trim newline function
  code << "string_trim_newline:" << std::endl;
  code << "    # Trim trailing newline from string at address in $a0" << std::endl;
  code << "    move $t0, $a0         # load buffer address from $a0" << std::endl;
  code << std::endl;
  code << "trim_loop:" << std::endl;
  code << "    lb $t1, 0($t0)        # load current character" << std::endl;
  code << "    beq $t1, $zero, trim_done # if null terminator, done" << std::endl;
  code << "    li $t2, 10            # ASCII code for newline (\\n)" << std::endl;
  code << "    beq $t1, $t2, trim_newline # if newline, remove it" << std::endl;
  code << "    li $t2, 13            # ASCII code for carriage return (\\r)" << std::endl;
  code << "    beq $t1, $t2, trim_newline # if carriage return, remove it" << std::endl;
  code << "    addiu $t0, $t0, 1     # next character" << std::endl;
  code << "    j trim_loop" << std::endl;
  code << std::endl;
  code << "trim_newline:" << std::endl;
  code << "    sb $zero, 0($t0)      # replace newline with null terminator" << std::endl;
  code << std::endl;
  code << "trim_done:" << std::endl;
  code << "    jr $ra                # return" << std::endl;

  return code.str();
}

}  // namespace scp::cgen
