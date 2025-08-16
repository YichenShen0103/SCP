#include <gtest/gtest.h>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "cgen/code_generator.h"
#include "parser/slr_parser.h"
#include "semant/type_checker.h"

namespace scp::test {

class CodeGeneratorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    parser_ = std::make_unique<parser::SLRParser>("CodeGeneratorTest");
#ifdef TEST_DATA_DIR
    std::string base_path = TEST_DATA_DIR;
#else
    std::string base_path = "test/data";
#endif
    test_data_path_ = base_path + "/code/";
    output_data_path_ = base_path + "/output/";
    temp_asm_file_ = "/tmp/test_output.s";
  }

  void TearDown() override {
    // Clean up temporary file
    std::filesystem::remove(temp_asm_file_);
  }

  std::unique_ptr<parser::SLRParser> parser_;
  std::string test_data_path_;
  std::string output_data_path_;
  std::string temp_asm_file_;

  // Helper function to read file content
  static auto ReadFile(const std::string &filepath) -> std::string {
    std::ifstream file(filepath);
    if (!file.is_open()) {
      return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Remove trailing newlines and whitespace
    while (!content.empty() &&
           (content.back() == '\n' || content.back() == '\r' || content.back() == ' ' || content.back() == '\t')) {
      content.pop_back();
    }

    return content;
  }

  // Helper function to write content to file
  static void WriteFile(const std::string &filepath, const std::string &content) {
    std::ofstream file(filepath);
    file << content;
  }

  // Helper function to execute SPIM and capture output
  auto ExecuteSpim(const std::string &asm_file) -> std::string {
    std::string command = "spim -quiet " + asm_file + " 2>&1";
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
      return "";
    }

    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      result += buffer;
    }
    pclose(pipe);

    // Remove trailing newlines and whitespace
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' ')) {
      result.pop_back();
    }

    return result;
  }

  // Helper function to test code generation end-to-end
  void TestCodeGeneration(const std::string &test_name) {
    // Read input file
    std::string input_file = test_data_path_ + test_name + ".scpl";
    std::string input_content = ReadFile(input_file);
    ASSERT_FALSE(input_content.empty()) << "Failed to read input file: " << input_file;

    // Read expected output file
    std::string output_file = output_data_path_ + test_name + ".txt";
    std::string expected_output = ReadFile(output_file);
    ASSERT_FALSE(expected_output.empty()) << "Failed to read expected output file: " << output_file;

    // Parse the input
    parser_->SetInput(input_content);
    auto ast = parser_->Parse();
    ASSERT_TRUE(ast) << "Failed to parse input: " << input_content;

    // Type check
    semant::TypeChecker type_checker(ast);
    auto type_environment = type_checker.CheckType();
    ASSERT_TRUE(type_environment) << "Type checking failed for: " << input_content;

    // Generate code
    cgen::CodeGenerator code_generator(ast, type_environment);
    std::string generated_code = code_generator.GenerateCode();
    ASSERT_FALSE(generated_code.empty()) << "Code generation failed for: " << input_content;

    // Write generated assembly to temporary file
    WriteFile(temp_asm_file_, generated_code);

    // Execute with SPIM and capture output
    std::string actual_output = ExecuteSpim(temp_asm_file_);

    // Compare with expected output
    EXPECT_EQ(expected_output, actual_output)
        << "Output mismatch for test: " << test_name << "\nInput: " << input_content
        << "\nExpected: " << expected_output << "\nActual: " << actual_output << "\nGenerated ASM: " << generated_code;
  }
};

// Test basic number output
TEST_F(CodeGeneratorTest, BasicNumber) { TestCodeGeneration("cgen_basic_number"); }

// Test basic string output
TEST_F(CodeGeneratorTest, BasicString) { TestCodeGeneration("cgen_basic_string"); }

// Test arithmetic operations
TEST_F(CodeGeneratorTest, Arithmetic) { TestCodeGeneration("cgen_arithmetic"); }

// Test string concatenation
TEST_F(CodeGeneratorTest, StringConcatenation) { TestCodeGeneration("cgen_string_concat"); }

// Test string repetition
TEST_F(CodeGeneratorTest, StringRepetition) { TestCodeGeneration("cgen_string_repeat"); }

// Test multiple variables
TEST_F(CodeGeneratorTest, MultipleVariables) { TestCodeGeneration("cgen_multiple_vars"); }

// Harder arithmetic precedence and nesting
TEST_F(CodeGeneratorTest, OperatorPrecedenceComplex) { TestCodeGeneration("cgen_operator_precedence2"); }
TEST_F(CodeGeneratorTest, NestedParentheses) { TestCodeGeneration("cgen_nested_parentheses"); }

// Harder string operations
TEST_F(CodeGeneratorTest, MultiConcat) { TestCodeGeneration("cgen_string_multi_concat"); }
TEST_F(CodeGeneratorTest, LargeRepeat) { TestCodeGeneration("cgen_string_repeat_large"); }
TEST_F(CodeGeneratorTest, MixConcatRepeatPrecedence) { TestCodeGeneration("cgen_mix_concat_repeat_precedence"); }
TEST_F(CodeGeneratorTest, ConcatParenThenRepeat) { TestCodeGeneration("cgen_concat_paren_repeat"); }
TEST_F(CodeGeneratorTest, RepeatWithComputedCount) { TestCodeGeneration("cgen_repeat_computed_count"); }
TEST_F(CodeGeneratorTest, LongChainConcatWithVars) { TestCodeGeneration("cgen_long_chain_concat"); }

// Test the existing iostream example
TEST_F(CodeGeneratorTest, InputOutput) {
  // This test requires manual input, so we'll just verify code generation works
  std::string input_content = ReadFile(test_data_path_ + "iostream.scpl");
  ASSERT_FALSE(input_content.empty());

  parser_->SetInput(input_content);
  auto ast = parser_->Parse();
  ASSERT_TRUE(ast);

  semant::TypeChecker type_checker(ast);
  auto type_environment = type_checker.CheckType();
  ASSERT_TRUE(type_environment);

  cgen::CodeGenerator code_generator(ast, type_environment);
  std::string generated_code = code_generator.GenerateCode();
  ASSERT_FALSE(generated_code.empty());

  // Verify the generated code contains expected components
  EXPECT_TRUE(generated_code.find(".data") != std::string::npos);
  EXPECT_TRUE(generated_code.find(".text") != std::string::npos);
  EXPECT_TRUE(generated_code.find("main:") != std::string::npos);
  EXPECT_TRUE(generated_code.find("string_concat:") != std::string::npos);
  EXPECT_TRUE(generated_code.find("string_trim_newline:") != std::string::npos);
}

}  // namespace scp::test
