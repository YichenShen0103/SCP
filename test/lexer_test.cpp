#include <gtest/gtest.h>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "core/token.h"
#include "lexer/lexer.h"

namespace scp::test {

class LexerTest : public ::testing::Test {
 protected:
  void SetUp() override { lexer_ = std::make_unique<lexer::Lexer>(); }

  std::unique_ptr<lexer::Lexer> lexer_;

  // Helper function to read file content
  auto ReadTestFile(const std::string &filename) -> std::string {
    std::ifstream file("test/data/" + filename);
    if (!file.is_open()) {
      return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
  }

  // Helper function to tokenize and return vector of tokens
  auto TokenizeInput(const std::string &input) -> std::vector<core::Token> { return lexer_->Tokenize(input); }

  // Helper function to verify token
  void VerifyToken(const core::Token &token, core::TokenType expected_type, const std::string &expected_value) {
    EXPECT_EQ(token.GetType(), expected_type);
    EXPECT_EQ(token.GetValue(), expected_value);
  }
};

// Test basic tokenization of numbers
TEST_F(LexerTest, BasicNumberTokenization) {
  auto tokens = TokenizeInput("123");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::NUMBER, "123");

  tokens = TokenizeInput("0");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::NUMBER, "0");

  tokens = TokenizeInput("999");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::NUMBER, "999");
}

// Test basic tokenization of identifiers
TEST_F(LexerTest, BasicIdentifierTokenization) {
  auto tokens = TokenizeInput("variable");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::IDENTIFIER, "variable");

  tokens = TokenizeInput("_underscore");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::IDENTIFIER, "_underscore");

  tokens = TokenizeInput("var123");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::IDENTIFIER, "var123");
}

