#include <gtest/gtest.h>
#include <fstream>
#include <memory>
#include <string>

#include "core/token.h"
#include "lexer/dfa.h"

namespace scp::test {

class DFATest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create NUMBER DFA: ^[0-9]+$
    std::string number_alphabet = "0123456789";
    number_dfa_ = std::make_unique<lexer::DeterministicFiniteAutomata>(2, number_alphabet, core::TokenType::NUMBER);

    // Add transitions for digits
    for (char c = '0'; c <= '9'; ++c) {
      number_dfa_->AddTransition(0, c, 1);  // 0 -> 1 on any digit
      number_dfa_->AddTransition(1, c, 1);  // 1 -> 1 on any digit (self-loop)
    }
    number_dfa_->SetFinalState(1);
    number_dfa_->Release();

    // Create IDENTIFIER DFA: ^[A-Za-z_][A-Za-z0-9_]*$
    std::string id_alphabet =
        "0123456789abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ_";
    identifier_dfa_ = std::make_unique<lexer::DeterministicFiniteAutomata>(2, id_alphabet, core::TokenType::IDENTIFIER);

    // Add transitions for initial characters (letters and underscore)
    for (char c = 'a'; c <= 'z'; ++c) {
      identifier_dfa_->AddTransition(0, c, 1);
    }
    for (char c = 'A'; c <= 'Z'; ++c) {
      identifier_dfa_->AddTransition(0, c, 1);
    }
    identifier_dfa_->AddTransition(0, '_', 1);

    // Add transitions for continuation characters (letters, digits, underscore)
    for (char c : id_alphabet) {
      identifier_dfa_->AddTransition(1, c, 1);
    }
    identifier_dfa_->SetFinalState(1);
    identifier_dfa_->Release();
  }

  std::unique_ptr<lexer::DeterministicFiniteAutomata> number_dfa_;
  std::unique_ptr<lexer::DeterministicFiniteAutomata> identifier_dfa_;

  // Helper function to get test data path
  static auto GetTestDataPath() -> std::string {
#ifdef TEST_DATA_DIR
    return TEST_DATA_DIR;
#else
    return "test/data";
#endif
  }

  // Helper function to read file content
  auto ReadTestFile(const std::string &filename) -> std::string {
    std::ifstream file(GetTestDataPath() + "/code/" + filename);
    if (!file.is_open()) {
      return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
  }

  // Helper function to test if DFA accepts a string
  auto TestDFAAccepts(lexer::DeterministicFiniteAutomata *dfa, const std::string &input) -> bool {
    dfa->Init();
    for (char c : input) {
      if (!dfa->Evaluate(c)) {
        return false;
      }
    }
    return dfa->IsAccepted();
  }
};

// Test NUMBER DFA with valid numbers
TEST_F(DFATest, NumberDFAValidInputs) {
  EXPECT_TRUE(TestDFAAccepts(number_dfa_.get(), "0"));
  EXPECT_TRUE(TestDFAAccepts(number_dfa_.get(), "1"));
  EXPECT_TRUE(TestDFAAccepts(number_dfa_.get(), "123"));
  EXPECT_TRUE(TestDFAAccepts(number_dfa_.get(), "999"));
  EXPECT_TRUE(TestDFAAccepts(number_dfa_.get(), "42"));
  EXPECT_TRUE(TestDFAAccepts(number_dfa_.get(), "1000000"));
}

// Test NUMBER DFA with invalid inputs
TEST_F(DFATest, NumberDFAInvalidInputs) {
  EXPECT_FALSE(TestDFAAccepts(number_dfa_.get(), ""));
  EXPECT_FALSE(TestDFAAccepts(number_dfa_.get(), "a"));
  EXPECT_FALSE(TestDFAAccepts(number_dfa_.get(), "123a"));
  EXPECT_FALSE(TestDFAAccepts(number_dfa_.get(), "a123"));
  EXPECT_FALSE(TestDFAAccepts(number_dfa_.get(), "_123"));
  EXPECT_FALSE(TestDFAAccepts(number_dfa_.get(), "12.3"));
  EXPECT_FALSE(TestDFAAccepts(number_dfa_.get(), " 123"));
  EXPECT_FALSE(TestDFAAccepts(number_dfa_.get(), "123 "));
}

// Test IDENTIFIER DFA with valid identifiers
TEST_F(DFATest, IdentifierDFAValidInputs) {
  EXPECT_TRUE(TestDFAAccepts(identifier_dfa_.get(), "a"));
  EXPECT_TRUE(TestDFAAccepts(identifier_dfa_.get(), "_"));
  EXPECT_TRUE(TestDFAAccepts(identifier_dfa_.get(), "variable"));
  EXPECT_TRUE(TestDFAAccepts(identifier_dfa_.get(), "variable_name"));
  EXPECT_TRUE(TestDFAAccepts(identifier_dfa_.get(), "_underscore_start"));
  EXPECT_TRUE(TestDFAAccepts(identifier_dfa_.get(), "var123"));
  EXPECT_TRUE(TestDFAAccepts(identifier_dfa_.get(), "_123"));
  EXPECT_TRUE(TestDFAAccepts(identifier_dfa_.get(), "camelCase"));
  EXPECT_TRUE(TestDFAAccepts(identifier_dfa_.get(), "PascalCase"));
  EXPECT_TRUE(TestDFAAccepts(identifier_dfa_.get(), "__double_underscore"));
}

