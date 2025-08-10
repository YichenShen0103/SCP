# Parser

## Overview

The Parser is the second component of the SCP compiler, responsible for analyzing the sequence of tokens produced by the Lexer and constructing an Abstract Syntax Tree (AST) according to the language's grammar. It uses the LL(1) parsing algorithm to ensure efficient and deterministic parsing.

## Architecture Design

### Core Classes

#### `scp::parser::LL1Parser`
The main parser class that implements the LL(1) parsing algorithm with predictive parsing table.

#### `scp::core::AST`
Abstract Syntax Tree class that represents the parsed program structure.

#### `scp::core::TreeNode`
Parse tree node used during parsing process before converting to AST.

## Supported Grammar

The Parser implements an LL(1) grammar for a simple programming language with the following constructs:

### Terminals
- `identifier`: Variable names (e.g., `variable1`, `_temp`, `myVar`)
- `number`: Integer literals (e.g., `123`, `0`, `999`)
- `plus` (+): Addition operator
- `times` (*): Multiplication operator
- `assign` (<-): Assignment operator
- `left_paren` ((): Left parenthesis
- `right_paren` ()): Right parenthesis
- `semicolon` (;): Statement terminator
- `$`: End-of-file marker

### Non-terminals
- `Program`: Top-level program structure
- `StatementList`: Sequence of statements
- `Statement`: Individual assignment statement
- `Expression`: Arithmetic expressions with addition
- `Term`: Arithmetic terms with multiplication
- `Factor`: Basic expression components

### Production Rules

```
Program → StatementList
StatementList → Statement StatementList | ε
Statement → identifier assign Expression semicolon
Expression → Term Expression'
Expression' → plus Term Expression' | ε
Term → Factor Term'
Term' → times Factor Term' | ε
Factor → identifier | number | left_paren Expression right_paren
```

## Main Features

### Core Parsing API

```cpp
// Initialize parser
void Init();

// Set input string for parsing
void SetInput(const std::string &input);

// Parse input and return AST
auto Parse() -> std::shared_ptr<core::AST>;
```

### Utility Functions

```cpp
// Print parsing table for debugging
void PrintParseTable() const;

// Print current parsing stack state
void PrintParsingStack() const;

// Validate symbols and parse table entries
auto IsValidSymbol(const std::string &symbol) const -> bool;
auto HasParseTableEntry(const std::string &nonTerminal, const std::string &terminal) const -> bool;
```

### AST Construction

```cpp
// Build AST from parse tree
auto BuildAST(const std::shared_ptr<core::TreeNode> &parse_tree) -> std::shared_ptr<core::AST>;
```

## Usage Examples

### Basic Usage

```cpp
#include "parser/ll1_parser.h"

scp::parser::LL1Parser parser;

// Parse simple assignment
parser.SetInput("result <- 123 + 456;");
auto ast = parser.Parse();

// Parse complex expression
parser.SetInput("result <- (a + b) * factor;");
auto complex_ast = parser.Parse();

// Parse multiple statements
parser.SetInput("a <- 10; b <- 20; c <- a + b;");
auto program_ast = parser.Parse();
```

### Input Examples

#### Simple Assignment
```
result <- 42;
```

#### Arithmetic Expression
```
result <- (variable1 + variable2) * factor;
```

#### Multiple Statements
```
a <- 10;
b <- 20;
c <- a + b;
result <- c * 2;
```

#### Operator Precedence
```
result <- 1 + 2 * 3;  // Parsed as: result <- 1 + (2 * 3)
```

## Technical Implementation

### LL(1) Parsing Algorithm

The parser uses a predictive parsing approach with the following components:

- **Parsing Table**: Pre-computed table mapping (non-terminal, terminal) pairs to production rules
- **Parsing Stack**: Stack-based algorithm for tracking parsing state
- **Lookahead**: Single token lookahead for decision making

### Parsing Table Structure

The parsing table is implemented as a nested hash map:
```cpp
std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> parse_table_;
```

Example entries:
- `parse_table_["Expression"]["identifier"] = {"Term", "Expression'"}`
- `parse_table_["Expression'"]["plus"] = {"plus", "Term", "Expression'"}`
- `parse_table_["Expression'"]["semicolon"] = {"ε"}`

### Algorithm Flow

1. **Initialization**: 
   - Initialize parsing stack with end marker (`$`) and start symbol (`Program`)
   - Set up lexer with input string

2. **Parsing Loop**:
   - Get current token from lexer
   - Examine top of parsing stack
   - If top is terminal: match with current token
   - If top is non-terminal: use parsing table to find production rule
   - Apply production rule by replacing non-terminal with production symbols

3. **Epsilon Handling**:
   - Special handling for epsilon (ε) productions
   - No stack operations for epsilon productions

4. **Error Recovery**:
   - Detailed error messages for parsing failures
   - Validation of parse table entries and symbols

### AST Construction

