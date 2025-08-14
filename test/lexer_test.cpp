#include <gtest/gtest.h>
#include <fstream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "core/token.h"
#include "lexer/lexer.h"

namespace scp::test {

struct ExpectedToken {
  core::TokenType type_;
  std::string value_;
};

class LexerTest : public ::testing::Test {
 protected:
  void SetUp() override { lexer_ = std::make_unique<lexer::Lexer>(); }

  std::unique_ptr<lexer::Lexer> lexer_;

  // Helper function to read file content
  static auto ReadTestFile(const std::string &filename) -> std::string {
    std::ifstream file("/Users/shenyc/code/compiler/test/data/code/" + filename);
    if (!file.is_open()) {
      return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
  }

  // Helper function to read expected token file
  auto ReadExpectedTokens(const std::string &filename) -> std::vector<ExpectedToken> {
    std::ifstream file("/Users/shenyc/code/compiler/test/data/token/" + filename);
    std::vector<ExpectedToken> expected_tokens;

    if (!file.is_open()) {
      return expected_tokens;
    }

    std::string line;
    std::regex token_regex(R"(Token \d+: (\w+) = \"(.*)\")");
    std::smatch match;

    while (std::getline(file, line)) {
      if (std::regex_search(line, match, token_regex)) {
        std::string type_str = match[1].str();
        std::string value = match[2].str();

        core::TokenType type;
        if (type_str == "IDENTIFIER") {
          type = core::TokenType::IDENTIFIER;
        } else if (type_str == "NUMBER") {
          type = core::TokenType::NUMBER;
        } else if (type_str == "PLUS") {
          type = core::TokenType::PLUS;
        } else if (type_str == "TIMES") {
          type = core::TokenType::TIMES;
        } else if (type_str == "LP" || type_str == "LEFT_PAREN") {
          type = core::TokenType::LEFT_PAREN;
        } else if (type_str == "RP" || type_str == "RIGHT_PAREN") {
          type = core::TokenType::RIGHT_PAREN;
        } else if (type_str == "ASSIGN") {
          type = core::TokenType::ASSIGN;
        } else if (type_str == "SEMICOLON") {
          type = core::TokenType::SEMICOLON;
        } else if (type_str == "STRING") {
          type = core::TokenType::STRING;
        } else if (type_str == "END_OF_FILE") {
          type = core::TokenType::END_OF_FILE;
        } else {
          continue;  // Skip unknown token types
        }

        expected_tokens.push_back({type, value});
      }
    }

    return expected_tokens;
  }

  // Helper function to tokenize and return vector of tokens
  auto TokenizeInput(const std::string &input) -> std::vector<core::Token> { return lexer_->Tokenize(input); }

  // Helper function to verify token
  void VerifyToken(const core::Token &token, core::TokenType expected_type, const std::string &expected_value) {
    EXPECT_EQ(token.GetType(), expected_type);
    EXPECT_EQ(token.GetValue(), expected_value);
  }

  // Helper function to test file against expected tokens
  void TestFileAgainstExpected(const std::string &code_filename, const std::string &token_filename) {
    std::string content = ReadTestFile(code_filename);
    ASSERT_FALSE(content.empty()) << "Failed to read test file: " << code_filename;

    auto expected_tokens = ReadExpectedTokens(token_filename);
    ASSERT_FALSE(expected_tokens.empty()) << "Failed to read expected tokens from: " << token_filename;

    auto actual_tokens = TokenizeInput(content);

    ASSERT_EQ(actual_tokens.size(), expected_tokens.size()) << "Token count mismatch for " << code_filename;

    for (size_t i = 0; i < actual_tokens.size(); ++i) {
      EXPECT_EQ(actual_tokens[i].GetType(), expected_tokens[i].type_)
          << "Token " << (i + 1) << " type mismatch in " << code_filename;
      EXPECT_EQ(actual_tokens[i].GetValue(), expected_tokens[i].value_)
          << "Token " << (i + 1) << " value mismatch in " << code_filename;
    }
  }
};

// Test using expected token files - Basic cases
TEST_F(LexerTest, SimpleNumber) { TestFileAgainstExpected("simple_number.scpl", "simple_number.token"); }

TEST_F(LexerTest, SimpleIdentifier) { TestFileAgainstExpected("simple_identifier.scpl", "simple_identifier.token"); }

TEST_F(LexerTest, SimpleAddition) { TestFileAgainstExpected("simple_addition.scpl", "simple_addition.token"); }

TEST_F(LexerTest, Expression) { TestFileAgainstExpected("expression.scpl", "expression.token"); }

TEST_F(LexerTest, IdentifierVariations) {
  TestFileAgainstExpected("identifier_variations.scpl", "identifier_variations.token");
}

TEST_F(LexerTest, NumberVariations) { TestFileAgainstExpected("number_variations.scpl", "number_variations.token"); }

TEST_F(LexerTest, MixedTokens) { TestFileAgainstExpected("mixed_tokens.scpl", "mixed_tokens.token"); }

TEST_F(LexerTest, Parentheses) { TestFileAgainstExpected("parentheses.scpl", "parentheses.token"); }

TEST_F(LexerTest, OperatorPrecedence) {
  TestFileAgainstExpected("operator_precedence.scpl", "operator_precedence.token");
}

TEST_F(LexerTest, MultipleStatements) {
  TestFileAgainstExpected("multiple_statements.scpl", "multiple_statements.token");
}

TEST_F(LexerTest, WhitespaceVariations) {
  TestFileAgainstExpected("whitespace_variations.scpl", "whitespace_variations.token");
}

TEST_F(LexerTest, LeftAssociative) { TestFileAgainstExpected("left_associative.scpl", "left_associative.token"); }

TEST_F(LexerTest, String) { TestFileAgainstExpected("string.scpl", "string.token"); }

// Test escape sequences in strings
TEST_F(LexerTest, EscapeSequences) { TestFileAgainstExpected("escape_sequences.scpl", "escape_sequences.token"); }

// Test invalid escape sequences
TEST_F(LexerTest, InvalidEscape) { TestFileAgainstExpected("invalid_escape.scpl", "invalid_escape.token"); }

// Test mixed normal and escape strings
TEST_F(LexerTest, MixedEscape) { TestFileAgainstExpected("mixed_escape.scpl", "mixed_escape.token"); }

TEST_F(LexerTest, RealCode) { TestFileAgainstExpected("real_code.scpl", "real_code.token"); }

TEST_F(LexerTest, WhitespaceOnly) {
  std::string content = ReadTestFile("whitespace_only.scpl");
  auto tokens = TokenizeInput(content);
  EXPECT_EQ(tokens.size(), 0) << "Whitespace-only file should produce no tokens";
}

// Test invalid token handling - expect partial parsing before error
TEST_F(LexerTest, InvalidTokenHandling) {
  std::string content = ReadTestFile("invalid_tokens.scpl");
  if (!content.empty()) {
    auto tokens = TokenizeInput(content);

    // Should get some valid tokens before hitting invalid characters
    EXPECT_GT(tokens.size(), 0) << "Should parse some valid tokens before errors";

    // Check for specific expected tokens that should be parsed successfully
    if (!tokens.empty()) {
      VerifyToken(tokens[0], core::TokenType::NUMBER, "123");
    }
    if (tokens.size() >= 2) {
      VerifyToken(tokens[1], core::TokenType::IDENTIFIER, "abc");
    }
    if (tokens.size() >= 3) {
      VerifyToken(tokens[2], core::TokenType::IDENTIFIER, "_valid123invalid");
    }
    if (tokens.size() >= 4) {
      VerifyToken(tokens[3], core::TokenType::IDENTIFIER, "var");
    }
  }
}

// Test empty file handling
TEST_F(LexerTest, EmptyFile) {
  std::string content = ReadTestFile("empty.scpl");
  auto tokens = TokenizeInput(content);
  EXPECT_EQ(tokens.size(), 0) << "Empty file should produce no tokens";
}

// Test whitespace handling
TEST_F(LexerTest, WhitespaceHandling) {
  auto tokens = TokenizeInput("  hello   123  ");
  ASSERT_EQ(tokens.size(), 2);
  VerifyToken(tokens[0], core::TokenType::IDENTIFIER, "hello");
  VerifyToken(tokens[1], core::TokenType::NUMBER, "123");

  tokens = TokenizeInput("\thello\t123\t");
  ASSERT_EQ(tokens.size(), 2);
  VerifyToken(tokens[0], core::TokenType::IDENTIFIER, "hello");
  VerifyToken(tokens[1], core::TokenType::NUMBER, "123");
}

// Test streaming interface consistency
TEST_F(LexerTest, StreamingInterface) {
  std::string input = "hello 123 world";

  // Get tokens using batch interface
  auto batch_tokens = TokenizeInput(input);

  // Get tokens using streaming interface
  lexer_->SetInput(input);
  std::vector<core::Token> stream_tokens;
  while (lexer_->HasNext()) {
    auto token = lexer_->Next();
    if (token.has_value()) {
      stream_tokens.push_back(*token);
    }
  }

  // Should produce identical results
  ASSERT_EQ(batch_tokens.size(), stream_tokens.size());
  for (size_t i = 0; i < batch_tokens.size(); ++i) {
    EXPECT_EQ(batch_tokens[i].GetType(), stream_tokens[i].GetType());
    EXPECT_EQ(batch_tokens[i].GetValue(), stream_tokens[i].GetValue());
  }
}

// Test reset functionality
TEST_F(LexerTest, ResetFunctionality) {
  lexer_->SetInput("abc 123");

  auto first = lexer_->Next();
  ASSERT_TRUE(first.has_value());
  VerifyToken(*first, core::TokenType::IDENTIFIER, "abc");

  lexer_->Reset();
  EXPECT_TRUE(lexer_->HasNext());

  auto first_again = lexer_->Next();
  ASSERT_TRUE(first_again.has_value());
  VerifyToken(*first_again, core::TokenType::IDENTIFIER, "abc");
}

// Test specific token types
TEST_F(LexerTest, SpecificTokenTypes) {
  // Test assignment operator
  auto tokens = TokenizeInput("<-");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::ASSIGN, "<-");

