#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class DeterministicFiniteAutomata {
public:
  DeterministicFiniteAutomata(int num_states, std::string alphabet,
                              std::string token_name);
  ~DeterministicFiniteAutomata() = default;

  bool AddTransition(int from_state, char symbol, int to_state);
  bool SetFinalState(int state);
  void Release();
  bool Evaluate(char byte);
  void Init() { current_state_ = initial_state_; }
  bool isAccepted() const {
    return final_states_.find(current_state_) != final_states_.end();
  }
  std::string getTokenClass() const { return token_class_; }

private:
  std::string token_class_;
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
