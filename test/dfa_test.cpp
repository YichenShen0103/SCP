#include <iostream>
#include <memory>

#include "../include/dfa.h"

int main()
{
  // Example usage of DeterministicFiniteAutomata
  std::string id_alphabet = "0123456789abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ_";
  std::string number_alphabet = "0123456789";
  std::unique_ptr<DeterministicFiniteAutomata> number =
      std::make_unique<DeterministicFiniteAutomata>(2, number_alphabet,
                                                    "NUMBER"); // ^[0-9]+$
  std::unique_ptr<DeterministicFiniteAutomata> identifier =
      std::make_unique<DeterministicFiniteAutomata>(
          2, id_alphabet, "IDENTIFIER"); // ^[A-Za-z_][A-Za-z0-9_]*$

  // Set up NUMBER DFA transitions: ^[0-9]+$
  number->AddTransition(0, '0', 1); // 0 -> 1 on '0'
  number->AddTransition(1, '0', 1); // 1 -> 1 on '0' (stay in accepting state)
  for (int i = 0; i < 9; ++i)
  {
    char c = '1' + i;
    number->AddTransition(0, c, 1); // 0 -> 1 on '1'-'9'
    number->AddTransition(1, c, 1); // 1 -> 1 on '1'-'9'
  }
  number->SetFinalState(1); // State 1 is accepting state for numbers

  // Set up IDENTIFIER DFA transitions: ^[A-Za-z_][A-Za-z0-9_]*$
  // First, handle initial characters (letters and underscore)
  for (char c = 'a'; c <= 'z'; ++c)
  {
    identifier->AddTransition(0, c, 1); // 0 -> 1 on letters
  }
  for (char c = 'A'; c <= 'Z'; ++c)
  {
    identifier->AddTransition(0, c, 1); // 0 -> 1 on letters
  }
  identifier->AddTransition(0, '_', 1); // 0 -> 1 on underscore

  // Then, handle continuation characters (letters, digits, underscore)
  for (char c : id_alphabet)
  {
    identifier->AddTransition(1, c, 1); // 1 -> 1 on any valid identifier char
  }
  identifier->SetFinalState(1); // State 1 is accepting state for identifiers

  number->Release();
  identifier->Release();

  std::vector<std::unique_ptr<DeterministicFiniteAutomata>> dfa_list;
  std::vector<bool> survival_list;

  // Add the DFAs to the list
  dfa_list.push_back(std::move(number));
  dfa_list.push_back(std::move(identifier));

  // Initialize survival list
  survival_list.resize(dfa_list.size());

  std::vector<std::string> inputs;
  inputs.push_back("aa 123");
  inputs.push_back("ababc 4a56b7");
  inputs.push_back("xyz_789");

  bool have_survival = false;
  for (size_t input_idx = 0; input_idx < inputs.size(); ++input_idx)
  {
    const std::string &input = inputs[input_idx];
    std::cout << "Evaluating input: " << input << std::endl;
    for (size_t i = 0; i < dfa_list.size(); ++i)
    {
      dfa_list[i]->Init();
      survival_list[i] = true;
    }

    size_t prev = 0;
    size_t j = 0;

    while (j < input.size())
    {
      // Skip whitespace
      while (j < input.size() && input[j] == ' ')
      {
        ++j;
      }
      if (j >= input.size())
        break;

      prev = j;
      // Reset DFAs for next token
      for (size_t i = 0; i < dfa_list.size(); ++i)
      {
        dfa_list[i]->Init();
        survival_list[i] = true;
      }

      size_t last_accepted_pos = prev;
      int last_accepted_dfa = -1;

      // Try to consume characters as long as possible
      while (j < input.size() && input[j] != ' ')
      {
        have_survival = false;

        for (size_t i = 0; i < dfa_list.size(); ++i)
        {
          if (survival_list[i])
          {
            if (!dfa_list[i]->Evaluate(input[j]))
            {
              survival_list[i] = false;
            }
            else
            {
              have_survival = true;
              // Check if this DFA is in accepting state
              if (dfa_list[i]->isAccepted())
              {
                last_accepted_pos = j + 1;
                last_accepted_dfa = i;
              }
            }
          }
        }

        // If no DFA can continue, break
        if (!have_survival)
        {
          break;
        }

        ++j;
      }

      // If we found an accepted token, output it
      if (last_accepted_dfa != -1)
      {
        std::cout << dfa_list[last_accepted_dfa]->getTokenClass()
                  << " accepted input: "
                  << input.substr(prev, last_accepted_pos - prev) << std::endl;
        j = last_accepted_pos;
      }
      else
      {
        // Skip the problematic character
        std::cout << "No valid token found at position " << prev
                  << " for character '" << input[prev] << "'" << std::endl;
        ++j;
      }
    }
  }

  return 0;
}
