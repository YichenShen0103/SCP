#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "cgen/code_generator.h"
#include "parser/slr_parser.h"
#include "semant/type_checker.h"

namespace fs = std::filesystem;

/**
 * Print the usage information for the lexer program.
 * @param programName The name of the program (usually argv[0]).
 */
void PrintUsage(const std::string &programName) {
  std::cout << "Usage: " << programName << " <input_file> [-o <output_file>]" << std::endl;
  std::cout << "  input_file: Path to the source file to compile" << std::endl;
  std::cout << "  -o <output_file>: Specify output file for generated assembly code" << std::endl;
  std::cout << "                    If not specified, output to standard console" << std::endl;
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
 * Main entry point for the lexer program.
 * @param argc The number of command line arguments.
 * @param argv The command line arguments.
 * @return Exit status code.
 */
auto main(int argc, char *argv[]) -> int {
  // Check command line arguments
  if (argc < 2 || argc > 4) {
    std::cerr << "Error: Invalid number of arguments." << std::endl;
    PrintUsage(argv[0]);
    return 1;
  }

  std::string filename = argv[1];
  std::string output_file;
  bool output_to_file = false;

  // Parse command line options
  for (int i = 2; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "-o") {
      if (i + 1 < argc) {
        output_file = argv[i + 1];
        output_to_file = true;
        i++;  // Skip the next argument since it's the output file
      } else {
        std::cerr << "Error: -o option requires an output file argument." << std::endl;
        PrintUsage(argv[0]);
        return 1;
      }
    } else {
      std::cerr << "Error: Unknown option: " << arg << std::endl;
      PrintUsage(argv[0]);
      return 1;
    }
  }

  try {
    // Read the input file
    std::string file_content = ReadFile(filename);

    if (file_content.empty()) {
      std::cout << "Warning: The input file is empty." << std::endl;
      return 0;
    }

    // Parse the file content
    fs::path path(filename);
    scp::parser::SLRParser parser(path.stem().string());
    parser.SetInput(file_content);
    auto ast = parser.Parse();
    if (!ast) {
      std::cerr << "Error: Failed to parse the input file." << std::endl;
      return 1;
    }

    // Type check the AST
    scp::semant::TypeChecker type_checker(ast);
    auto type_environment = type_checker.CheckType();

    // Generate code from the AST
    scp::cgen::CodeGenerator code_generator(ast, type_environment);
    std::string generated_code = code_generator.GenerateCode();

    // Output the generated assembly code
    if (output_to_file) {
      std::ofstream output(output_file);
      if (!output.is_open()) {
        std::cerr << "Error: Cannot open output file: " << output_file << std::endl;
        return 1;
      }
      output << generated_code << std::endl;
      output.close();
      std::cout << "Assembly code generated successfully to: " << output_file << std::endl;
    } else {
      // Print to standard console
      std::cout << generated_code << std::endl;
    }

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