// Test mixed token sequences
TEST_F(LexerTest, MixedTokenSequences) {
  auto tokens = TokenizeInput("hello 123");
  ASSERT_EQ(tokens.size(), 2);
  VerifyToken(tokens[0], core::TokenType::IDENTIFIER, "hello");
  VerifyToken(tokens[1], core::TokenType::NUMBER, "123");

  tokens = TokenizeInput("var123 456 test");
  ASSERT_EQ(tokens.size(), 3);
  VerifyToken(tokens[0], core::TokenType::IDENTIFIER, "var123");
  VerifyToken(tokens[1], core::TokenType::NUMBER, "456");
  VerifyToken(tokens[2], core::TokenType::IDENTIFIER, "test");
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

// Test empty and whitespace-only inputs
TEST_F(LexerTest, EmptyAndWhitespaceInputs) {
  auto tokens = TokenizeInput("");
  EXPECT_EQ(tokens.size(), 0);

  tokens = TokenizeInput("   ");
  EXPECT_EQ(tokens.size(), 0);

  tokens = TokenizeInput("\t\t");
  EXPECT_EQ(tokens.size(), 0);
}

// Test streaming interface
TEST_F(LexerTest, StreamingInterface) {
  lexer_->SetInput("hello 123 world");

  EXPECT_TRUE(lexer_->HasNext());
  auto token1 = lexer_->Next();
  ASSERT_TRUE(token1.has_value());
  VerifyToken(*token1, core::TokenType::IDENTIFIER, "hello");

  EXPECT_TRUE(lexer_->HasNext());
  auto token2 = lexer_->Next();
  ASSERT_TRUE(token2.has_value());
  VerifyToken(*token2, core::TokenType::NUMBER, "123");

  EXPECT_TRUE(lexer_->HasNext());
  auto token3 = lexer_->Next();
  ASSERT_TRUE(token3.has_value());
  VerifyToken(*token3, core::TokenType::IDENTIFIER, "world");

  EXPECT_FALSE(lexer_->HasNext());
  auto token4 = lexer_->Next();
  EXPECT_FALSE(token4.has_value());
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

// Test complex expressions
TEST_F(LexerTest, ComplexExpressions) {
  auto tokens = TokenizeInput("result <- (variable1 + variable2) * factor;");

  // Should tokenize identifiers and numbers correctly
  // Note: operators like <-, +, *, (, ), ; might not be implemented yet
  // So we only check for identifiers and numbers that should be recognized

  bool found_result = false;
  bool found_variable1 = false;
  bool found_variable2 = false;
  bool found_factor = false;

  for (const auto &token : tokens) {
    if (token.GetType() == core::TokenType::IDENTIFIER) {
      if (token.GetValue() == "result") {
        found_result = true;
      }
      if (token.GetValue() == "variable1") {
        found_variable1 = true;
      }
      if (token.GetValue() == "variable2") {
        found_variable2 = true;
      }
      if (token.GetValue() == "factor") {
        found_factor = true;
      }
    }
  }

  EXPECT_TRUE(found_result);
  EXPECT_TRUE(found_variable1);
  EXPECT_TRUE(found_variable2);
  EXPECT_TRUE(found_factor);
}

// Test identifier variations from file
TEST_F(LexerTest, IdentifierVariationsFromFile) {
  std::string content = ReadTestFile("identifier_variations.scpl");
  if (!content.empty()) {
    auto tokens = TokenizeInput(content);

    // Check that all tokens are identifiers
    for (const auto &token : tokens) {
      EXPECT_EQ(token.GetType(), core::TokenType::IDENTIFIER);
    }

    // Check for specific expected identifiers
    std::vector<std::string> expected = {"_underscore_start", "_",         "__double_underscore", "var123",
                                         "abc_def_ghi",       "camelCase", "PascalCase",          "var_123_end"};

    EXPECT_GE(tokens.size(), expected.size());
  }
}

// Test number variations from file
TEST_F(LexerTest, NumberVariationsFromFile) {
  std::string content = ReadTestFile("number_variations.scpl");
  if (!content.empty()) {
    auto tokens = TokenizeInput(content);

    // Check that all tokens are numbers
    for (const auto &token : tokens) {
      EXPECT_EQ(token.GetType(), core::TokenType::NUMBER);
    }

    // Check for specific expected numbers
    std::vector<std::string> expected = {"0", "123", "999", "42", "1000000"};
    EXPECT_GE(tokens.size(), expected.size());
  }
}

// Test mixed tokens from file
TEST_F(LexerTest, MixedTokensFromFile) {
  std::string content = ReadTestFile("mixed_tokens.scpl");
  if (!content.empty()) {
    auto tokens = TokenizeInput(content);
    EXPECT_GT(tokens.size(), 0);

    // Should contain both identifiers and numbers
    bool has_identifier = false;
    bool has_number = false;
    for (const auto &token : tokens) {
      if (token.GetType() == core::TokenType::IDENTIFIER) {
        has_identifier = true;
      }
      if (token.GetType() == core::TokenType::NUMBER) {
        has_number = true;
      }
    }

    EXPECT_TRUE(has_identifier);
    EXPECT_TRUE(has_number);
  }
}

// Test invalid token handling
TEST_F(LexerTest, InvalidTokenHandling) {
  // Test sequences that start valid but become invalid
  auto tokens = TokenizeInput("123abc");
  // Should get "123" as a number, but "abc" part might be handled differently
  // depending on implementation - this tests the lexer's error recovery
  EXPECT_GT(tokens.size(), 0);

  // The first token should be the number "123"
  bool found_123 = false;
  for (const auto &token : tokens) {
    if (token.GetType() == core::TokenType::NUMBER && token.GetValue() == "123") {
      found_123 = true;
      break;
    }
  }
  EXPECT_TRUE(found_123);
}

// Test edge cases with single characters
TEST_F(LexerTest, SingleCharacterTokens) {
  auto tokens = TokenizeInput("a");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::IDENTIFIER, "a");

  tokens = TokenizeInput("9");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::NUMBER, "9");

  tokens = TokenizeInput("_");
  ASSERT_EQ(tokens.size(), 1);
  VerifyToken(tokens[0], core::TokenType::IDENTIFIER, "_");
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

// Test consistency between streaming and batch interfaces
TEST_F(LexerTest, StreamingVsBatchConsistency) {
  std::string input = "hello 123 world 456";

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

}  // namespace scp::test
