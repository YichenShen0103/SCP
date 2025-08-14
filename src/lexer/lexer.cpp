#include "lexer/lexer.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "constant/alphabet.h"
#include "core/token.h"

namespace scp::lexer {

Lexer::Lexer() { InitializeDFAs(); }

void Lexer::InitializeDFAs() {
  // Create DFA instances using configuration constants
  number_dfa_ =
      std::make_unique<DeterministicFiniteAutomata>(2, constant::Alphabet::DIGIT_ALPHABET, core::TokenType::NUMBER);
  identifier_dfa_ = std::make_unique<DeterministicFiniteAutomata>(2, constant::Alphabet::IDENTIFIER_ALPHABET,
                                                                  core::TokenType::IDENTIFIER);
  plus_dfa_ =
      std::make_unique<DeterministicFiniteAutomata>(2, constant::Alphabet::PLUS_ALPHABET, core::TokenType::PLUS);
  left_paren_dfa_ = std::make_unique<DeterministicFiniteAutomata>(2, constant::Alphabet::LEFT_PAREN_ALPHABET,
                                                                  core::TokenType::LEFT_PAREN);
  right_paren_dfa_ = std::make_unique<DeterministicFiniteAutomata>(2, constant::Alphabet::RIGHT_PAREN_ALPHABET,
                                                                   core::TokenType::RIGHT_PAREN);
  times_dfa_ =
      std::make_unique<DeterministicFiniteAutomata>(2, constant::Alphabet::TIMES_ALPHABET, core::TokenType::TIMES);
  assign_dfa_ =
      std::make_unique<DeterministicFiniteAutomata>(3, constant::Alphabet::ASSIGN_ALPHABET, core::TokenType::ASSIGN);
  semicolon_dfa_ = std::make_unique<DeterministicFiniteAutomata>(2, constant::Alphabet::SEMICOLON_ALPHABET,
                                                                 core::TokenType::SEMICOLON);
  string_dfa_ =
      std::make_unique<DeterministicFiniteAutomata>(3, constant::Alphabet::STRING_ALPHABET, core::TokenType::STRING);

  // Setup transitions for each DFA
  SetupNumberDFA();
  SetupIdentifierDFA();
  SetupPlusDFA();
  SetupLeftParenDFA();
  SetupRightParenDFA();
  SetupTimesDFA();
  SetupAssignDFA();
  SetupSemicolonDFA();
  SetupStringDFA();

  // Release the DFAs (make them ready for evaluation)
  number_dfa_->Release();
  identifier_dfa_->Release();
  times_dfa_->Release();
  plus_dfa_->Release();
  left_paren_dfa_->Release();
  right_paren_dfa_->Release();
  assign_dfa_->Release();
  semicolon_dfa_->Release();
  string_dfa_->Release();

  // Set up the DFA list for iteration
  dfa_list_.push_back(number_dfa_.get());
  dfa_list_.push_back(identifier_dfa_.get());
  dfa_list_.push_back(times_dfa_.get());
  dfa_list_.push_back(plus_dfa_.get());
  dfa_list_.push_back(left_paren_dfa_.get());
  dfa_list_.push_back(right_paren_dfa_.get());
  dfa_list_.push_back(assign_dfa_.get());
  dfa_list_.push_back(semicolon_dfa_.get());
  dfa_list_.push_back(string_dfa_.get());

  // Initialize survival list
  survival_list_.resize(dfa_list_.size());
}

void Lexer::SetupNumberDFA() {
  // Set up NUMBER DFA transitions: ^[0-9]+$
  // Transition from state 0 to state 1 on any digit
  for (char c : std::string(constant::Alphabet::DIGIT_ALPHABET)) {
    number_dfa_->AddTransition(0, c, 1);
    number_dfa_->AddTransition(1, c, 1);  // Stay in accepting state
  }
  number_dfa_->SetFinalState(1);  // State 1 is accepting
}

void Lexer::SetupIdentifierDFA() {
  // Set up IDENTIFIER DFA transitions: ^[A-Za-z_][A-Za-z0-9_]*$
  // First character must be letter or underscore
  for (char c = 'a'; c <= 'z'; ++c) {
    identifier_dfa_->AddTransition(0, c, 1);
  }
  for (char c = 'A'; c <= 'Z'; ++c) {
    identifier_dfa_->AddTransition(0, c, 1);
  }
  identifier_dfa_->AddTransition(0, '_', 1);

  // Subsequent characters can be letters, digits, or underscore
  for (char c : std::string(constant::Alphabet::IDENTIFIER_ALPHABET)) {
    identifier_dfa_->AddTransition(1, c, 1);
  }
  identifier_dfa_->SetFinalState(1);  // State 1 is accepting
}

void Lexer::SetupPlusDFA() {
  plus_dfa_->AddTransition(0, '+', 1);
  plus_dfa_->SetFinalState(1);
}

void Lexer::SetupLeftParenDFA() {
  left_paren_dfa_->AddTransition(0, '(', 1);
  left_paren_dfa_->SetFinalState(1);
}

void Lexer::SetupRightParenDFA() {
  right_paren_dfa_->AddTransition(0, ')', 1);
  right_paren_dfa_->SetFinalState(1);
}

void Lexer::SetupTimesDFA() {
  times_dfa_->AddTransition(0, '*', 1);
  times_dfa_->SetFinalState(1);
}

void Lexer::SetupAssignDFA() {
  assign_dfa_->AddTransition(0, '<', 1);
  assign_dfa_->AddTransition(1, '-', 2);
  assign_dfa_->SetFinalState(2);
}

void Lexer::SetupSemicolonDFA() {
  semicolon_dfa_->AddTransition(0, ';', 1);
  semicolon_dfa_->SetFinalState(1);
}

void Lexer::SetupStringDFA() {
  string_dfa_->AddTransition(0, '\"', 1);
  string_dfa_->AddTransition(1, '\"', 2);
  for (char c : std::string(constant::Alphabet::STRING_ALPHABET)) {
    // Stay in state 1 for valid characters (except quotes which transition to final state)
    if (c != '\"') {
      string_dfa_->AddTransition(1, c, 1);
    }
  }
  string_dfa_->SetFinalState(2);
}

void Lexer::SetInput(const std::string &input) {
  input_ = input;
  current_pos_ = 0;
  current_line_ = 1;
  current_column_ = 1;
}

auto Lexer::Next() -> std::optional<core::Token> {
  core::Token token(core::TokenType::IDENTIFIER, "", 0, 0);  // dummy initialization
  if (GetNextToken(token)) {
    return token;
  }
  return std::nullopt;
}

auto Lexer::HasNext() const -> bool {
  // Skip whitespace to check if there's more content
  size_t pos = current_pos_;
  while (pos < input_.size() &&
         (input_[pos] == ' ' || input_[pos] == '\t' || input_[pos] == '\n' || input_[pos] == '\r')) {
    ++pos;
  }
  return pos < input_.size();
}

void Lexer::Reset() {
  current_pos_ = 0;
  current_line_ = 1;
  current_column_ = 1;
}

void Lexer::SkipWhitespace() {
  while (current_pos_ < input_.size() && (input_[current_pos_] == ' ' || input_[current_pos_] == '\t' ||
                                          input_[current_pos_] == '\n' || input_[current_pos_] == '\r')) {
    if (input_[current_pos_] == '\n') {
      current_line_++;
      current_column_ = 1;
    } else {
      current_column_++;
    }
    ++current_pos_;
  }
}

auto Lexer::GetNextToken(core::Token &token) -> bool {
  // Skip whitespace
  SkipWhitespace();

  if (current_pos_ >= input_.size()) {
    return false;
  }

  size_t token_start = current_pos_;
  int token_start_line = current_line_;
  int token_start_column = current_column_;

  // Reset DFAs for next token
  for (size_t i = 0; i < dfa_list_.size(); ++i) {
    dfa_list_[i]->Init();
    survival_list_[i] = true;
  }

  size_t last_accepted_pos = token_start;
  int last_accepted_dfa = -1;
  bool have_survival = true;

  // Try to consume characters as long as possible
  while (current_pos_ < input_.size() && have_survival) {
    // Stop at whitespace only if string DFA is not active
    if ((input_[current_pos_] == ' ' || input_[current_pos_] == '\t' || input_[current_pos_] == '\n' ||
         input_[current_pos_] == '\r') &&
        !survival_list_[8]) {  // 8 is the index of string_dfa_
      break;
    }

    have_survival = false;

    for (size_t i = 0; i < dfa_list_.size(); ++i) {
      if (survival_list_[i]) {
        if (!dfa_list_[i]->Evaluate(input_[current_pos_])) {
          survival_list_[i] = false;
        } else {
          have_survival = true;
          // Check if this DFA is in accepting state
          if (dfa_list_[i]->IsAccepted()) {
            last_accepted_pos = current_pos_ + 1;
            last_accepted_dfa = static_cast<int>(i);
          }
        }
      }
    }

    // Update position tracking
    if (input_[current_pos_] == '\n') {
      current_line_++;
      current_column_ = 1;
    } else {
      current_column_++;
    }

    ++current_pos_;
  }

  // If we found an accepted token, create and return it
  if (last_accepted_dfa != -1) {
    std::string token_value = input_.substr(token_start, last_accepted_pos - token_start);
    core::TokenType token_type = dfa_list_[last_accepted_dfa]->GetTokenClassRaw();

    token = core::Token(token_type, token_value, token_start_line, token_start_column);

    // Set current_pos_ to the position after the accepted token
    current_pos_ = last_accepted_pos;
    return true;
  }
  // Skip the problematic character
  std::cerr << "Lexer: No valid token found at line " << current_line_ << ", column " << current_column_
            << " for character '" << input_[token_start] << "'" << std::endl;
  if (input_[current_pos_] == '\n') {
    current_line_++;
    current_column_ = 1;
  } else {
    current_column_++;
  }
  ++current_pos_;
  return false;
}

auto Lexer::Tokenize(const std::string &input) -> std::vector<core::Token> {
  std::vector<core::Token> tokens;

  // Set input and reset position
  SetInput(input);

  // Use the streaming interface to get all tokens
  auto token_opt = Next();
  while (token_opt.has_value()) {
    tokens.push_back(token_opt.value());
    token_opt = Next();
  }

  return tokens;
}

}  // namespace scp::lexer
