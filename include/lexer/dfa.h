#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/token.h"

namespace scp::lexer {

/**
 * Class representing a deterministic finite automaton (DFA).
 */
class DeterministicFiniteAutomata {
 public:
  /**
   * Constructor for a DFA.
   * @param num_states The number of states in the DFA.
   * @param alphabet The alphabet of the DFA.
   * @param token_class The token class associated with the DFA.
   */
  DeterministicFiniteAutomata(int num_states, std::string alphabet, core::TokenType token_class);

  /**
   * Destructor for a DFA.
   */
  ~DeterministicFiniteAutomata() = default;

  /**
   * Add a transition to the DFA.
   * @param from_state The state to transition from.
   * @param symbol The input symbol for the transition.
   * @param to_state The state to transition to.
   * @return True if the transition was added successfully, false otherwise.
   */
  auto AddTransition(int from_state, char symbol, int to_state) -> bool;

  /**
   * Set a state as a final state.
   * @param state The state to set as final.
   * @return True if the state was set as final successfully, false if the DFA is released.
   */
  auto SetFinalState(int state) -> bool;

  /**
   * Release the DFA.
   */
  void Release();

  /**
   * Evaluate the DFA on a single input byte.
   * @param byte The input byte to evaluate.
   * @return True if the DFA accepted the input, false otherwise.
   */
  auto Evaluate(char byte) -> bool;

  /**
   * Initialize the DFA to the initial state.
   */
  void Init() { current_state_ = initial_state_; }

  /**
   * Check if the DFA is in an accepted state.
   * @return True if the current state is a final state, false otherwise.
   */
  auto IsAccepted() const -> bool { return final_states_.find(current_state_) != final_states_.end(); }

  /**
   * Get the token class associated with the DFA.
   * @return The token class associated with the DFA.
   */
  auto GetTokenClass() const -> std::string { return core::ToString(token_class_); }

  /**
   * Get the token class associated with the DFA.
   * @return The token class associated with the DFA.
   */
  auto GetTokenClassRaw() const -> core::TokenType { return token_class_; }

 private:
  /* The token class associated with the DFA */
  core::TokenType token_class_;
  /* The set of final states for the DFA */
  std::unordered_set<int> final_states_;
  /* The initial state of the DFA */
  const int initial_state_ = 0;
  /* The current state of the DFA */
  int current_state_{0};

  // Table and parameters
  /* The number of states in the DFA */
  int num_states_;
  /* The size of the alphabet for the DFA */
  size_t alphabet_size_;
  /* The transition table for the DFA */
  std::vector<std::shared_ptr<std::vector<int>>> states_transition_released_;
  /* The transition table for the DFA */
  std::vector<std::vector<int>> states_transition_;
  /* The alphabet of the DFA */
  std::unordered_map<char, int> alphabet_;

  // Released or Building
  /* Whether the DFA is released or still being built */
  bool released_{false};
};

}  // namespace scp::lexer
