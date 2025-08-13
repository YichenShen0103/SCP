#include "lexer/lexer.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/**
 * Print the usage information for the lexer program.
 * @param programName The name of the program (usually argv[0]).
 */
void PrintUsage(const std::string &programName) {
  std::cout << "Usage: " << programName << " <input_file>" << std::endl;
  std::cout << "  input_file: Path to the source file to tokenize" << std::endl;
}

/**
 * Read the contents of a file into a string.
 * @param filename The name of the file to read.
 * @return The contents of the file as a string.
 */
auto ReadFile(const std::string &filename) -> std::string {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file: " + filename);
  }

  std::string content;
  std::string line;
  while (std::getline(file, line)) {
    content += line + "\n";
  }

  file.close();
  return content;
}

/**
 * Print the tokens found in the input file.
 * @param tokens The vector of tokens to print.
 */
void PrintTokens(const std::vector<scp::core::Token> &tokens) {
  if (tokens.empty()) {
    std::cout << "No tokens found in the input file." << std::endl;
    return;
  }

  std::cout << "Tokens found:" << std::endl;
  std::cout << std::string(50, '-') << std::endl;

  for (size_t i = 0; i < tokens.size(); ++i) {
    const scp::core::Token &token = tokens[i];
    std::cout << "Token " << (i + 1) << ": " << scp::core::ToString(token.GetType()) << " = \"" << token.GetValue()
              << "\" at (" << token.GetLine() << ", " << token.GetColumn() << ")" << std::endl;
  }

  std::cout << std::string(50, '-') << std::endl;
  std::cout << "Total tokens: " << tokens.size() << std::endl;
}

/**
 * Main entry point for the lexer program.
 * @param argc The number of command line arguments.
 * @param argv The command line arguments.
 * @return Exit status code.
 */
auto main(int argc, char *argv[]) -> int {
  // Check command line arguments
  if (argc != 2) {
    std::cerr << "Error: Invalid number of arguments." << std::endl;
    PrintUsage(argv[0]);
    return 1;
  }

  std::string filename = argv[1];

  try {
    // Read the input file
    std::string file_content = ReadFile(filename);

    if (file_content.empty()) {
      std::cout << "Warning: The input file is empty." << std::endl;
      return 0;
    }

    // Create lexer and tokenize
    scp::lexer::Lexer lexer;
    std::vector<scp::core::Token> tokens = lexer.Tokenize(file_content);

    // Print results
    std::cout << "File: " << filename << std::endl;
    std::cout << "Content length: " << file_content.length() << " characters" << std::endl;
    std::cout << std::endl;

    PrintTokens(tokens);

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
