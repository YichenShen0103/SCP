#include <gtest/gtest.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "core/ast.h"
#include "parser/ll1_parser.h"

namespace scp::test {

class LL1ParserTest : public ::testing::Test {
 protected:
  void SetUp() override { parser_ = std::make_unique<parser::LL1Parser>("LL1ParserTest"); }

  std::unique_ptr<parser::LL1Parser> parser_;

  // Helper function to read file content
  auto ReadTestFile(const std::string &filename) -> std::string {
    std::ifstream file("/Users/shenyc/code/compiler/test/data/code/" + filename);
    if (!file.is_open()) {
      return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
  }

  // Helper function to read expected AST output
  auto ReadExpectedAST(const std::string &filename) -> std::string {
    std::ifstream file("/Users/shenyc/code/compiler/test/data/ast/" + filename);
    if (!file.is_open()) {
      return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
  }

  // Helper function to format AST as string (similar to PrintAST in parser.cpp)
  auto FormatAST(const std::shared_ptr<core::AST::ASTNode> &node, int depth = 0) -> std::string {
    if (!node) {
      std::string result;
      for (int i = 0; i < depth; ++i) {
        result += "  ";
      }
      result += "NULL\n";
      return result;
    }

    std::string result;
    for (int i = 0; i < depth; ++i) {
      result += "  ";
    }

    result += "Type: ";
    switch (node->GetType()) {
      case core::ASTNodeType::ROOT:
        result += "ROOT";
        break;
      case core::ASTNodeType::IDENTIFIER:
        result += "IDENTIFIER";
        break;
      case core::ASTNodeType::NUMBER:
        result += "NUMBER";
        break;
      case core::ASTNodeType::PLUS:
        result += "PLUS";
        break;
      case core::ASTNodeType::TIMES:
        result += "TIMES";
        break;
      case core::ASTNodeType::ASSIGN:
        result += "ASSIGN";
        break;
      case core::ASTNodeType::STRING:
        result += "STRING";
        break;
    }
    result += ", Value: '" + node->GetValue() + "'\n";

    for (const auto &child : node->GetChildren()) {
      result += FormatAST(child, depth + 1);
    }

    return result;
  }

  // Helper function to parse input and return AST
  auto ParseInput(const std::string &input) -> std::shared_ptr<core::AST> {
    parser_->SetInput(input);
    return parser_->Parse();
  }

  // Helper function to compare AST output with expected result
  void VerifyAST(const std::string &input_filename, const std::string &expected_filename) {
    std::string input = ReadTestFile(input_filename);
    ASSERT_FALSE(input.empty()) << "Failed to read input file: " << input_filename;

    std::string expected_output = ReadExpectedAST(expected_filename);
    ASSERT_FALSE(expected_output.empty()) << "Failed to read expected output file: " << expected_filename;

    auto ast = ParseInput(input);
    ASSERT_TRUE(ast != nullptr) << "Parsing failed for input: " << input_filename;

    auto root = ast->GetRoot();
    ASSERT_TRUE(root != nullptr) << "AST root is null for input: " << input_filename;

    std::string actual_output = FormatAST(root);
    EXPECT_EQ(actual_output, expected_output) << "AST mismatch for input: " << input_filename;
  }
};

// Test simple addition parsing
TEST_F(LL1ParserTest, SimpleAddition) { VerifyAST("simple_addition.scpl", "simple_addition.ast"); }

// Test complex expression with parentheses and multiple operators
TEST_F(LL1ParserTest, ComplexExpression) { VerifyAST("expression.scpl", "expression.ast"); }

// Test multiple statements
TEST_F(LL1ParserTest, MultipleStatements) { VerifyAST("multiple_statements.scpl", "multiple_statements.ast"); }

// Test operator precedence (multiplication has higher precedence than addition)
TEST_F(LL1ParserTest, OperatorPrecedence) { VerifyAST("operator_precedence.scpl", "operator_precedence.ast"); }

// Test parentheses overriding operator precedence
TEST_F(LL1ParserTest, ParenthesesPrecedence) { VerifyAST("parentheses.scpl", "parentheses.ast"); }

// Test left associativity
TEST_F(LL1ParserTest, LeftAssociativity) { VerifyAST("left_associative.scpl", "left_associative.ast"); }

// Test string literals
TEST_F(LL1ParserTest, StringLiterals) { VerifyAST("string.scpl", "string.ast"); }

// Test mixed string and variable expressions
TEST_F(LL1ParserTest, MixedStringExpressions) { VerifyAST("mixed_string.scpl", "mixed_string.ast"); }

// Test complex real-world code
TEST_F(LL1ParserTest, RealWorldCode) { VerifyAST("real_code.scpl", "real_code.ast"); }

// Test empty input handling
TEST_F(LL1ParserTest, EmptyInput) {
  std::string input = ReadTestFile("empty.scpl");
  auto ast = ParseInput(input);
  // For empty input, parsing should either fail gracefully or return an empty AST
  // Based on the observed behavior, empty files don't parse successfully
  EXPECT_TRUE(ast == nullptr || (ast->GetRoot() && ast->GetRoot()->GetChildren().empty()));
}

// Test basic AST node creation and structure
TEST_F(LL1ParserTest, ASTNodeStructure) {
  std::string input = "x <- 42;";
  auto ast = ParseInput(input);
  ASSERT_TRUE(ast != nullptr);

  auto root = ast->GetRoot();
  ASSERT_TRUE(root != nullptr);
  EXPECT_EQ(root->GetType(), core::ASTNodeType::ROOT);

  // Should have one child (the assignment)
  const auto &children = root->GetChildren();
  ASSERT_EQ(children.size(), 1);

  auto assignment = children.front();
  EXPECT_EQ(assignment->GetType(), core::ASTNodeType::ASSIGN);
  EXPECT_EQ(assignment->GetValue(), "<-");

  // Assignment should have two children: identifier and number
  const auto &assign_children = assignment->GetChildren();
  ASSERT_EQ(assign_children.size(), 2);

  auto identifier = assign_children.front();
  auto number = *std::next(assign_children.begin());

  EXPECT_EQ(identifier->GetType(), core::ASTNodeType::IDENTIFIER);
  EXPECT_EQ(identifier->GetValue(), "x");

  EXPECT_EQ(number->GetType(), core::ASTNodeType::NUMBER);
  EXPECT_EQ(number->GetValue(), "42");
}

// Test expression tree structure
TEST_F(LL1ParserTest, ExpressionTreeStructure) {
  std::string input = "result <- a + b * c;";
  auto ast = ParseInput(input);
  ASSERT_TRUE(ast != nullptr);

  auto root = ast->GetRoot();
  ASSERT_TRUE(root != nullptr);

  const auto &children = root->GetChildren();
  ASSERT_EQ(children.size(), 1);

  auto assignment = children.front();
  EXPECT_EQ(assignment->GetType(), core::ASTNodeType::ASSIGN);

  const auto &assign_children = assignment->GetChildren();
  ASSERT_EQ(assign_children.size(), 2);

  auto identifier = assign_children.front();
  auto expression = *std::next(assign_children.begin());

  EXPECT_EQ(identifier->GetValue(), "result");

  // The expression should be structured as: a + (b * c) due to precedence
  EXPECT_EQ(expression->GetType(), core::ASTNodeType::PLUS);

  const auto &plus_children = expression->GetChildren();
  ASSERT_EQ(plus_children.size(), 2);

  auto left_operand = plus_children.front();               // 'a'
  auto right_operand = *std::next(plus_children.begin());  // 'b * c'

  EXPECT_EQ(left_operand->GetType(), core::ASTNodeType::IDENTIFIER);
  EXPECT_EQ(left_operand->GetValue(), "a");

  EXPECT_EQ(right_operand->GetType(), core::ASTNodeType::TIMES);

  const auto &times_children = right_operand->GetChildren();
  ASSERT_EQ(times_children.size(), 2);

  auto b_operand = times_children.front();
  auto c_operand = *std::next(times_children.begin());

  EXPECT_EQ(b_operand->GetType(), core::ASTNodeType::IDENTIFIER);
  EXPECT_EQ(b_operand->GetValue(), "b");

  EXPECT_EQ(c_operand->GetType(), core::ASTNodeType::IDENTIFIER);
  EXPECT_EQ(c_operand->GetValue(), "c");
}

// Test invalid input handling
TEST_F(LL1ParserTest, InvalidInputHandling) {
  // Test various invalid inputs
  // Note: The parser might still return an AST object even for invalid inputs,
  // but the parsing should fail (indicated by error messages)
  std::vector<std::string> invalid_inputs = {"a <-",       // Incomplete assignment
                                             "a <- ;",     // Missing expression
                                             "<- 123;",    // Missing identifier
                                             "a < 123;"};  // Wrong operator

  for (const auto &input : invalid_inputs) {
    auto ast = ParseInput(input);
    // For these clearly invalid inputs, we expect parsing to fail
    // However, if the parser returns an AST, it should be empty or have no meaningful content
    if (ast != nullptr) {
      auto root = ast->GetRoot();
      // If there's a root, it should have no valid children for these invalid cases
      if (root != nullptr) {
        const auto &children = root->GetChildren();
        EXPECT_TRUE(children.empty()) << "Expected empty AST for invalid input: " << input;
      }
    }
  }

  // Test inputs that should definitely not be valid statements but might tokenize
  std::vector<std::string> token_only_inputs = {"123",             // Just a number, no assignment
                                                "variable_name"};  // Just an identifier

  for (const auto &input : token_only_inputs) {
    auto ast = ParseInput(input);
    // These should either return null or an empty AST since they're not complete statements
    if (ast != nullptr) {
      auto root = ast->GetRoot();
      if (root != nullptr) {
        const auto &children = root->GetChildren();
        EXPECT_TRUE(children.empty()) << "Expected no valid statements for incomplete input: " << input;
      }
    }
  }
}

// Test AST node types and values
TEST_F(LL1ParserTest, ASTNodeTypes) {
  // Test all possible AST node types
  std::string input = "test <- num1 + num2 * num3;";
  auto ast = ParseInput(input);
  ASSERT_TRUE(ast != nullptr);

  auto root = ast->GetRoot();
  ASSERT_TRUE(root != nullptr);
  EXPECT_EQ(root->GetType(), core::ASTNodeType::ROOT);

  const auto &children = root->GetChildren();
  ASSERT_EQ(children.size(), 1);

  auto assignment = children.front();
  EXPECT_EQ(assignment->GetType(), core::ASTNodeType::ASSIGN);
  EXPECT_EQ(assignment->GetValue(), "<-");

  const auto &assign_children = assignment->GetChildren();
  ASSERT_EQ(assign_children.size(), 2);

  auto identifier = assign_children.front();
  auto expression = *std::next(assign_children.begin());

  EXPECT_EQ(identifier->GetType(), core::ASTNodeType::IDENTIFIER);
  EXPECT_EQ(identifier->GetValue(), "test");

  EXPECT_EQ(expression->GetType(), core::ASTNodeType::PLUS);
  EXPECT_EQ(expression->GetValue(), "+");
}

// Test nested parentheses
TEST_F(LL1ParserTest, NestedParentheses) {
  std::string input = "result <- ((a + b) * c);";
  auto ast = ParseInput(input);
  ASSERT_TRUE(ast != nullptr);

  auto root = ast->GetRoot();
  ASSERT_TRUE(root != nullptr);

  const auto &children = root->GetChildren();
  ASSERT_EQ(children.size(), 1);

  auto assignment = children.front();
  const auto &assign_children = assignment->GetChildren();
  ASSERT_EQ(assign_children.size(), 2);

  auto expression = *std::next(assign_children.begin());
  EXPECT_EQ(expression->GetType(), core::ASTNodeType::TIMES);

  // Verify the structure: (a + b) * c
  const auto &times_children = expression->GetChildren();
  ASSERT_EQ(times_children.size(), 2);

  auto plus_expr = times_children.front();
  auto c_identifier = *std::next(times_children.begin());

  EXPECT_EQ(plus_expr->GetType(), core::ASTNodeType::PLUS);
  EXPECT_EQ(c_identifier->GetType(), core::ASTNodeType::IDENTIFIER);
  EXPECT_EQ(c_identifier->GetValue(), "c");
}

// Test single assignment
TEST_F(LL1ParserTest, SingleAssignment) {
  std::string input = "x <- 1;";
  auto ast = ParseInput(input);
  ASSERT_TRUE(ast != nullptr);

  auto root = ast->GetRoot();
  ASSERT_TRUE(root != nullptr);

  const auto &children = root->GetChildren();
  ASSERT_EQ(children.size(), 1);

  auto assignment = children.front();
  EXPECT_EQ(assignment->GetType(), core::ASTNodeType::ASSIGN);

  const auto &assign_children = assignment->GetChildren();
  ASSERT_EQ(assign_children.size(), 2);

  auto identifier = assign_children.front();
  auto number = *std::next(assign_children.begin());

  EXPECT_EQ(identifier->GetType(), core::ASTNodeType::IDENTIFIER);
  EXPECT_EQ(identifier->GetValue(), "x");
  EXPECT_EQ(number->GetType(), core::ASTNodeType::NUMBER);
  EXPECT_EQ(number->GetValue(), "1");
}

// Test string literal AST structure
TEST_F(LL1ParserTest, StringLiteralStructure) {
  std::string input = "msg <- \"hello world\";";
  auto ast = ParseInput(input);
  ASSERT_TRUE(ast != nullptr);

  auto root = ast->GetRoot();
  ASSERT_TRUE(root != nullptr);
  EXPECT_EQ(root->GetType(), core::ASTNodeType::ROOT);

  // Should have one child (the assignment)
  const auto &children = root->GetChildren();
  ASSERT_EQ(children.size(), 1);

  auto assignment = children.front();
  EXPECT_EQ(assignment->GetType(), core::ASTNodeType::ASSIGN);
  EXPECT_EQ(assignment->GetValue(), "<-");

  // Assignment should have two children: identifier and string
  const auto &assign_children = assignment->GetChildren();
  ASSERT_EQ(assign_children.size(), 2);

  auto identifier = assign_children.front();
  auto string_literal = *std::next(assign_children.begin());

  EXPECT_EQ(identifier->GetType(), core::ASTNodeType::IDENTIFIER);
  EXPECT_EQ(identifier->GetValue(), "msg");

  EXPECT_EQ(string_literal->GetType(), core::ASTNodeType::STRING);
  EXPECT_EQ(string_literal->GetValue(), "\"hello world\"");
}

// Performance test for heavy_test.scpl
TEST_F(LL1ParserTest, PerformanceTest) {
  std::string input = ReadTestFile("heavy_test.scpl");
  ASSERT_FALSE(input.empty()) << "Failed to read heavy_test.scpl";

  auto start_time = std::chrono::high_resolution_clock::now();
  auto ast = ParseInput(input);
  auto end_time = std::chrono::high_resolution_clock::now();

  ASSERT_TRUE(ast != nullptr) << "Parsing failed for heavy_test.scpl";

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  std::cout << "LL1 Parser Performance: Parsed heavy_test.scpl in " << duration.count() << " milliseconds" << std::endl;
}

}  // namespace scp::test
