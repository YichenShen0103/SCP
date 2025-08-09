#include "lexer/dfa.h"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "constant/error_messages.h"

namespace scp::lexer {

struct VectorHash {
  auto operator()(const std::vector<int> &v) const noexcept -> std::size_t {
    std::size_t hash = 0;
    for (int num : v) {
      hash ^= std::hash<int>{}(num) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    return hash;
  }
};

struct VectorEqual {
  auto operator()(const std::vector<int> &a, const std::vector<int> &b) const noexcept -> bool { return a == b; }
};

auto Compress(std::vector<std::vector<int>> input) -> std::vector<std::shared_ptr<std::vector<int>>> {
  using RowPtr = std::shared_ptr<std::vector<int>>;
  std::unordered_map<std::vector<int>, RowPtr, VectorHash, VectorEqual> row_pool;
  std::vector<RowPtr> result;
  result.reserve(input.size());

  for (auto &row : input) {
    auto it = row_pool.find(row);
    if (it != row_pool.end()) {
      result.push_back(it->second);
    } else {
      auto ptr = std::make_shared<std::vector<int>>(std::move(row));
      row_pool[*ptr] = ptr;
      result.push_back(ptr);
    }
  }
  return result;
}

DeterministicFiniteAutomata::DeterministicFiniteAutomata(int num_states, std::string alphabet,
                                                         core::TokenType token_class)
    : token_class_(token_class), num_states_(num_states) {
  states_transition_.resize(num_states);
  alphabet_size_ = alphabet.size();

  for (int i = 0; i < num_states; ++i) {
    states_transition_[i] = std::vector<int>(alphabet_size_, -1);
  }

  // Build alphabet
  for (size_t i = 0; i < alphabet.size(); ++i) {
    alphabet_[alphabet[i]] = i;
  }
}

auto DeterministicFiniteAutomata::Release() -> void {
  released_ = true;
  states_transition_released_ = Compress(states_transition_);
}

auto DeterministicFiniteAutomata::AddTransition(int from_state, char symbol, int to_state) -> bool {
  if (released_) {
    std::cerr << constant::ErrorMessages::DFA_RELEASED_CANNOT_ADD_TRANSITION << std::endl;
    return false;
  }

  if (from_state < 0 || from_state >= num_states_ || to_state < 0 || to_state >= num_states_) {
    std::cerr << constant::ErrorMessages::INVALID_STATE << std::endl;
    return false;
  }

  auto symbol_it = alphabet_.find(symbol);
  if (symbol_it == alphabet_.end()) {
    std::cerr << constant::ErrorMessages::SymbolNotInAlphabetWithDetails(symbol) << std::endl;
    return false;
  }

  states_transition_[from_state][symbol_it->second] = to_state;
  return true;
}

auto DeterministicFiniteAutomata::SetFinalState(int state) -> bool {
  if (released_) {
    std::cerr << constant::ErrorMessages::DFA_RELEASED_CANNOT_SET_FINAL << std::endl;
    return false;
  }

  if (state < 0 || state >= num_states_) {
    std::cerr << constant::ErrorMessages::INVALID_STATE << std::endl;
    return false;
  }

  final_states_.insert(state);
  return true;
}

auto DeterministicFiniteAutomata::Evaluate(char byte) -> bool {
  if (!released_) {
    std::cerr << constant::ErrorMessages::DFA_NOT_RELEASED_CANNOT_EVALUATE << std::endl;
    return false;
  }

  if (alphabet_.find(byte) == alphabet_.end()) {
    current_state_ = -1;
    return false;
  }

  int symbol_index = alphabet_[byte];
  current_state_ = (*states_transition_released_[current_state_])[symbol_index];

  return current_state_ != -1;
}

}  // namespace scp::lexer
