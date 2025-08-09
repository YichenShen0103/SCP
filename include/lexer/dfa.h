#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/token.h"

namespace scp::lexer {

class DeterministicFiniteAutomata {
 public:
  DeterministicFiniteAutomata(int num_states, std::string alphabet, core::TokenType token_class);
  ~DeterministicFiniteAutomata() = default;

  auto AddTransition(int from_state, char symbol, int to_state) -> bool;
  auto SetFinalState(int state) -> bool;
  void Release();
  auto Evaluate(char byte) -> bool;
  void Init() { current_state_ = initial_state_; }
  auto IsAccepted() const -> bool { return final_states_.find(current_state_) != final_states_.end(); }
  auto GetTokenClass() const -> std::string { return core::ToString(token_class_); }
  auto GetTokenClassRaw() const -> core::TokenType { return token_class_; }

 private:
  core::TokenType token_class_;
  std::unordered_set<int> final_states_;
  const int initial_state_ = 0;
  int current_state_{0};

  // Table and parameters
  int num_states_;
  size_t alphabet_size_;
  std::vector<std::shared_ptr<std::vector<int>>> states_transition_released_;
  std::vector<std::vector<int>> states_transition_;
  std::unordered_map<char, int> alphabet_;

  // Released or Building
  bool released_{false};
};

}  // namespace scp::lexer
