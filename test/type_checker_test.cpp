#include <gtest/gtest.h>
#include <fstream>
#include <memory>
#include <string>

#include "parser/slr_parser.h"
#include "semant/type_checker.h"

namespace scp::test {

class TypeCheckerTest : public ::testing::Test {
 protected:
  void SetUp() override { parser_ = std::make_unique<parser::SLRParser>("TypeCheckerTest"); }

  std::unique_ptr<parser::SLRParser> parser_;

  // Helper function to read file content
  static auto ReadTestFile(const std::string &filename) -> std::string {
    std::ifstream file("/Users/shenyc/code/compiler/test/data/code/" + filename);
    if (!file.is_open()) {
      return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
  }

  // Helper function to parse input and create type checker
  auto CreateTypeChecker(const std::string &input) -> std::unique_ptr<semant::TypeChecker> {
    parser_->SetInput(input);
    auto ast = parser_->Parse();
    return std::make_unique<semant::TypeChecker>(ast);
  }

  // Helper function to test type checking with expected success
  void ExpectTypeCheckSuccess(const std::string &input) {
    auto type_checker = CreateTypeChecker(input);
    EXPECT_NO_THROW(type_checker->CheckType()) << "Type checking should succeed for input: " << input;
  }

  // Helper function to test type checking with expected failure
  void ExpectTypeCheckFailure(const std::string &input) {
    auto type_checker = CreateTypeChecker(input);
    EXPECT_THROW(type_checker->CheckType(), std::runtime_error) << "Type checking should fail for input: " << input;
  }
};

// Test valid number operations
TEST_F(TypeCheckerTest, ValidNumberOperations) {
  // Simple number assignment
  ExpectTypeCheckSuccess("a <- 42;");

  // Number addition
  ExpectTypeCheckSuccess("result <- 1 + 2;");

  // Number multiplication
  ExpectTypeCheckSuccess("result <- 3 * 4;");

  // Complex number expression
  ExpectTypeCheckSuccess("result <- 1 + 2 * 3;");

  // Multiple number assignments
  ExpectTypeCheckSuccess("a <- 10; b <- 20; c <- a + b;");
}

// Test valid string operations
TEST_F(TypeCheckerTest, ValidStringOperations) {
  // Simple string assignment
  ExpectTypeCheckSuccess(R"(greeting <- "hello";)");

  // String concatenation
  ExpectTypeCheckSuccess(R"(message <- "hello" + "world";)");

  // String multiplication by number
  ExpectTypeCheckSuccess(R"(repeated <- "abc" * 3;)");

  // Number multiplication by string
  ExpectTypeCheckSuccess(R"(repeated <- 2 * "xyz";)");
}

// Test mixed valid operations
TEST_F(TypeCheckerTest, ValidMixedOperations) {
  // Assign number then use in expression
  ExpectTypeCheckSuccess("x <- 5; y <- x + 10;");

  // Assign string then use in concatenation
  ExpectTypeCheckSuccess(R"(s1 <- "hello"; s2 <- s1 + "world";)");

  // Complex mixed assignments
  ExpectTypeCheckSuccess(R"(num <- 42; str <- "test"; result <- str * num;)");
}

// Test invalid type combinations
TEST_F(TypeCheckerTest, InvalidTypeOperations) {
  // Cannot add number and string
  ExpectTypeCheckFailure(R"(result <- 1 + "hello";)");

  // Cannot add string and number
  ExpectTypeCheckFailure(R"(result <- "hello" + 1;)");

  // Cannot multiply two strings
  ExpectTypeCheckFailure(R"(result <- "hello" * "world";)");
}

// Test undefined variable usage
TEST_F(TypeCheckerTest, UndefinedVariableUsage) {
  // Use undefined variable in expression
  ExpectTypeCheckFailure("result <- undefined_var + 1;");

  // Use undefined variable in assignment
  ExpectTypeCheckFailure("result <- undefined_var;");

  // Use undefined variable in multiplication
  ExpectTypeCheckFailure("result <- undefined_var * 2;");
}

// Test type reassignment conflicts
TEST_F(TypeCheckerTest, TypeReassignmentConflicts) {
  // Assign number then try to assign string
  ExpectTypeCheckFailure(R"(x <- 42; x <- "hello";)");

  // Assign string then try to assign number
  ExpectTypeCheckFailure(R"(s <- "test"; s <- 123;)");
}

// Test with existing code files
TEST_F(TypeCheckerTest, ExistingCodeFiles) {
  // Test simple addition file
  std::string simple_addition = ReadTestFile("simple_addition.scpl");
  ASSERT_FALSE(simple_addition.empty());
  ExpectTypeCheckSuccess(simple_addition);

  // Test multiple statements file
  std::string multiple_statements = ReadTestFile("multiple_statements.scpl");
  ASSERT_FALSE(multiple_statements.empty());
  ExpectTypeCheckSuccess(multiple_statements);
}

// Test with new type checking test files
TEST_F(TypeCheckerTest, TypeValidationFiles) {
  // Test valid number operations
  std::string valid_numbers = ReadTestFile("type_valid_numbers.scpl");
  ASSERT_FALSE(valid_numbers.empty());
  ExpectTypeCheckSuccess(valid_numbers);

  // Test valid string operations
  std::string valid_strings = ReadTestFile("type_valid_strings.scpl");
  ASSERT_FALSE(valid_strings.empty());
  ExpectTypeCheckSuccess(valid_strings);

  // Test valid mixed operations
  std::string valid_mixed = ReadTestFile("type_valid_mixed.scpl");
  ASSERT_FALSE(valid_mixed.empty());
  ExpectTypeCheckSuccess(valid_mixed);

  // Test complex valid operations
  std::string complex_valid = ReadTestFile("type_complex_valid.scpl");
  ASSERT_FALSE(complex_valid.empty());
  ExpectTypeCheckSuccess(complex_valid);
}

// Test type error files
TEST_F(TypeCheckerTest, TypeErrorFiles) {
  // Test number + string error
  std::string error_add_num_str = ReadTestFile("type_error_add_number_string.scpl");
  ASSERT_FALSE(error_add_num_str.empty());
  ExpectTypeCheckFailure(error_add_num_str);

  // Test string + number error
  std::string error_add_str_num = ReadTestFile("type_error_add_string_number.scpl");
  ASSERT_FALSE(error_add_str_num.empty());
  ExpectTypeCheckFailure(error_add_str_num);

  // Test string * string error
  std::string error_mult_strings = ReadTestFile("type_error_multiply_strings.scpl");
  ASSERT_FALSE(error_mult_strings.empty());
  ExpectTypeCheckFailure(error_mult_strings);

  // Test undefined variable error
  std::string error_undefined = ReadTestFile("type_error_undefined_variable.scpl");
  ASSERT_FALSE(error_undefined.empty());
  ExpectTypeCheckFailure(error_undefined);

  // Test reassignment error
  std::string error_reassign = ReadTestFile("type_error_reassignment.scpl");
  ASSERT_FALSE(error_reassign.empty());
  ExpectTypeCheckFailure(error_reassign);
}

// Test complex expressions
TEST_F(TypeCheckerTest, ComplexExpressions) {
  // Nested arithmetic
  ExpectTypeCheckSuccess("result <- (1 + 2) * (3 + 4);");

  // Multiple variable dependencies
  ExpectTypeCheckSuccess("a <- 1; b <- 2; c <- 3; result <- a + b * c;");

  // String operations with variables
  ExpectTypeCheckSuccess(R"(prefix <- "hello"; suffix <- "world"; message <- prefix + suffix;)");
}

// Test input files
TEST_F(TypeCheckerTest, TypeIOStreamFiles) {
  // Test number + string error
  std::string error_add_num_str = ReadTestFile("iostream.scpl");
  ASSERT_FALSE(error_add_num_str.empty());
  ExpectTypeCheckSuccess(error_add_num_str);

  // Test stdin error
  std::string error_add_str_num = ReadTestFile("error_io_1.scpl");
  ASSERT_FALSE(error_add_str_num.empty());
  ExpectTypeCheckFailure(error_add_str_num);

  // Test stdout error
  std::string error_mult_strings = ReadTestFile("error_io_2.scpl");
  ASSERT_FALSE(error_mult_strings.empty());
  ExpectTypeCheckFailure(error_mult_strings);

  // Test stdin operation error
  std::string error_undefined = ReadTestFile("error_io_3.scpl");
  ASSERT_FALSE(error_undefined.empty());
  ExpectTypeCheckFailure(error_undefined);
}

}  // namespace scp::test