// Test IDENTIFIER DFA with invalid identifiers
TEST_F(DFATest, IdentifierDFAInvalidInputs) {
  EXPECT_FALSE(TestDFAAccepts(identifier_dfa_.get(), ""));
  EXPECT_FALSE(TestDFAAccepts(identifier_dfa_.get(), "123"));
  EXPECT_FALSE(TestDFAAccepts(identifier_dfa_.get(), "123abc"));
  EXPECT_FALSE(TestDFAAccepts(identifier_dfa_.get(), " variable"));
  EXPECT_FALSE(TestDFAAccepts(identifier_dfa_.get(), "variable "));
  EXPECT_FALSE(TestDFAAccepts(identifier_dfa_.get(), "var@invalid"));
  EXPECT_FALSE(TestDFAAccepts(identifier_dfa_.get(), "var$bad"));
  EXPECT_FALSE(TestDFAAccepts(identifier_dfa_.get(), "var.name"));
}

// Test DFA initialization and reset
TEST_F(DFATest, DFAInitialization) {
  // Test that DFA starts in initial state
  number_dfa_->Init();
  EXPECT_FALSE(number_dfa_->IsAccepted());  // Should not be in accepting state initially

  // Process some input
  EXPECT_TRUE(number_dfa_->Evaluate('1'));
  EXPECT_TRUE(number_dfa_->IsAccepted());  // Should be in accepting state after '1'

  // Reset and test again
  number_dfa_->Init();
  EXPECT_FALSE(number_dfa_->IsAccepted());  // Should be back to initial state

  // Test identifier DFA
  identifier_dfa_->Init();
  EXPECT_FALSE(identifier_dfa_->IsAccepted());  // Should not be in accepting state initially

  EXPECT_TRUE(identifier_dfa_->Evaluate('a'));
  EXPECT_TRUE(identifier_dfa_->IsAccepted());  // Should be in accepting state after 'a'

  identifier_dfa_->Init();
  EXPECT_FALSE(identifier_dfa_->IsAccepted());  // Should be back to initial state
}

// Test DFA token type retrieval
TEST_F(DFATest, TokenTypeRetrieval) {
  EXPECT_EQ(number_dfa_->GetTokenClassRaw(), core::TokenType::NUMBER);
  EXPECT_EQ(identifier_dfa_->GetTokenClassRaw(), core::TokenType::IDENTIFIER);

  EXPECT_STREQ(number_dfa_->GetTokenClass().c_str(), "NUMBER");
  EXPECT_STREQ(identifier_dfa_->GetTokenClass().c_str(), "IDENTIFIER");
}

// Test DFA with file inputs
TEST_F(DFATest, FileInputTests) {
  // Test simple number file
  std::string number_content = ReadTestFile("simple_number.scpl");
  if (!number_content.empty()) {
    // Remove potential newline
    if (!number_content.empty() && number_content.back() == '\n') {
      number_content.pop_back();
    }
    EXPECT_TRUE(TestDFAAccepts(number_dfa_.get(), number_content));
  }

  // Test simple identifier file
  std::string identifier_content = ReadTestFile("simple_identifier.scpl");
  if (!identifier_content.empty()) {
    // Remove potential newline
    if (!identifier_content.empty() && identifier_content.back() == '\n') {
      identifier_content.pop_back();
    }
    EXPECT_TRUE(TestDFAAccepts(identifier_dfa_.get(), identifier_content));
  }
}

// Test DFA partial matching behavior
TEST_F(DFATest, PartialMatching) {
  // Test that DFA can accept intermediate states during processing
  number_dfa_->Init();
  EXPECT_TRUE(number_dfa_->Evaluate('1'));
  EXPECT_TRUE(number_dfa_->IsAccepted());  // Should accept "1"

  EXPECT_TRUE(number_dfa_->Evaluate('2'));
  EXPECT_TRUE(number_dfa_->IsAccepted());  // Should still accept "12"

  EXPECT_TRUE(number_dfa_->Evaluate('3'));
  EXPECT_TRUE(number_dfa_->IsAccepted());  // Should still accept "123"

  // Test failure case
  EXPECT_FALSE(number_dfa_->Evaluate('a'));  // Should fail on non-digit
  EXPECT_FALSE(number_dfa_->IsAccepted());   // Should no longer be accepted
}

// Test edge cases
TEST_F(DFATest, EdgeCases) {
  // Test single character inputs
  EXPECT_TRUE(TestDFAAccepts(number_dfa_.get(), "0"));
  EXPECT_TRUE(TestDFAAccepts(number_dfa_.get(), "9"));
  EXPECT_TRUE(TestDFAAccepts(identifier_dfa_.get(), "a"));
  EXPECT_TRUE(TestDFAAccepts(identifier_dfa_.get(), "Z"));
  EXPECT_TRUE(TestDFAAccepts(identifier_dfa_.get(), "_"));

  // Test that numbers can't start with letters
  identifier_dfa_->Init();
  EXPECT_TRUE(identifier_dfa_->Evaluate('a'));
  EXPECT_TRUE(identifier_dfa_->IsAccepted());
  EXPECT_TRUE(identifier_dfa_->Evaluate('1'));  // Should accept digit after letter
  EXPECT_TRUE(identifier_dfa_->IsAccepted());

  // But numbers can't start with letters
  number_dfa_->Init();
  EXPECT_FALSE(number_dfa_->Evaluate('a'));  // Should fail immediately
}

}  // namespace scp::test