  // Test plus operator
  tokens = TokenizeInput("+");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::PLUS, "+");

  // Test times operator
  tokens = TokenizeInput("*");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::TIMES, "*");

  // Test parentheses
  tokens = TokenizeInput("()");
  ASSERT_EQ(tokens.size(), 2);
  VerifyToken(tokens[0], core::TokenType::LEFT_PAREN, "(");
  VerifyToken(tokens[1], core::TokenType::RIGHT_PAREN, ")");

  // Test semicolon
  tokens = TokenizeInput(";");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::SEMICOLON, ";");
}

// Test edge cases
TEST_F(LexerTest, EdgeCases) {
  // Single character identifier
  auto tokens = TokenizeInput("a");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::IDENTIFIER, "a");

  // Single digit number
  tokens = TokenizeInput("9");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::NUMBER, "9");

  // Underscore identifier
  tokens = TokenizeInput("_");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::IDENTIFIER, "_");

  // Zero
  tokens = TokenizeInput("0");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::NUMBER, "0");
}

// Test boundary conditions
TEST_F(LexerTest, BoundaryConditions) {
  // Test very long identifier
  std::string long_id(1000, 'a');
  auto tokens = TokenizeInput(long_id);
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::IDENTIFIER, long_id);

  // Test very long number
  std::string long_num(1000, '9');
  tokens = TokenizeInput(long_num);
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::NUMBER, long_num);
}

}  // namespace scp::test