The parser constructs an Abstract Syntax Tree through a two-phase process:

1. **Parse Tree Generation**: Build parse tree during parsing process
2. **AST Transformation**: Convert parse tree to AST by eliminating unnecessary nodes

#### AST Node Types

```cpp
enum class ASTNodeType {
  ROOT,        // Program root
  IDENTIFIER,  // Variable names
  NUMBER,      // Numeric literals
  PLUS,        // Addition operations
  TIMES,       // Multiplication operations
  ASSIGN,      // Assignment statements
};
```

### Operator Precedence and Associativity

The grammar naturally handles operator precedence:
- **Multiplication (`*`)**: Higher precedence (handled in `Term`)
- **Addition (`+`)**: Lower precedence (handled in `Expression`)
- **Left Associativity**: Achieved through left-recursive elimination using prime non-terminals

## Error Handling

The parser provides comprehensive error handling:

- **Token Mismatch**: When terminal symbols don't match current token
- **Missing Productions**: When no production rule exists for (non-terminal, terminal) pair
- **Unexpected EOF**: When input ends unexpectedly
- **Parse Table Errors**: When accessing undefined entries

Error messages include context information:
```cpp
"Parsing error: expected 'semicolon', got 'identifier' with value 'x'"
"No production rule for symbol 'Expression' and terminal 'times'"
```

## Test Coverage

The Parser has extensive test coverage including:

- **Basic Functionality Tests**: Simple assignments and expressions
- **Complex Expression Tests**: Nested parentheses, operator precedence
- **Multiple Statement Tests**: Programs with multiple assignments
- **Error Case Tests**: Invalid syntax, missing operators, unbalanced parentheses
- **Edge Case Tests**: Empty programs, whitespace handling
- **AST Validation Tests**: Correct AST structure generation

### Test File Examples

Test files located in `test/data/code/`:
- `expression.scpl`: Complex arithmetic expressions
- `multiple_statements.scpl`: Multi-statement programs
- `operator_precedence.scpl`: Precedence validation
- `parentheses.scpl`: Nested parentheses
- `error_*.scpl`: Various error conditions

## Performance Characteristics

- **Linear Time Complexity**: O(n) parsing time for n tokens
- **Predictive Parsing**: No backtracking required
- **Efficient Table Lookup**: Constant time parsing decisions
- **Memory Efficient**: Stack-based approach with minimal memory overhead

## Grammar Properties

### LL(1) Conditions

The grammar satisfies LL(1) conditions:
- **No Left Recursion**: Eliminated using standard transformation
- **Left Factored**: No common prefixes in production alternatives
- **Predictable**: FIRST and FOLLOW sets are disjoint where required

### FIRST and FOLLOW Sets

**FIRST Sets:**
- FIRST(Program) = {identifier, ε}
- FIRST(Expression) = {identifier, number, left_paren}
- FIRST(Term) = {identifier, number, left_paren}
- FIRST(Factor) = {identifier, number, left_paren}

**FOLLOW Sets:**
- FOLLOW(Program) = {$}
- FOLLOW(Expression) = {semicolon, right_paren}
- FOLLOW(Term) = {plus, semicolon, right_paren}
- FOLLOW(Factor) = {times, plus, semicolon, right_paren}

## Extensibility

The parser architecture supports easy extension:

### Adding New Operators
1. Add terminal to lexer token types
2. Update grammar production rules
3. Extend parsing table with new entries
4. Add corresponding AST node types
5. Update AST transformation methods

### Adding New Statement Types
1. Extend `Statement` production rules
2. Add new non-terminals as needed
3. Update parsing table
4. Implement AST transformation for new constructs

## Dependencies

- `core/ast.h`: AST data structures
- `core/token.h`: Token definitions from lexer
- `lexer/lexer.h`: Lexer integration
- `constant/AST_constant.h`: AST-related constants
- `constant/error_messages.h`: Error message definitions
- Standard library: `<memory>`, `<stack>`, `<unordered_map>`, `<vector>`

## File Structure

```
include/parser/
└── ll1_parser.h         # LL1Parser class declaration

src/parser/
└── ll1_parser.cpp       # LL1Parser implementation

include/core/
└── ast.h               # AST class declarations

docs/
└── ll1_grammar.txt     # Formal grammar specification

test/
├── parser_test.cpp     # Unit tests
└── data/
    ├── code/           # Test input files
    └── ast/            # Expected AST outputs
```

## Integration with Lexer

The parser seamlessly integrates with the lexer:

- **Token Stream Processing**: Consumes tokens one at a time using `lexer_.Next()`
- **Lookahead Management**: Single token lookahead for parsing decisions
- **EOF Handling**: Proper handling of end-of-file conditions
- **Error Coordination**: Consistent error reporting between lexer and parser

This parser implementation provides a robust and efficient foundation for syntax analysis in the SCP compiler, with clear separation of concerns, comprehensive error handling, and extensible architecture for future language features.
