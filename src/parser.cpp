#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include "core/ast.h"
#include "parser/ll1_parser.h"

/**
 * Print the Abstract Syntax Tree (AST).
 * @param node The root node of the AST.
 * @param depth The current depth in the tree (for indentation).
 */
void PrintAST(const std::shared_ptr<scp::core::AST::ASTNode> &node, int depth = 0) {
  if (!node) {
    for (int i = 0; i < depth; ++i) {
      std::cout << "  ";
    }
    std::cout << "NULL" << std::endl;
    return;
  }

  for (int i = 0; i < depth; ++i) {
    std::cout << "  ";
  }

  std::cout << "Type: ";
  switch (node->GetType()) {
    case scp::core::ASTNodeType::ROOT:
      std::cout << "ROOT";
      break;
    case scp::core::ASTNodeType::IDENTIFIER:
      std::cout << "IDENTIFIER";
      break;
    case scp::core::ASTNodeType::NUMBER:
      std::cout << "NUMBER";
      break;
    case scp::core::ASTNodeType::PLUS:
      std::cout << "PLUS";
      break;
    case scp::core::ASTNodeType::TIMES:
      std::cout << "TIMES";
      break;
    case scp::core::ASTNodeType::ASSIGN:
      std::cout << "ASSIGN";
      break;
  }
  std::cout << ", Value: '" << node->GetValue() << "'" << std::endl;

  for (const auto &child : node->GetChildren()) {
    PrintAST(child, depth + 1);
  }
}

/**
 * Print the usage information for the parser program.
 * @param program_name The name of the program (usually argv[0]).
 */
void PrintUsage(const std::string &program_name) {
  std::cout << "Usage: " << program_name << " [options]" << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  -f <file>    Parse file input" << std::endl;
  std::cout << "  -s <string>  Parse string input" << std::endl;
  std::cout << "  -h, --help   Show this help message" << std::endl;
  std::cout << std::endl;
  std::cout << "Examples:" << std::endl;
  std::cout << "  " << program_name << " -s \"a <- 1 + 2;\"" << std::endl;
  std::cout << "  " << program_name << " -f input.scpl" << std::endl;
}

/**
 * Read the contents of a file into a string.
 * @param filename The name of the file to read.
 * @return The contents of the file as a string.
 */
auto ReadFile(const std::string &filename) -> std::string {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
    return "";
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
 * Parses the input using the LL(1) parsing algorithm.
 * Handles epsilon productions, error recovery, and proper stack management.
 *
 * @param input The input string to parse.
 * @param source_description A description of the source (for error messages).
 * @param program_name The name of the program (for error messages).
 * @return True if parsing was successful, false otherwise.
 */
auto ParseInput(const std::string &input, const std::string &source_description, const std::string &program_name)
    -> bool {
  scp::parser::LL1Parser parser(program_name);
  parser.SetInput(input);

  std::cout << "=== Parsing " << source_description << " ===" << std::endl;
  std::cout << "Input: \n" << input << std::endl;

  auto ast = parser.Parse();
  if (ast) {
    std::cout << "✓ Parsing successful!" << std::endl;
    auto root = ast->GetRoot();
    if (root) {
      std::cout << "\n=== Abstract Syntax Tree ===" << std::endl;
      PrintAST(root);
    } else {
      std::cout << "Warning: AST root is null." << std::endl;
    }
    return true;
  }
  std::cout << "✗ Parsing failed!" << std::endl;
  return false;
}

/**
 * Main entry point for the parser program.
 * @param argc The number of command line arguments.
 * @param argv The command line arguments.
 * @return Exit status code.
 */
auto main(int argc, char *argv[]) -> int {
  if (argc < 2) {
    PrintUsage(argv[0]);
    return 1;
  }

  std::string option = argv[1];

  if (option == "-h" || option == "--help") {
    PrintUsage(argv[0]);
    return 0;
  }

  if (option == "-s" && argc == 3) {
    // Parse string input
    std::string input = argv[2];
    bool success = ParseInput(input, "string input", "temporary");
    return success ? 0 : 1;
  }

  if (option == "-f" && argc == 3) {
    // Parse file input
    std::string filename = argv[2];
    std::string input = ReadFile(filename);
    if (input.empty()) {
      return 1;
    }
    bool success = ParseInput(input, "file '" + filename + "'", filename);
    return success ? 0 : 1;
  }

  std::cerr << "Error: Invalid arguments." << std::endl;
  PrintUsage(argv[0]);
  return 1;
}
