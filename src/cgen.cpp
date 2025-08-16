#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "cgen/code_generator.h"
#include "cgen/runtime_environment.h"
#include "core/ast.h"
#include "semant/type_checker.h"

/**
 * Print the usage information for the lexer program.
 * @param programName The name of the program (usually argv[0]).
 */
void PrintUsage(const std::string &programName) {
  std::cout << "Usage: " << programName << " <input_file>" << std::endl;
  std::cout << "  input_file: Path to the source file to generate assembly code" << std::endl;
}

/**
 * Read the contents of a file into a string.
 * @param filename The name of the file to read.
 * @return The contents of the file as a string.
 */
auto ReadFile(const std::string &filename) -> std::shared_ptr<std::ifstream> {
  auto file = std::make_shared<std::ifstream>(filename);
  if (!file->is_open()) {
    throw std::runtime_error("Cannot open file: " + filename);
  }
  return file;
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
    auto file = ReadFile(filename);

    if (file == nullptr) {
      std::cout << "Warning: The input file is empty." << std::endl;
      return 0;
    }

    // Build the AST
    auto ast = std::make_shared<scp::core::AST>(*file);

    // Build Type Checker
    scp::semant::TypeChecker type_checker(ast);
    auto type_environment = type_checker.CheckType();

    // Generate code
    scp::cgen::CodeGenerator code_generator(ast, type_environment);
    std::string generated_code = code_generator.GenerateCode();

    std::cout << "Generated Code:" << std::endl;
    std::cout << generated_code << std::endl;

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
